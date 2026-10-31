# Yokoi - Game & Watch Emulator for 3DS, Android & PortMaster

A Game & Watch emulator for 3DS, Android & Linux/PortMaster: SM5A, SM510, and SM511/SM512.

<img src="screenshot/all.png" alt="Gameplay"/>

# Supported devices

- **Nintendo 3DS/2DS family** (Old/New models) running Homebrew/CFW.
- **Anbernic RGDS (dual screen, Android)**: supported and recommended. 
- **AYN Thor (dual screen, Android)**: supported. (Thanks [ABerguerand](https://github.com/ABerguerand))
- **Android (arm64-v8a)** devices (phones/tablets/handhelds).
- **Linux handhelds via PortMaster**: tested on SpruceOS, MustardOS, ArkOS, and
  Anbernic RGDS running Linux. The PortMaster version supports single and dual-screen devices,
  including RGDS. Single-screen devices display both panels on one screen.

# Supported games

Supported games are all of the Game & Watch titles and some Tronica and Elektronika games.

- Full list: [CONVERT_ROM/GNW_LIST.md](/CONVERT_ROM/GNW_LIST.md)
- Compatible Game & Watch models are those equipped with Sharp processors SM5A, SM510, SM511, SM512
	- Refers to Game & Watch models released by Nintendo between 1980 and 1991
- Not compatible with Game & Watch Mini (SM530 processor)
- Notes:
	- Availability depends on which ROMs/artwork you provide when generating your rompack.
	- Some games may still have quirks (see [Known issues](#known-issues)).

# Features

- Dual-screen support for double-screen Game & Watch on 3DS, Android dual-screen devices, and Linux/PortMaster dual-screen devices (including Anbernic RGDS)
- Save state support
- Time synchronization
- Stereoscopic 3D on 3ds
- Camera support for Game & Watch Crystal Screen games on 3ds (reproduces the "transparent" screen)
- Ajustable LCD Segment Shadow
- Optional Full screen (Android only)
- Optional Virtual Input (Android only) 
- Optional Swap screens for dual-screen devices (Android and Linux/PortMaster)
- Optional Top screen only mode (Android and Linux/PortMaster)

# Installation

Installation requires both the application and a rompack with the games. 

**Note:** The rompack is **not provided** as it contains original Game & Watch ROMs and graphics. You will need to build your own. Details are in the [CONVERT_ROM/README.md](/CONVERT_ROM/README.md)

## 3DS

The emulator is provided as `.cia` and `.3dsx` files for 3DS.

### Install (CIA)

Prerequisites:

- A 3DS with CFW (e.g. Luma3DS) so you can install CIAs.
- A title installer such as **FBI**.

Steps:

1. Copy `Yokoi*.cia` to your SD card (any folder).
2. Launch **FBI** on the 3DS.
3. Go to **SD** -> find the `.cia` -> choose **Install and delete CIA** (or Install).
4. Launch **Yokoi** from the HOME Menu.

### Install (3DSX)

Prerequisites:

- The Homebrew Launcher.

Steps:

1. Copy `Yokoi*.3dsx` (or the release `.3dsx`) to `sdmc:/3ds/yokoi/`.
2. Launch the Homebrew Launcher and start **Yokoi**.

### ROM pack 

The rompack is **not provided**, you will need to generate your own from MAME roms.

1. Generate a 3DS rompack as detailed in the [CONVERT_ROM/README.md](/CONVERT_ROM/README.md)
2. Copy your generated pack file to:
	- `sdmc:/3ds/yokoi_pack_3ds.ykp`

If the emulator starts but shows no games or reports a missing pack, double-check the filename and location exactly match the path above.

## Android

The emulator is provided as an `.apk` in the latest releases section.

### Install (APK)

1. Download the latest `Yokoi*.apk` from the releases.
2. Enable installing from unknown sources (wording depends on Android version):
	- Settings -> Security/Privacy -> Install unknown apps -> allow your browser/file manager.
3. Open the APK and install it.
4. Launch **Yokoi**.

### ROM pack (Android)

The rompack is **not provided**, you will need to generate your own from MAME roms.

1. Generate an Android rompack as detailed in the [CONVERT_ROM/README.md](/CONVERT_ROM/README.md)
2. Copy the generated rompack `yokoi_pack_rgds.ykp` somewhere accessible (e.g. `Download/`).
3. Launch the app and use **Import/Update ROM pack** option shown to select the file.

## Linux / PortMaster

Use a PortMaster setup supported by your device's firmware. Tested platforms are
SpruceOS, MustardOS, ArkOS, and Linux on Anbernic RGDS.

The PortMaster ZIP includes the launcher, executable, and all required support
files. No compilation is needed; the rompack must be supplied separately.

1. Download and extract the provided **Yokoi PortMaster ZIP**.
2. Copy the entire extracted `yokoi/` folder into your device's `ports/`
   directory. On most firmwares, place `Yokoi.sh` alongside it, but some firmwares
   require the launcher in a separate apps/launcher directory instead. Follow
   your firmware's layout.
3. Generate an **Android/RGDS rompack** as detailed in
   [CONVERT_ROM/README.md](/CONVERT_ROM/README.md), then drop
   `yokoi_pack_rgds.ykp` into `ports/yokoi/`. Use content version 3 or later.
   The 3DS rompack is not compatible.
4. Launch **Yokoi** from the device's Ports menu.

Typical installed layout (the launcher location may differ by firmware):

```text
ports/
  Yokoi.sh
  yokoi/
    yokoi.aarch64
    yokoi_pack_rgds.ykp
    licenses/
```

The executable name above is for ARM64 devices. Saves are stored under
`ports/yokoi/conf/`, and launch diagnostics are written to `ports/yokoi/log.txt`.
Dual-screen support is automatic when SDL exposes both displays. Otherwise,
both panels are composed on one screen.

# Building

See [BUILDING.md](BUILDING.md) for how to build the 3DS, Android, and Linux/PortMaster applications.

For ROM pack / asset generation, see [CONVERT_ROM/README.md](/CONVERT_ROM/README.md).

# Known issues
- Very imperfect sound / strange resonance on Game & Watch SM510 (e.g., Donkey Kong JR Widescreen)
- Bug on some Game & Watch SM5A (sometimes freezes at the end of a game)


# License
Public Domain / Free to use  
No attribution required :)

# Credits
Code inspired by MAME, Game & Watch FPGA projects (Adam Gastineau), and official Sharp documentation.  
ROMs and artwork are based on MAME ROMs.  
Thanks to everyone who contributed to MAME, Adam Gastineau, and all those who worked on Game & Watch ROMs and artwork!  
Thanks to [ABerguerand](https://github.com/ABerguerand) for the Swap screens option and AYN Thor support!
