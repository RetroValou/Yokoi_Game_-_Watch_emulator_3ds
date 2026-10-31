#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat <<'EOF'
Usage: cross-build-aarch64.sh [--sysroot DIR] [--output FILE]

Build Yokoi for ARM64 Linux, without replacing linux/build/yokoi.
Defaults:
  --sysroot       Automatically cached ARM64 Linux + SDL2 SDK.
  --output        linux/build/yokoi.aarch64 (relative to the repository).

Successful builds also stage linux/portmaster/yokoi/yokoi.aarch64 for SD-card copying,
including when --output selects a different build destination.

On x86-64 Linux, get_gnu_toolchain.sh downloads and caches Ubuntu Focal
GNU cross-compilers and an ARM64 sysroot with PortMaster's SDL2 2.26.2.
Host requirements: apt-get, dpkg-deb, make, pkg-config, curl, tar, gzip,
sha256sum, flock. Ubuntu 22.04 or a compatible newer host is recommended.

Use --sysroot for a complete target root containing its headers, libraries,
and sdl2.pc. The compiler and C++ runtime must be compatible with that root.
Environment overrides: CXX (skip automatic setup), PKG_CONFIG, READELF,
YOKOI_TOOLCHAIN_CACHE, YOKOI_BUILD_JOBS (default 2).
With an explicit CXX and no --sysroot, use the host's ARM64 multiarch files.
No system packages are installed and no ROM pack is needed.
EOF
}

fail() {
    printf 'Error: %s\n' "$*" >&2
    exit 1
}

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
sysroot=${YOKOI_SYSROOT:-}
output="$script_dir/build/yokoi.aarch64"
staged_output="$script_dir/portmaster/yokoi/yokoi.aarch64"
while (($#)); do
    case "$1" in
        --sysroot|--output)
            (($# >= 2)) && [[ -n "$2" ]] || fail "$1 requires a value."
            case "$1" in
                --sysroot) sysroot=$2 ;;
                --output) output=$2 ;;
            esac
            shift 2
            ;;
        -h|--help) usage; exit 0 ;;
        *) fail "Unknown argument: $1 (use --help)." ;;
    esac
done

[[ -z "$sysroot" || -d "$sysroot" ]] || fail "Sysroot directory does not exist: $sysroot"
export YOKOI_SYSROOT="$sysroot"
source "$script_dir/get_gnu_toolchain.sh"
sysroot=${YOKOI_SYSROOT:-/}
sysroot=$(cd -- "$sysroot" && pwd)
compiler=${CXX:-aarch64-linux-gnu-g++}
pkg_config=${PKG_CONFIG:-pkg-config}
readelf=${READELF:-readelf}
for tool in "$compiler" "$pkg_config" "$readelf" make; do
    command -v "$tool" >/dev/null || fail "Missing tool: $tool. See linux/portmaster/README.md for prerequisites."
done
machine=$("$compiler" -dumpmachine)
case "$machine" in
    aarch64*-linux*) ;;
    *) fail "Compiler targets $machine, not ARM64 Linux: $compiler" ;;
esac

# Do not let a host sdl2.pc or PKG_CONFIG_PATH supply x86 libraries.
root=${sysroot%/}
libdir="$root/usr/lib/aarch64-linux-gnu/pkgconfig:$root/usr/aarch64-linux-gnu/lib/pkgconfig:$root/lib/aarch64-linux-gnu/pkgconfig"
if [[ "$sysroot" != / ]]; then
    libdir+=":$root/usr/lib/pkgconfig:$root/usr/local/lib/pkgconfig:$root/usr/local/lib/aarch64-linux-gnu/pkgconfig"
fi
export PKG_CONFIG_PATH=
export PKG_CONFIG_LIBDIR="$libdir"
export PKG_CONFIG_SYSROOT_DIR="$sysroot"
if ! "$pkg_config" --print-errors --atleast-version=2.0.10 sdl2; then
    fail "ARM64 SDL2 development files (>= 2.0.10) are missing under $sysroot. Install libsdl2-dev:arm64 or supply a target sysroot."
fi
sdl_cflags=$("$pkg_config" --cflags sdl2)
sdl_libs=$("$pkg_config" --libs sdl2)

mkdir -p -- "$script_dir/build"
temp_dir=$(mktemp -d "$script_dir/build/.aarch64.XXXXXX")
trap 'rm -rf -- "$temp_dir"' EXIT
# Handheld libstdc++ versions vary, so only libc and SDL2 are taken from the device.
LDFLAGS="${LDFLAGS:+$LDFLAGS }-static-libstdc++ -static-libgcc" \
make --no-print-directory -C "$script_dir" -B \
    TARGET="$temp_dir/yokoi" CXX="$compiler" SYSROOT="$sysroot" \
    SDL_CFLAGS="$sdl_cflags" SDL_LIBS="$sdl_libs"

header=$(LC_ALL=C "$readelf" -h "$temp_dir/yokoi")
[[ "$header" =~ Class:[[:space:]]+ELF64 ]] &&
    [[ "$header" =~ Machine:[[:space:]]+AArch64 ]] ||
    fail "Build did not produce an ARM64 ELF executable; output was not installed."

mkdir -p -- "$(dirname -- "$output")" "$(dirname -- "$staged_output")"
install -m 755 -T -- "$temp_dir/yokoi" "$output"
install -m 755 -T -- "$temp_dir/yokoi" "$staged_output"
printf '\nBuilt ARM64 executable: %s\n' "$output"
printf 'Staged SD-card executable: %s\n' "$staged_output"
LC_ALL=C "$readelf" -d "$output" | awk '/NEEDED/ { print }'
printf 'Check GLIBC/GLIBCXX and SDL2 compatibility with your handheld before deployment.\n'
