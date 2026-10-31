#!/bin/bash
launcher_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd) || exit 1
GAMEDIR="$launcher_dir/yokoi"
# The launcher can live away from the port data: stock RGDS can expose a second
# launcher in Roms/APPS, and muOS keeps it in ROMS/Ports with the data in ports/yokoi.
for candidate in "$launcher_dir/../../Ports/yokoi" "$launcher_dir/../../ports/yokoi" \
    /mnt/sdcard/ports/yokoi /mnt/mmc/ports/yokoi; do
    [ -d "$GAMEDIR" ] && break
    if [ -d "$candidate" ]; then
        GAMEDIR=$(cd -- "$candidate" && pwd) || exit 1
    fi
done
log_file="$GAMEDIR/log.txt"
if [ ! -d "$GAMEDIR" ]; then
    log_file="$launcher_dir/yokoi-launch.log"
fi
# Capture control-script and path failures too, before invoking any PortMaster code.
: > "$log_file" || exit 1
exec > >(tee "$log_file") 2>&1

notify() {
    printf '%s\n' "$1" >&2
    if command -v pm_message >/dev/null 2>&1; then
        pm_message "$1"
    fi
}

if [ ! -d "$GAMEDIR" ]; then
    notify "Missing Yokoi game directory: $GAMEDIR. Copy the yokoi folder alongside the Ports launcher."
    exit 1
fi
printf 'Yokoi game directory: %s\n' "$GAMEDIR"
XDG_DATA_HOME=${XDG_DATA_HOME:-${HOME:-/}/.local/share}
if [ -z "${controlfolder:-}" ]; then
    if [ -d /mnt/ports/PortMaster ]; then
        controlfolder=/mnt/ports/PortMaster
    elif [ -d /PortMaster ]; then
        controlfolder=/PortMaster
    elif [ -d /opt/system/Tools/PortMaster ]; then
        controlfolder=/opt/system/Tools/PortMaster
    elif [ -d /opt/tools/PortMaster ]; then
        controlfolder=/opt/tools/PortMaster
    elif [ -d "$XDG_DATA_HOME/PortMaster" ]; then
        controlfolder="$XDG_DATA_HOME/PortMaster"
    else
        controlfolder=/roms/ports/PortMaster
    fi
fi
if [ ! -f "$controlfolder/control.txt" ]; then
    notify "Missing PortMaster control.txt in $controlfolder"
    exit 1
fi
# Its exit status is whatever its last line returned, so it does not signal failure.
source "$controlfolder/control.txt"
[ -f "$controlfolder/mod_${CFW_NAME:-}.txt" ] && source "$controlfolder/mod_${CFW_NAME:-}.txt"
get_controls
cd -- "$GAMEDIR" || exit 1
if command -v pm_finish >/dev/null 2>&1; then
    trap pm_finish EXIT
fi
# Older stock control.txt versions do not set DEVICE_ARCH or provide pm_* helpers.
if [ -z "${DEVICE_ARCH:-}" ]; then
    machine=$(uname -m)
    case "$machine" in
        aarch64|arm64) DEVICE_ARCH=aarch64 ;;
        x86_64) DEVICE_ARCH=x86_64 ;;
        *)
            notify "PortMaster did not set DEVICE_ARCH; unsupported host architecture: $machine"
            exit 1
            ;;
    esac
    printf 'Detected device architecture: %s\n' "$DEVICE_ARCH"
fi
export DEVICE_ARCH
export SDL_GAMECONTROLLERCONFIG="$sdl_controllerconfig"
export LD_LIBRARY_PATH="$GAMEDIR/libs.${DEVICE_ARCH}${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
if [ ! -f yokoi_pack_rgds.ykp ]; then
    notify "Yokoi needs your Android ROM pack. Copy yokoi_pack_rgds.ykp into $GAMEDIR."
    exit 1
fi
binary="$GAMEDIR/yokoi.${DEVICE_ARCH}"
$ESUDO chmod +x "$binary" 2>/dev/null
if [ ! -x "$binary" ]; then
    notify "Missing Yokoi executable for ${DEVICE_ARCH}: $binary. See the port build instructions."
    exit 1
fi
mkdir -p "$GAMEDIR/conf" || exit 1
# Exit is handled in SDL so save-state writing completes before shutdown.
if command -v pm_platform_helper >/dev/null 2>&1; then
    # Non-fatal: muOS's helper can fail writing its process tracking file.
    pm_platform_helper "$binary"
else
    printf 'PortMaster platform helper unavailable; launching directly.\n'
fi
"$binary" "$GAMEDIR/yokoi_pack_rgds.ykp" "$GAMEDIR/conf"
result=$?
printf 'Yokoi exited with status %s\n' "$result"
exit "$result"
