# Yokoi for PortMaster (development port)

Native Linux SDL2 frontend derived from Yokoi's Android version. Uses the existing
SM5A/SM510/SM511/SM512 C++ cores, Android controller mappings and PNG ROM packs.
No Android runtime or APK is required. No ROMs or commercial artwork are included.

## Frontend source layout

The Linux frontend mirrors Android's `core/`, `input/`, and `render/` separation:

- [main.cpp](../main.cpp): command-line defaults, help, and top-level error reporting.
- [core/yokoi_runtime.cpp](../core/yokoi_runtime.cpp): SDL lifetime, events, and 60 Hz frame pacing.
- [core/yokoi_app.cpp](../core/yokoi_app.cpp): title selection, CPU boot/save/resume,
  grouped navigation, settings/pointer input, CPU stepping, and LCD blink protection.
- [core/yokoi_ui.h](../core/yokoi_ui.h): shared menu/settings state and option lists.
- [core/yokoi_preferences.cpp](../core/yokoi_preferences.cpp): saved screen-swap
  preference.
- [core/yokoi_audio.cpp](../core/yokoi_audio.cpp): CPU sound sampling, SDL format
  conversion, queue limits, and audio resource ownership.
- [input/yokoi_controller.cpp](../input/yokoi_controller.cpp): keyboard/controller
  bindings, stick thresholds, hotplug, and held versus newly pressed buttons.
- [input/yokoi_pointer.h](../input/yokoi_pointer.h): touch identity and pointer phases.
- [render/yokoi_render.cpp](../render/yokoi_render.cpp): title textures, menu,
  multi-screen layout, and artwork/segment layering.
- [render/yokoi_graphics.cpp](../render/yokoi_graphics.cpp): packed PNG decoding,
  tinting, atlas coordinate conversion, and text drawing.

The shared CPU utilities and per-game input mappings still come from Android.
The smoke executable links these Linux modules directly rather than including
the application entry point.

## Build

Use an architecture-compatible PortMaster build environment:
https://portmaster.games/build-environments.html

Inside that environment, install/use a C++20 compiler, make and SDL2 development
files (SDL2 >= 2.0.10; PortMaster documents SDL2 2.26.2). From the repo root:

```sh
make -C linux
file linux/build/yokoi
```

All Linux executables are written to the ignored `linux/build/` directory.
For a native aarch64 build, copy `linux/build/yokoi` to
`linux/portmaster/yokoi/yokoi.aarch64`. For other architectures, build against their
sysroot and use the matching PortMaster DEVICE_ARCH suffix. Do not rename an x86
binary to aarch64. Avoid host-specific `-march=native` flags. Use `ldd`/`readelf`
to check shared dependencies and GLIBC/GLIBCXX compatibility with the target CFW.
The desktop build needs only SDL2 and the C++ standard library at runtime.
PNG decoding and TrueType rendering are vendored, with their licenses in the headers.
The Audiowide font and its SIL Open Font License are embedded in the executable;
no external font files or SDL_ttf installation are needed.

### Menu font

Menus and settings use [Audiowide](https://fonts.google.com/specimen/Audiowide):
smooth retro-futuristic lettering rather than a pixel font. stb_truetype rasterizes
glyphs at 64 pixels and caches antialiased textures separately for each display.
UTF-8 is decoded directly, without depending on firmware iconv/gconv conversion
modules. Text uses the font's supported glyphs; missing characters produce a warning.
Title and prompt text is fitted to the available width.

Audiowide is free to bundle/use under SIL OFL 1.1. Its unmodified font and licence
are vendored from Google Fonts revision `6c1dbe27dcf7d817d4b62cec033ba77fa8e2a5ec`.
The licence is embedded too, so a standalone handheld binary retains it:

```sh
./linux/build/yokoi --font-license
```

### Cross-compile ARM64 from an x86-64 Linux host

Run one command from the repository root:

```sh
./linux/cross-build-aarch64.sh
```

At startup it sources `linux/get_gnu_toolchain.sh`, which follows the
download/cache pattern of
[aminch/bma800xl's helper](https://github.com/aminch/bma800xl/blob/main/get_gnu_toolchain.sh),
but uses **GNU Linux**, not bare-metal, toolchains.

The automatic environment follows
[PortMaster's build guide](https://portmaster.games/build-environments.html):
Ubuntu 20.04 (Focal) GNU AArch64 cross-compilers, an ARM64 Ubuntu sysroot, and
SDL2 **2.26.2** built for that sysroot. Compiler packages come from the Ubuntu
x86 archive; target packages come from `ports.ubuntu.com`. Signed APT metadata
and package hashes verify the Ubuntu downloads; pinned SHA256 checksums verify
the Ubuntu keyring and SDL2 source. GCC is restricted to the Focal GCC 10 series;
the exact package revisions follow Focal updates when initially provisioned.

A binary built against older libraries also runs on newer ones, but not the reverse,
so Focal's glibc 2.31 is the newest version a build can require; the current build
needs only GLIBC_2.27. libstdc++ and libgcc are linked statically, so handhelds
only need to supply glibc and SDL2.

Downloads and the completed compiler/sysroot are cached under
`${XDG_CACHE_HOME:-$HOME/.cache}/yokoi/toolchains`. Subsequent builds reuse the
completed cache without contacting package repositories or rebuilding SDL2.
Installations are staged under a cache lock, so failed setup does not replace a
previous environment. No `sudo`, system package installation, foreign-architecture
registration, Docker, chroot, or QEMU is used.

Use an x86-64 Ubuntu 20.04 or compatible newer Linux host with `apt-get`,
`dpkg-deb`, `make`, `pkg-config`, `curl`, `tar`, `gzip`, `sha256sum`, and `flock`.
On Ubuntu/Mint the extra host prerequisites can be installed with:

```sh
sudo apt install make pkg-config curl
```

First use requires internet access and space for the downloaded packages,
compiler, sysroot, and temporary SDL2 build. Optional overrides:

```sh
YOKOI_TOOLCHAIN_CACHE=/path/to/cache YOKOI_BUILD_JOBS=4 \
  ./linux/cross-build-aarch64.sh --output /path/to/ports/yokoi/yokoi.aarch64
```

An explicit `CXX` skips automatic provisioning, retaining the manual workflow.
Supply an ARM64 sysroot containing SDL2 headers, libraries and `sdl2.pc`, or omit
`--sysroot` to use an already configured host ARM64 multiarch installation:

```sh
CXX=/path/to/toolchain/bin/aarch64-linux-gnu-g++ \
  ./linux/cross-build-aarch64.sh --sysroot /path/to/arm64-sysroot
```

Manual sysroots must match the compiler's C++ runtime and the handheld's ABI.
Expected pkg-config locations include `usr/lib/aarch64-linux-gnu/pkgconfig`
and, for a supplied sysroot, `usr/lib/pkgconfig` or `usr/local/lib/pkgconfig`.
Older CFWs may still require a lower GLIBC baseline than Focal provides.
Verify the shared libraries on the actual device; do not blindly copy the whole
cached sysroot to the handheld. The script preserves `linux/build/yokoi` and verifies
ELF64/AArch64 before installing the result to
`linux/build/yokoi.aarch64` by default. Every successful cross-build also places
an executable copy in `linux/portmaster/yokoi/yokoi.aarch64`, including builds using
`--output`. Both outputs and ROM packs are ignored by Git. Check them with:

```sh
file linux/build/yokoi.aarch64
readelf --version-info linux/build/yokoi.aarch64
file linux/portmaster/yokoi/yokoi.aarch64
```

A standard Linux desktop development build:

```sh
sudo apt install build-essential libsdl2-dev
make -C linux
./linux/build/yokoi /path/to/yokoi_pack_rgds.ykp /path/to/writable-save-directory
```

`make -C linux clean` removes the desktop, smoke-test and ARM64 build executables.
Cross-build temporary files also stay under `linux/build/` and are removed after
success or failure. The downloaded toolchain cache remains outside the repository.

## Install on handheld

1. From the repository, copy `linux/portmaster/Yokoi.sh` and the
   `linux/portmaster/yokoi/` directory into your device's `ports/` directory.
2. The ARM64 cross-build already stages `yokoi/yokoi.aarch64`. For another
   architecture, place its executable in `ports/yokoi/` with the matching suffix.
3. Generate your own **Android/RGDS pack**, as described in
   [CONVERT_ROM/README.md](../../CONVERT_ROM/README.md).
   Copy it to `ports/yokoi/yokoi_pack_rgds.ykp` (content version 3 or later).
   A 3DS pack contains incompatible textures and is explicitly rejected.
4. Run Yokoi from the Ports menu. Saves are in `ports/yokoi/conf/yokoi_gw_saves/`;
   launch output is in `ports/yokoi/log.txt`.

The launcher resolves the game folder alongside itself instead of assuming
`/roms/ports`. On stock RGDS this is `/mnt/mmc/Ports/yokoi`; a launcher copied
into `/mnt/mmc/Roms/APPS` also finds that same Ports folder. Update both launcher
copies if your device has both menu entries.

PortMaster is detected at `/mnt/ports/PortMaster`, `/PortMaster`, the usual
`/opt/...` locations, or the user data directory. An inherited `controlfolder`
can override detection. Older stock control scripts without `DEVICE_ARCH` use
the host's ARM64/x86-64 architecture; optional `pm_*` helpers are used when present.

Logging starts **before** PortMaster setup. If the game directory is missing,
diagnostics go to `yokoi-launch.log` alongside the launcher instead. Normal
`yokoi/log.txt` includes startup errors and the executable's exit status and is
overwritten on each launch. For a full stock RGDS shell trace over SSH:

```sh
bash -x /mnt/mmc/Ports/Yokoi.sh > /tmp/yokoi-launch.log 2>&1
cat /tmp/yokoi-launch.log
```

Audio requests stereo output and allows SDL to negotiate the device's frequency
and channel count; the CPU's mono sound is converted to that output format.
Some handheld ALSA devices reject mono output requests. The log records the
opened frequency and channel count, or `Audio disabled` if opening fails.
An audio failure does not prevent play, but there will be no sound.

At startup, SDL's display count is detected. With two or more displays, Yokoi
opens fullscreen windows on the first two and uses the same panel policy as
the [Android layout](../../android/app/src/main/cpp/core/yokoi_layout.cpp) and
[3DS renderer](../../source/virtual_i_o/3ds_screen.cpp):

| Mode | Top display | Bottom display |
| --- | --- | --- |
| Selection menu | Game details and controls | Console artwork |
| Single-screen game | Game LCD | Console artwork |
| Horizontal two-screen game | Both LCDs side by side | Console artwork |
| Clamshell two-screen game | LCD 0 | LCD 1 |

Panels preserve artwork proportions, except for the small clamshell correction
described below. Missing console artwork leaves
the bottom panel empty. On stock Linux RGDS, SDL display 0 is the physical top
panel and display 1 is the bottom panel. This differs from Android's default
display ordering but produces the same physical top/bottom layout.

The video driver, SDL display count, and chosen mode are recorded in `log.txt`.
Detection is performed at startup, not on hotplug. The SDL video backend must
expose both displays and support windows on each; two physical panels alone
do not guarantee this. If SDL reports one display or cannot initialize the second
window/renderer, Yokoi logs the reason and composes the same top/bottom panels
on one display. In-game settings can temporarily enlarge the top panel only.
On one display, each panel is fitted into half of the actual output height,
including square 720x720 screens, instead of a letterboxed 640x480 outer canvas.
Fitting uses the actual LCD/console bounds, excluding padding inside the render
targets. Landscape clamshell LCDs that fit the display width therefore fill each
half vertically and meet at the middle, without cropping artwork.
Clamshell LCDs whose aspect ratios differ by at most 3% share one fitted size,
correcting small extraction differences such as Donkey Kong II's 638x400 and
639x403 LCDs. Backgrounds and segments receive the same small correction on both
single- and dual-display layouts. More dissimilar LCDs retain their proportions.
On an odd-height single display, one spare row is preferable to unequal LCD sizes.
Side padding can remain when stacking two
landscape panels on a square display. Window resizing updates this fit.
Top screen only fits the actual LCD bounds against the full output, not a 4:3
outer viewport. Wide layouts, including Donkey Kong Hockey's combined LCD,
use the full width of a 1280x720 display; remaining aspect-ratio bars are not cropped.
This mode preserves the original LCD proportions and resets when leaving the game.
With two displays, the saved Swap
screens option exchanges panel content; settings remain on
the physical bottom display.

## Controls

Labels below follow **SDL controller names**, which may differ from physical
Nintendo button labels. PortMaster supplies each device's SDL mapping.

| Input | Menu | Playing |
| --- | --- | --- |
| D-pad / left stick | Left/right: game; up/down: manufacturer | Left-side game controls |
| A | Open/confirm fresh or saved-state choice | Right/action input |
| B | Cancel start choice | Right/down input |
| X | — | Right/up input |
| Y | — | Right/left input |
| Start | — | Game A |
| Select | — | Game B |
| L | — | Time |
| R / Guide / Back | Open/close settings | Open/close settings (pauses game) |
| L + B | — | Save and return to menu |
| Select + Start | Save and exit | Save and exit |

Desktop: arrows; Z=A, X=B, S=X, A=Y; Enter=Start, Tab=Select, Q=L.
R, F1 or Escape toggles settings; B also closes them. In settings, up/down
selects a row, left/right changes its value and A activates a button.
Normal exit uses Select+Start or window close, saving the running game first.
Selection/start-choice settings also offer Exit App.

### SpruceOS / TrimUI

Spruce's Ports controller mode defaults to X360. In Spruce's Ports list,
highlight Yokoi without launching it, press **X** to open its configuration,
then choose **Controller Mode → Nintendo** to match all printed face-button
labels. Yokoi uses the supplied SDL mapping without an additional button swap.

Menu/SDL Guide and R open Yokoi settings; Select remains Game B and Start+Select
saves and exits. Menu and Start+Select work in the currently tested Spruce setup;
no Menu workaround is needed there. Yokoi does not disable the system watchdog
or modify firmware settings.

The selection panel shows manufacturer, game name and year in the Android
layout. Left/right cycles only the current manufacturer's games; up/down changes
manufacturer, skipping missing groups and restoring each group's last selection.
A opens the Android-style fresh/saved-state prompt. Left/right toggles the choice,
A confirms and B cancels. An existing saved state is selected by default.

### Settings and persistence

Linux version 2.5 appears in the settings footer and startup log.
Run `./yokoi.aarch64 --version` (or `./build/yokoi --version` on desktop)
to print `Yokoi Linux 2.5` without loading a ROM pack or initializing SDL.
The Linux version is defined once in `linux/core/yokoi_version.h`;
Android versioning remains independent.

Both selection and in-game settings offer:
- Segments: None (0), Light (5), Dark (10).
- Background: Light Yellow, Light Blue, Greyish, White, Light Green or Cream.
- Swap screens when a second display is active (off by default).

In-game settings additionally offer Quit Game (save and return to selection),
and Top screen only / Show both screens on a single display. Opening settings
pauses emulation and clears queued audio; closing resumes play.

Touch/mouse input follows Android's bottom-screen navigation and game-action
regions; Linux uses physical controllers/keyboard rather than an on-screen
button overlay. Confirmation presses are released/suppressed before
gameplay so choosing a game does not accidentally press its action button.

Under `ports/yokoi/conf/` (or the supplied desktop storage directory):
- `yokoi_gw_settings.dat`: shared segment/background settings.
- `yokoi_gw_last_game_by_mfr.dat`: last manufacturer and each manufacturer's
  selected game reference. Boot restores the last selection, even after pack
  reordering; a new profile prefers `JR_55`, otherwise the first game.
- `yokoi_linux_preferences.dat`: screen swap (`YLP1`).
  Both `YLP1` and retired `YLP2` files retain screen swap; their retired
  controls/A/B-swap byte is ignored. New writes set that byte to zero.
- `yokoi_gw_saves/`: per-game CPU saved states.

Top-only mode is deliberately temporary. Invalid settings files produce warnings;
failed settings writes are reported rather than silently discarded.

### Automatic clock

Games with known clock RAM mappings initialize from the device's local time.
Like Android, time is reapplied during ROM warm-up so startup cannot latch the
default 12:00 clock, and after loading a saved state so its old clock is replaced.
The selected time and timezone (or missing clock mapping) are logged at startup.
Check `date` over SSH: incorrect device time/timezone also affects the game clock.

## Verification and remaining work

Verified in this workspace on x86_64 Linux with SDL2 2.30:
- Production executable builds; `--help` and `--version` work without a ROM pack.
- ARM64 executable cross-compiles with cached Ubuntu Focal GNU GCC 10 and SDL2
  2.26.2; ELF architecture and cache reuse are verified.
- Four original synthetic ROM/PNG packs (single, vertical, horizontal, mask) exercise
  atlas orientation, PNG basename lookup, menu, CPU stepping, input mapping,
  SDL audio conversion (including a stereo-only device regression), rendering,
  save/load and title switch/resume.
- Interleaved and reordered multi-manufacturer packs verify navigation, first-boot
  defaults, stable-reference restoration and saved per-manufacturer selections.
- An original SM510 startup program verifies that ROM clock overwrites do not
  latch a stale display value and that restored saves use the current device time.
- Settings tests cover context-specific options, all segment/background presets,
  paused/resumed CPU state, saved swaps/legacy preferences, touch input, temporary top-only
  mode, menu text/segment pixels, corrupt settings and failed writes.
  Selection/prompt Exit App is tested with controllers and pointers.
- Font checks verify proportional metrics, UTF-8 text, embedded licensing and
  antialiased glyph pixels using real SDL rendering. They simulate unavailable
  firmware character conversion and reject malformed/overlong UTF-8.
- Wrong-platform packs, old packs and missing packs are rejected.
- Virtual-controller checks cover button mappings, pressed/held edges, stick
  thresholds and hotplug; the real SDL window/event loop is exercised on exit.
  A Guide+R chord is checked for duplicate menu presses.
- Dual-panel pixel checks cover menus, gameplay, console restoration, horizontal
  games, clamshell LCD isolation, independent aspect ratios and missing artwork.
  Window pixel checks verify that menu details go to display 0 and artwork to display 1.
  Mocked display enumeration covers 0/1/2/3 displays and second-window, placement,
  renderer and logical-size failures, using real SDL windows/renderers.
- Real SDL pixel checks cover 720x720 and odd-height square composition,
  artwork proportions, full-output modal dimming and top-only fitting.
  Window resizing covers square, 4:3 and widescreen outputs plus transformed
  pointer activation of settings using the software renderer. Landscape LCD
  tests check top/bottom edges, the center seam, fractional bounds and preserved
  image edges. Shared-size tests cover Donkey Kong II and Black Jack dimensions,
  the 3% correction boundary, and combined/separate displays. Fixed-size 1280x720
  tests verify full-width wide/horizontal LCDs and a 1152x720 fit for 16:10 LCDs.
  Fixed-size square and widescreen windows also exercise offscreen OpenGL;
  handheld GPU behavior still needs device confirmation.
- Launcher passes `bash -n`.
- Launcher tests cover Ports/APPS paths, early logging, old/new control scripts,
  missing files, architecture detection and executable exit status.

Startup, sound and the corrected top/bottom routing have been confirmed on stock
RGDS. The new menu/settings behavior still needs on-device confirmation.
This is a development port, **not a broadly
tested handheld release**. Real-game accuracy, audio quality, display ordering,
controller mappings, CFW compatibility and performance still require testing.
The menu/settings behavior is a controller-friendly SDL adaptation:
the Audiowide font and widgets are not pixel-identical to Android's system UI.
Android platform features such as camera, pack-import activities and stereoscopic
3D are not implemented. LCD marking, shadow and tint passes follow Android.
The existing core's known emulation issues remain applicable.

Before PortMaster submission: produce target binaries, include required library
licenses, add real-device screenshot/metadata (`port.json`, `gameinfo.xml`), and
complete the official community testing process. This folder is an installation
staging directory, not a submission-ready PortMaster archive.

## Synthetic smoke check (no commercial data)

```sh
make -C linux check
# Cross-build orchestration and toolchain cache/checksum tests (mocked tools):
make -C linux check-cross-build
make -C linux check-launcher
# To keep frames for visual inspection:
mkdir -p /tmp/yokoi-smoke
python3 linux/tests/make_fixture.py /tmp/yokoi-smoke/fixture.ykp
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
  ./linux/build/smoke-test /tmp/yokoi-smoke/fixture.ykp /tmp/yokoi-smoke
```

The smoke executable also writes menu/game BMP frames for visual inspection.

## Source and licensing

Based on RetroValou/Yokoi_Game_-_Watch_emulator_3ds at
`f273025cb372c2924e176c04ef821abafd080a17`.
Upstream README declares Public Domain / Free to use. New frontend code is made
available on the same terms. stb_image and stb_truetype retain their embedded
MIT/public-domain license notices. SDL2 is dynamically linked; include its license
if redistributing its library. ROMs and artwork must be supplied separately.
Audiowide is separately licensed under SIL OFL 1.1; its copyright notice and
licence are in [Audiowide-OFL.txt](../third_party/Audiowide-OFL.txt) and embedded
in each executable. stb_truetype is pinned to upstream revision
`003501f6d7a8668ce8b6d875e34dd97ad0d5d5b5`.
