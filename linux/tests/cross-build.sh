#!/usr/bin/env bash
set -euo pipefail

source_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
temp_dir=$(mktemp -d)
trap 'rm -rf -- "$temp_dir"' EXIT
script_dir="$temp_dir/repo/linux"
export TEST_SYSROOT="$temp_dir/sysroot"
mkdir -p "$temp_dir/bin" "$TEST_SYSROOT/usr/lib/aarch64-linux-gnu/pkgconfig" \
    "$temp_dir/host" "$temp_dir/empty" "$script_dir"
cp "$source_dir/cross-build-aarch64.sh" "$source_dir/get_gnu_toolchain.sh" "$script_dir/"

cat > "$TEST_SYSROOT/usr/lib/aarch64-linux-gnu/pkgconfig/sdl2.pc" <<'EOF'
prefix=/usr
libdir=${prefix}/lib/aarch64-linux-gnu
includedir=${prefix}/include
Name: SDL2
Description: ARM64 test metadata
Version: 2.26.2
Libs: -L${libdir} -lSDL2
Cflags: -I${includedir}/SDL2
EOF
cat > "$temp_dir/host/sdl2.pc" <<'EOF'
Name: SDL2
Description: Host metadata that must not be used
Version: 2.30.0
Libs: -lWRONG_HOST_SDL
EOF
cat > "$temp_dir/bin/aarch64-linux-gnu-g++" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
[[ "$1" == -dumpmachine ]]
echo "${TEST_COMPILER_MACHINE:-aarch64-linux-gnu}"
EOF
cat > "$temp_dir/bin/make" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
target= cflags= libs= sysroot=
for arg in "$@"; do
    case "$arg" in
        TARGET=*) target=${arg#TARGET=} ;;
        SDL_CFLAGS=*) cflags=${arg#SDL_CFLAGS=} ;;
        SDL_LIBS=*) libs=${arg#SDL_LIBS=} ;;
        SYSROOT=*) sysroot=${arg#SYSROOT=} ;;
    esac
done
[[ "$sysroot" == "$TEST_SYSROOT" ]]
[[ "$cflags" == *"-I$TEST_SYSROOT/usr/include/SDL2"* ]]
[[ "$libs" == *"-L$TEST_SYSROOT/usr/lib/aarch64-linux-gnu"* ]]
[[ "$libs" == *"-lSDL2"* && "$libs" != *WRONG_HOST* ]]
[[ "$target" == */linux/build/.aarch64.*/yokoi ]]
if [[ "${TEST_BUILD_FAIL:-0}" != 0 ]]; then
    echo 'Simulated compilation failure' >&2
    exit 2
fi
printf 'built\n' > "$target"
printf '%s\n' "$target" > "$TEST_TARGET_LOG"
EOF
cat > "$temp_dir/bin/readelf" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
[[ -f "$2" ]]
case "$1" in
    -h) printf '  Class: ELF64\n  Machine: %s\n' "${TEST_ELF_MACHINE:-AArch64}" ;;
    -d) echo '  (NEEDED) Shared library: [libSDL2-2.0.so.0]' ;;
    *) exit 1 ;;
esac
EOF
chmod +x "$temp_dir/bin/"*
export PATH="$temp_dir/bin:$PATH"
export CXX=aarch64-linux-gnu-g++
export READELF=readelf
export PKG_CONFIG=pkg-config
export PKG_CONFIG_PATH="$temp_dir/host"
export TEST_TARGET_LOG="$temp_dir/target"
output="$temp_dir/output/yokoi.aarch64"
staged_output="$script_dir/portmaster/yokoi/yokoi.aarch64"

expect_failure() {
    local message=$1
    shift
    if "$@" > "$temp_dir/log" 2>&1; then
        echo "Expected failure: $message" >&2
        exit 1
    fi
    if ! grep -Fq -- "$message" "$temp_dir/log"; then
        cat "$temp_dir/log" >&2
        echo "Missing diagnostic: $message" >&2
        exit 1
    fi
}

bash "$script_dir/cross-build-aarch64.sh" --help > "$temp_dir/log"
grep -Fq 'linux/build/yokoi.aarch64' "$temp_dir/log"
grep -Fq 'linux/portmaster/yokoi/yokoi.aarch64' "$temp_dir/log"
expect_failure 'Unknown argument' bash "$script_dir/cross-build-aarch64.sh" --bad
expect_failure 'requires a value' bash "$script_dir/cross-build-aarch64.sh" --sysroot
expect_failure 'does not exist' bash "$script_dir/cross-build-aarch64.sh" --sysroot "$temp_dir/missing"
expect_failure 'Missing tool' env CXX="$temp_dir/missing-compiler" \
    bash "$script_dir/cross-build-aarch64.sh"
expect_failure 'not ARM64 Linux' env TEST_COMPILER_MACHINE=x86_64-linux-gnu \
    bash "$script_dir/cross-build-aarch64.sh"
expect_failure 'SDL2 development files' bash "$script_dir/cross-build-aarch64.sh" \
    --sysroot "$temp_dir/empty" --output "$output"
[[ ! -e "$output" && ! -e "$staged_output" ]]

bash "$script_dir/cross-build-aarch64.sh" --sysroot "$TEST_SYSROOT" > "$temp_dir/log"
[[ -x "$script_dir/build/yokoi.aarch64" && -x "$staged_output" ]]
cmp "$script_dir/build/yokoi.aarch64" "$staged_output"
[[ ! -e "$temp_dir/repo/portmaster" ]]

bash "$script_dir/cross-build-aarch64.sh" --sysroot "$TEST_SYSROOT" \
    --output "$output" > "$temp_dir/log"
[[ -x "$output" && "$(cat "$output")" == built ]]
[[ -x "$staged_output" ]]
cmp "$output" "$staged_output"
[[ ! -d "$(dirname -- "$(cat "$TEST_TARGET_LOG")")" ]]

bash "$script_dir/cross-build-aarch64.sh" --sysroot "$TEST_SYSROOT" \
    --output "$staged_output" > "$temp_dir/log"
[[ -x "$staged_output" && "$(cat "$staged_output")" == built ]]

printf 'previous build\n' > "$output"
printf 'previous staged build\n' > "$staged_output"
expect_failure 'output was not installed' env TEST_ELF_MACHINE='Advanced Micro Devices X86-64' \
    bash "$script_dir/cross-build-aarch64.sh" --sysroot "$TEST_SYSROOT" --output "$output"
[[ "$(cat "$output")" == 'previous build' ]]
[[ "$(cat "$staged_output")" == 'previous staged build' ]]
expect_failure 'Simulated compilation failure' env TEST_BUILD_FAIL=1 bash "$script_dir/cross-build-aarch64.sh" \
    --sysroot "$TEST_SYSROOT" --output "$output"
[[ "$(cat "$output")" == 'previous build' ]]
[[ "$(cat "$staged_output")" == 'previous staged build' ]]

mv "$staged_output" "$temp_dir/previous-staged"
mkdir "$staged_output"
expect_failure 'cannot overwrite directory' bash "$script_dir/cross-build-aarch64.sh" \
    --sysroot "$TEST_SYSROOT" --output "$output"
! grep -Fq 'Built ARM64 executable:' "$temp_dir/log"
[[ ! -d "$(dirname -- "$(cat "$TEST_TARGET_LOG")")" ]]
echo 'ARM64 cross-build orchestration tests passed (compiler/build/ELF inspection mocked).'
