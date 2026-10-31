#!/usr/bin/env bash
set -euo pipefail

source_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
temp_dir=$(mktemp -d)
trap 'rm -rf -- "$temp_dir"' EXIT
ports="$temp_dir/sd/Ports"
apps="$temp_dir/sd/Roms/APPS"
game="$ports/yokoi"
mkdir -p "$game" "$apps" "$temp_dir/control" "$temp_dir/bin" "$temp_dir/missing"
cp "$source_dir/portmaster/Yokoi.sh" "$ports/Yokoi.sh"
cp "$source_dir/portmaster/Yokoi.sh" "$apps/Yokoi.sh"
cp "$source_dir/portmaster/Yokoi.sh" "$temp_dir/missing/Yokoi.sh"
touch "$game/yokoi_pack_rgds.ykp"
cat > "$temp_dir/bin/uname" <<'EOF'
#!/usr/bin/env bash
printf '%s\n' "${TEST_MACHINE:-aarch64}"
EOF
cat > "$temp_dir/control/control.txt" <<'EOF'
HOME=/
directory=roms
get_controls() {
    sdl_controllerconfig=test-controller
    if [ -n "${TEST_DEVICE_ARCH:-}" ]; then
        DEVICE_ARCH=$TEST_DEVICE_ARCH
    else
        unset DEVICE_ARCH
    fi
}
printf 'Control script loaded\n'
if [ "${TEST_HELPERS:-0}" == 1 ]; then
    pm_message() { printf 'PM message: %s\n' "$1"; }
    pm_platform_helper() { printf 'Platform helper: %s\n' "$1"; }
    pm_finish() { printf 'Finish helper\n'; }
fi
EOF
cat > "$game/yokoi.aarch64" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
[[ "$PWD" == "$TEST_GAME_DIR" ]]
[[ "$1" == "$TEST_GAME_DIR/yokoi_pack_rgds.ykp" ]]
[[ "$2" == "$TEST_GAME_DIR/conf" && -d "$2" ]]
[[ "$SDL_GAMECONTROLLERCONFIG" == test-controller ]]
[[ "$LD_LIBRARY_PATH" == "$TEST_GAME_DIR/libs.aarch64:/stock/libs" ]]
printf 'Fake Yokoi started\n'
exit "${TEST_EXIT_CODE:-0}"
EOF
chmod +x "$temp_dir/bin/uname" "$game/yokoi.aarch64"
export PATH="$temp_dir/bin:$PATH"
export controlfolder="$temp_dir/control"
export TEST_GAME_DIR="$game"
export LD_LIBRARY_PATH=/stock/libs
unset DEVICE_ARCH CFW_NAME

run_launcher() {
    # Wait for tee as well as bash so log assertions do not race process substitution.
    bash "$1" 2>&1 | cat > "$temp_dir/capture"
}

expect_failure() {
    local expected=$1 launcher=$2 status
    if run_launcher "$launcher"; then
        echo "Expected launcher failure: $expected" >&2
        exit 1
    else
        status=$?
    fi
    [[ "$status" == "$expected" ]]
}

run_launcher "$ports/Yokoi.sh"
grep -Fq 'Detected device architecture: aarch64' "$game/log.txt"
grep -Fq 'Control script loaded' "$game/log.txt"
grep -Fq 'Fake Yokoi started' "$game/log.txt"
grep -Fq 'Yokoi exited with status 0' "$game/log.txt"

run_launcher "$apps/Yokoi.sh"
grep -Fq "Yokoi game directory: $game" "$game/log.txt"
grep -Fq 'Fake Yokoi started' "$game/log.txt"

export TEST_HELPERS=1 TEST_DEVICE_ARCH=aarch64
run_launcher "$ports/Yokoi.sh"
grep -Fq 'Platform helper:' "$game/log.txt"
grep -Fq 'Finish helper' "$game/log.txt"
export TEST_EXIT_CODE=42
expect_failure 42 "$ports/Yokoi.sh"
grep -Fq 'Yokoi exited with status 42' "$game/log.txt"
unset TEST_EXIT_CODE TEST_HELPERS TEST_DEVICE_ARCH

mv "$game/yokoi_pack_rgds.ykp" "$temp_dir/pack"
expect_failure 1 "$ports/Yokoi.sh"
grep -Fq 'needs your Android ROM pack' "$game/log.txt"
mv "$temp_dir/pack" "$game/yokoi_pack_rgds.ykp"
chmod -x "$game/yokoi.aarch64"
expect_failure 1 "$ports/Yokoi.sh"
grep -Fq 'Missing Yokoi executable for aarch64' "$game/log.txt"
chmod +x "$game/yokoi.aarch64"

export TEST_MACHINE=riscv64
expect_failure 1 "$ports/Yokoi.sh"
grep -Fq 'unsupported host architecture: riscv64' "$game/log.txt"
unset TEST_MACHINE
export controlfolder="$temp_dir/no-control"
expect_failure 1 "$ports/Yokoi.sh"
grep -Fq 'control.txt' "$game/log.txt"
! grep -Fq 'Fake Yokoi started' "$game/log.txt"

expect_failure 1 "$temp_dir/missing/Yokoi.sh"
grep -Fq 'Missing Yokoi game directory' "$temp_dir/missing/yokoi-launch.log"
echo 'Launcher paths, early logs, stock/modern controls, errors and exit status tests passed.'
