#!/usr/bin/env bash
set -euo pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
temp_dir=$(mktemp -d)
trap 'rm -rf -- "$temp_dir"' EXIT
mkdir "$temp_dir/bin"
cat > "$temp_dir/bin/curl" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
echo download >> "$TEST_DOWNLOAD_LOG"
while (($#)); do
    if [[ "$1" == --output ]]; then
        cp "$TEST_DOWNLOAD_SOURCE" "$2"
        break
    fi
    shift
done
if [[ "${TEST_DOWNLOAD_FAIL:-0}" != 0 ]]; then
    echo 'Simulated download failure' >&2
    exit 22
fi
EOF
chmod +x "$temp_dir/bin/curl"
export PATH="$temp_dir/bin:$PATH"
export TEST_DOWNLOAD_LOG="$temp_dir/downloads"
export TEST_DOWNLOAD_SOURCE="$temp_dir/source"
printf 'verified archive\n' > "$TEST_DOWNLOAD_SOURCE"
checksum=$(sha256sum "$TEST_DOWNLOAD_SOURCE")
checksum=${checksum%% *}
export CXX=manual-compiler
source "$script_dir/get_gnu_toolchain.sh"
[[ "$CXX" == manual-compiler && ! -e "$TEST_DOWNLOAD_LOG" ]]

expect_failure() {
    local message=$1
    shift
    if "$@" > "$temp_dir/log" 2>&1; then
        echo "Expected failure: $message" >&2
        exit 1
    fi
    grep -Fq "$message" "$temp_dir/log" || {
        cat "$temp_dir/log" >&2
        exit 1
    }
}

yokoi_download https://example.invalid/toolchain "$temp_dir/archive" "$checksum"
[[ "$(wc -l < "$TEST_DOWNLOAD_LOG")" == 1 ]]
yokoi_download https://example.invalid/toolchain "$temp_dir/archive" "$checksum"
[[ "$(wc -l < "$TEST_DOWNLOAD_LOG")" == 1 ]]
printf 'corrupt archive\n' > "$temp_dir/archive"
yokoi_download https://example.invalid/toolchain "$temp_dir/archive" "$checksum"
cmp "$temp_dir/archive" "$TEST_DOWNLOAD_SOURCE"
[[ "$(wc -l < "$TEST_DOWNLOAD_LOG")" == 2 ]]
expect_failure 'SHA256 verification failed' yokoi_download \
    https://example.invalid/toolchain "$temp_dir/bad" "${checksum//?/0}"
[[ ! -e "$temp_dir/bad" && ! -e "$temp_dir/bad.part" ]]
export TEST_DOWNLOAD_FAIL=1
expect_failure 'Simulated download failure' yokoi_download \
    https://example.invalid/toolchain "$temp_dir/failed" "$checksum"
[[ ! -e "$temp_dir/failed" && ! -e "$temp_dir/failed.part" ]]

home="$temp_dir/cache/ubuntu-focal-aarch64-gcc10-sdl2-2.26.2-v1"
mkdir -p "$home/bin" "$home/sysroot/usr/include/SDL2" \
    "$home/sysroot/usr/lib/aarch64-linux-gnu/pkgconfig"
cat > "$home/bin/aarch64-linux-gnu-g++" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
case "$1" in
    -dumpfullversion) echo "${TEST_GCC_VERSION:-10.5.0}" ;;
    -dumpmachine) echo aarch64-linux-gnu ;;
    *) exit 1 ;;
esac
EOF
cp "$home/bin/aarch64-linux-gnu-g++" "$home/bin/aarch64-linux-gnu-readelf"
chmod +x "$home/bin/aarch64-linux-gnu-g++" "$home/bin/aarch64-linux-gnu-readelf"
touch "$home/sysroot/usr/include/SDL2/SDL.h" \
    "$home/sysroot/usr/lib/aarch64-linux-gnu/pkgconfig/sdl2.pc" \
    "$home/sysroot/usr/lib/aarch64-linux-gnu/libSDL2.so"
printf '%s\n' ubuntu-focal-aarch64-gcc10-sdl2-2.26.2-v1 > "$home/.yokoi-toolchain"
downloads_before=$(wc -l < "$TEST_DOWNLOAD_LOG")
for run in 1 2; do
    (
        unset CXX READELF YOKOI_SYSROOT
        export YOKOI_TOOLCHAIN_CACHE="$temp_dir/cache"
        source "$script_dir/get_gnu_toolchain.sh"
        [[ "$CXX" == "$home/bin/aarch64-linux-gnu-g++" ]]
        [[ "$READELF" == "$home/bin/aarch64-linux-gnu-readelf" ]]
        [[ "$YOKOI_SYSROOT" == "$home/sysroot" ]]
    )
done
[[ "$(wc -l < "$TEST_DOWNLOAD_LOG")" == "$downloads_before" ]]
expect_failure 'Simulated download failure' env CXX= READELF= YOKOI_SYSROOT= \
    YOKOI_TOOLCHAIN_CACHE="$temp_dir/cache" TEST_GCC_VERSION=11.4.0 \
    bash "$script_dir/get_gnu_toolchain.sh"
[[ -f "$home/.yokoi-toolchain" && -x "$home/bin/aarch64-linux-gnu-g++" ]]
echo 'Toolchain checksum, cache reuse, explicit override and failure tests passed.'
