#!/usr/bin/env bash
# Source to provision the Ubuntu Focal cross-build environment documented at:
# https://portmaster.games/build-environments.html
# Cache/download workflow inspired by aminch/bma800xl's get_gnu_toolchain.sh.

yokoi_download() {
    local url=$1 archive=$2 checksum=$3
    if [[ -f "$archive" ]]; then
        if printf '%s  %s\n' "$checksum" "$archive" | sha256sum --check --status; then
            return 0
        fi
        printf 'Cached archive failed SHA256 verification; downloading again: %s\n' "$archive" >&2
    fi
    printf 'Downloading %s\n' "$url" >&2
    if ! curl --fail --location --retry 3 --output "$archive.part" "$url"; then
        rm -f -- "$archive.part"
        return 1
    fi
    if ! printf '%s  %s\n' "$checksum" "$archive.part" | sha256sum --check --status; then
        printf 'Error: SHA256 verification failed: %s\n' "$url" >&2
        rm -f -- "$archive.part"
        return 1
    fi
    mv -- "$archive.part" "$archive"
}

yokoi_toolchain_valid() {
    local home=$1
    [[ -f "$home/.yokoi-toolchain" &&
       "$(cat "$home/.yokoi-toolchain")" == ubuntu-focal-aarch64-gcc10-sdl2-2.26.2-v1 &&
       -x "$home/bin/aarch64-linux-gnu-g++" &&
       -x "$home/bin/aarch64-linux-gnu-readelf" &&
       -f "$home/sysroot/usr/include/SDL2/SDL.h" &&
       -f "$home/sysroot/usr/lib/aarch64-linux-gnu/pkgconfig/sdl2.pc" &&
       -f "$home/sysroot/usr/lib/aarch64-linux-gnu/libSDL2.so" ]] &&
        [[ "$("$home/bin/aarch64-linux-gnu-g++" -dumpfullversion)" == 10.* ]] &&
        [[ "$("$home/bin/aarch64-linux-gnu-g++" -dumpmachine)" == aarch64-linux-gnu ]]
}

yokoi_ubuntu_packages() (
    set -euo pipefail
    local cache=$1 arch=$2 destination=$3 keyring=$4
    shift 4
    local apt_dir="$cache/apt-$arch" mirror=archive.ubuntu.com/ubuntu
    [[ "$arch" != arm64 ]] || mirror=ports.ubuntu.com/ubuntu-ports
    mkdir -p "$apt_dir/etc/apt" "$apt_dir/var/lib/apt/lists/partial" \
        "$apt_dir/var/cache/apt/archives/partial" "$apt_dir/plan/partial" || exit 1
    touch "$apt_dir/status" || exit 1
    cat > "$apt_dir/etc/apt/sources.list" <<EOF
deb [arch=$arch signed-by=$keyring] https://$mirror focal main universe
deb [arch=$arch signed-by=$keyring] https://$mirror focal-updates main universe
deb [arch=$arch signed-by=$keyring] https://$mirror focal-security main universe
EOF
    local -a options=(
        -o "Dir=$apt_dir"
        -o "Dir::State::status=$apt_dir/status"
        -o "Dir::Etc::sourcelist=$apt_dir/etc/apt/sources.list"
        -o "Dir::Etc::sourceparts=-"
        -o "Dir::Etc::main=-" -o "Dir::Etc::parts=-"
        -o "Dir::Etc::trusted=-" -o "Dir::Etc::trustedparts=-"
        -o "APT::Architecture=$arch" -o "APT::Architectures::=$arch"
        -o "APT::Sandbox::User=$(id -un)"
        -o "APT::Get::AllowUnauthenticated=false"
        -o "Acquire::AllowInsecureRepositories=false"
        -o "Acquire::Languages=none"
    )
    printf 'Downloading signed Ubuntu Focal %s packages...\n' "$arch" >&2
    apt-get "${options[@]}" update > "$apt_dir/update.log" 2>&1 || {
        cat "$apt_dir/update.log" >&2
        exit 1
    }
    apt-get "${options[@]}" -o "Dir::Cache::archives=$apt_dir/plan" \
        --print-uris --no-install-recommends -y install "$@" \
        > "$apt_dir/plan.log" 2>&1 || {
        cat "$apt_dir/plan.log" >&2
        exit 1
    }
    awk '/^\047https:\/\// { print $2 }' "$apt_dir/plan.log" > "$apt_dir/packages" || exit 1
    [[ -s "$apt_dir/packages" ]] || {
        printf 'Error: Ubuntu package resolution produced no downloadable packages.\n' >&2
        exit 1
    }
    apt-get "${options[@]}" --download-only --no-install-recommends -y install "$@" \
        > "$apt_dir/download.log" 2>&1 || {
        cat "$apt_dir/download.log" >&2
        exit 1
    }
    local package
    while IFS= read -r package; do
        dpkg-deb -x "$apt_dir/var/cache/apt/archives/$package" "$destination" || exit 1
    done < "$apt_dir/packages"
)

yokoi_install_toolchain() (
    set -euo pipefail
    local cache=$1 home=$2 stage
    flock 9 || exit 1
    yokoi_toolchain_valid "$home" && exit 0
    stage=$(mktemp -d "$cache/.install.XXXXXX") || exit 1
    trap 'rm -rf -- "$stage"' EXIT
    yokoi_download \
        https://archive.ubuntu.com/ubuntu/pool/main/u/ubuntu-keyring/ubuntu-keyring_2021.03.26_all.deb \
        "$cache/ubuntu-keyring_2021.03.26_all.deb" \
        0d0e7ed6b112f5d03eabf3c7eb01ebdacf9c57714b279e90495cfc58c8c4520f || exit 1
    dpkg-deb -x "$cache/ubuntu-keyring_2021.03.26_all.deb" "$stage/keys" || exit 1
    local keyring="$stage/keys/usr/share/keyrings/ubuntu-archive-keyring.gpg"
    mkdir "$stage/toolchain" "$stage/toolchain/sysroot" "$stage/toolchain/bin" \
        "$stage/sdl-build" || exit 1
    yokoi_ubuntu_packages "$cache" amd64 "$stage/toolchain" "$keyring" \
        g++-10-aarch64-linux-gnu libwayland-bin || exit 1
    yokoi_ubuntu_packages "$cache" arm64 "$stage/toolchain/sysroot" "$keyring" \
        libc6-dev libstdc++-10-dev libsdl2-dev libdrm-dev libgbm-dev \
        libegl1-mesa-dev libgles2-mesa-dev || exit 1

    # Only compiler processes use the cached host libraries, never host make/APT.
    cat > "$stage/toolchain/bin/tool" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
home=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
tool=${0##*/}
extra=()
case "$tool" in
    aarch64-linux-gnu-gcc|aarch64-linux-gnu-g++)
        sysroot="$home/sysroot"
        next_sysroot=0
        for arg in "$@"; do
            if ((next_sysroot)); then
                sysroot=$arg
                next_sysroot=0
            fi
            case "$arg" in
                --sysroot=*) sysroot=${arg#--sysroot=} ;;
                --sysroot) next_sysroot=1 ;;
            esac
        done
        # Prefer the sysroot's libc over Ubuntu's absolute-path cross-libc scripts.
        extra=("--sysroot=$sysroot" "-L$sysroot/usr/lib/aarch64-linux-gnu" "-L$sysroot/lib/aarch64-linux-gnu")
        tool+=-10
        ;;
esac
exec env LD_LIBRARY_PATH="$home/usr/lib/x86_64-linux-gnu${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" \
    "$home/usr/bin/$tool" "${extra[@]}" "$@"
EOF
    chmod +x "$stage/toolchain/bin/tool" || exit 1
    local tool
    for tool in gcc g++ ar as ld nm objcopy objdump ranlib readelf strip; do
        ln -s tool "$stage/toolchain/bin/aarch64-linux-gnu-$tool" || exit 1
    done
    ln -s tool "$stage/toolchain/bin/wayland-scanner" || exit 1
    export PATH="$stage/toolchain/bin:$stage/toolchain/usr/bin:$PATH"
    local sysroot="$stage/toolchain/sysroot"
    # Focal's dev symlinks are absolute; left alone they dangle and the linker picks libc's .a files.
    local link
    while IFS= read -r -d '' link; do
        ln -sfr -- "$sysroot$(readlink "$link")" "$link" || exit 1
    done < <(find "$sysroot" -lname '/*' -print0)
    yokoi_download \
        https://github.com/libsdl-org/SDL/releases/download/release-2.26.2/SDL2-2.26.2.tar.gz \
        "$cache/SDL2-2.26.2.tar.gz" \
        95d39bc3de037fbdfa722623737340648de4f180a601b0afad27645d150b99e0 || exit 1
    tar -xzf "$cache/SDL2-2.26.2.tar.gz" -C "$stage" || exit 1
    printf 'Building PortMaster SDL2 2.26.2 for the cached ARM64 sysroot...\n' >&2
    cd "$stage/sdl-build" || exit 1
    CC=aarch64-linux-gnu-gcc CXX=aarch64-linux-gnu-g++ LD=aarch64-linux-gnu-ld \
    CFLAGS="-O2 --sysroot=$sysroot" CXXFLAGS="-O2 --sysroot=$sysroot" \
    CPPFLAGS= LDFLAGS="--sysroot=$sysroot" LIBS= \
    PKG_CONFIG_PATH= \
    PKG_CONFIG_LIBDIR="$sysroot/usr/lib/aarch64-linux-gnu/pkgconfig:$sysroot/usr/lib/pkgconfig:$sysroot/usr/share/pkgconfig" \
    PKG_CONFIG_SYSROOT_DIR="$sysroot" \
        "$stage/SDL2-2.26.2/configure" --host=aarch64-linux-gnu \
        --prefix=/usr --libdir=/usr/lib/aarch64-linux-gnu \
        --enable-shared --disable-static > "$stage/configure.log" 2>&1 || {
            cat "$stage/configure.log" >&2
            if [[ -f config.log ]]; then
                cat config.log >&2
            fi
            exit 1
        }
    make -j"${YOKOI_BUILD_JOBS:-2}" WAYLAND_SCANNER="$stage/toolchain/bin/wayland-scanner" \
        > "$stage/build.log" 2>&1 || {
        cat "$stage/build.log" >&2
        exit 1
    }
    make DESTDIR="$sysroot" WAYLAND_SCANNER="$stage/toolchain/bin/wayland-scanner" install \
        > "$stage/install.log" 2>&1 || {
        cat "$stage/build.log" >&2
        cat "$stage/install.log" >&2
        exit 1
    }
    printf '%s\n' ubuntu-focal-aarch64-gcc10-sdl2-2.26.2-v1 \
        > "$stage/toolchain/.yokoi-toolchain" || exit 1
    yokoi_toolchain_valid "$stage/toolchain" || {
        printf 'Error: cached Ubuntu ARM64 toolchain/SDL2 validation failed.\n' >&2
        exit 1
    }
    if [[ -e "$home" ]]; then
        mv -- "$home" "$stage/previous-toolchain" || exit 1
    fi
    mv -- "$stage/toolchain" "$home" || exit 1
) 9>"$1/.lock"

yokoi_get_gnu_toolchain() {
    # Explicit compiler overrides retain the existing manual/multiarch workflow.
    [[ -z "${CXX:-}" ]] || return 0
    if [[ "$(uname -s)" != Linux || "$(uname -m)" != x86_64 ]]; then
        printf 'Error: automatic setup requires x86-64 Linux; supply CXX and --sysroot on other hosts.\n' >&2
        return 1
    fi
    local cache=${YOKOI_TOOLCHAIN_CACHE:-${XDG_CACHE_HOME:-$HOME/.cache}/yokoi/toolchains}
    local tool
    for tool in apt-get dpkg-deb curl sha256sum tar gzip make pkg-config flock; do
        if ! command -v "$tool" >/dev/null; then
            printf 'Error: missing bootstrap prerequisite: %s\n' "$tool" >&2
            return 1
        fi
    done
    if [[ "$cache" == *[!a-zA-Z0-9_./-]* ]]; then
        printf 'Error: toolchain cache path must not contain spaces or shell metacharacters: %s\n' "$cache" >&2
        return 1
    fi
    mkdir -p -- "$cache" || return 1
    cache=$(cd -- "$cache" && pwd) || return 1
    local home="$cache/ubuntu-focal-aarch64-gcc10-sdl2-2.26.2-v1"
    if ! yokoi_toolchain_valid "$home"; then
        yokoi_install_toolchain "$cache" "$home" || return 1
    fi
    export PATH="$home/bin:$home/usr/bin:$PATH"
    export CXX="$home/bin/aarch64-linux-gnu-g++"
    export READELF=${READELF:-$home/bin/aarch64-linux-gnu-readelf}
    export YOKOI_SYSROOT=${YOKOI_SYSROOT:-$home/sysroot}
    printf 'Using cached PortMaster Ubuntu Focal ARM64 toolchain: %s\n' "$home" >&2
}

if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
    set -euo pipefail
fi
yokoi_get_gnu_toolchain
