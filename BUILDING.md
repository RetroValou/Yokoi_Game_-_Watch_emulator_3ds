# Building Yokoi

This BUILDING.md readme covers building the applications (3DS + Android).

To build the application you will need to generate the assets/rompack first. See [CONVERT_ROM/README.md](/CONVERT_ROM/README.md) for details.

Building has only been tested on Windows.

If you are building the applications yourself you can choose to build either an **embedded** or **rompack** build. The embedded build has all the assets and roms built into the application and the rompack build does not, which then requires the separate rompack to also be built and installed.

## 3DS (devkitPro)

### Prerequisites

- [devkitPro](https://devkitpro.org/wiki/Getting_Started) with **devkitARM**

### Build (rompack build) (default)

From the repo root:

- First build Game & Watch assets needed. See: [CONVERT_ROM/README.md](/CONVERT_ROM/README.md)
- Builds both `.3dsx` and `.cia`:
    - `make clean` (Optional)
	- `make`

Output files include `rompack` in their names.

### Build (embedded)

This mode builds an app that includes ROMs/assets embedded in the application.

- First build Game & Watch assets needed. See: [CONVERT_ROM/README.md](/CONVERT_ROM/README.md)
- Builds both `.3dsx` and `.cia`:
    - `make clean` (Optional)
    - `make EMBEDDED=1`

## Android (Android Studio / Gradle)

### Prerequisites

- Android Studio (or command-line Android SDK)
- Android NDK + CMake (the project is configured for CMake 3.22.1)

### Build in Android Studio

1. First build Game & Watch assets needed. See: [CONVERT_ROM/README.md](/CONVERT_ROM/README.md)
2. Open the `android/` folder in Android Studio.
3. Use **Build Variants** to select `rompackOnlyDebug` (default) or `embeddedDebug`.
4. Build/run from Android Studio.
