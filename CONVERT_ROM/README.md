# CONVERT_ROM tools

Helper scripts for converting  MAME ROMs + artwork data (version 0.282 as of writing) into Yokoi assets, roms, graphics and rompacks, used by the 3DS and Android versions of the emulator.

This repo does **not** include original Game & Watch ROMs or artwork. You need to find them yourself.

If you are looking for how to build the apps themselves, see [BUILDING.md](/BUILDING.md).

## Table of contents

- [CONVERT\_ROM tools](#convert_rom-tools)
  - [Table of contents](#table-of-contents)
  - [Supported games](#supported-games)
  - [Manufacturer grouping (menu)](#manufacturer-grouping-menu)
  - [Prerequisites](#prerequisites)
    - [Python](#python)
    - [External apps](#external-apps)
    - [ROMs, artwork, screenshots](#roms-artwork-screenshots)
      - [Generating console png screenshots with MAME](#generating-console-png-screenshots-with-mame)
  - [Build assets, roms, graphics and rompacks](#build-assets-roms-graphics-and-rompacks)
    - [3DS](#3ds)
    - [Android (RGDS)](#android-rgds)
      - [Additional notes](#additional-notes)
  - [Advanced script descriptions and options](#advanced-script-descriptions-and-options)
    - [`convert_original.py`](#convert_originalpy)
    - [`convert_3ds.py`](#convert_3dspy)
  - [External ROM pack](#external-rom-pack)
    - [Rom pack locations](#rom-pack-locations)
  - [Developer SCripts](#developer-scripts)

## Supported games

Supported games are all of the Game & Watch titles and some Tronica games.

Full list: [GNW_LIST.md](/CONVERT_ROM/GNW_LIST.md)

Notes: You do not have to include all games, you only need to include the games you want to be part of the emulator/rompack.

## Manufacturer grouping (menu)

The 3DS and Android menus group games by manufacturer automatically.

- **Left/Right**: change game within the current manufacturer
- **Up/Down**: switch manufacturer group

## Prerequisites

### Python

- Python (recommended: use a virtual environment)
- Install dependencies from this folder:

```powershell
cd CONVERT_ROM
python -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install -r requirements.txt
```

### External apps

Some steps call external executables:

- **Inkscape**: exports SVG layers to PNGs. https://inkscape.org/
- **tex3ds**: 3DS target only -> builds `.t3x` textures from generated `.t3s`.
  - Comes with devkitPro: https://devkitpro.org/wiki/Getting_Started

Configure external paths:

1. Copy `external_apps_template.py` to `external_apps.py`
2. Edit `INKSCAPE_PATH` and `TEX3DS_PATH` to match your system

### ROMs, artwork, screenshots

You will need the following MAME data (tested with **MAME 0.282** at the time of writing):

- ROM zip files -> put them in `rom/roms/`
- Artwork zip files -> put them in `rom/artwork/`
- A screenshot for each console (one per game), named like the ROM zip (e.g. `gnw_ball.png`) -> put them in `rom/console/`
    - Generate with MAME, see below.

#### Generating console png screenshots with MAME

Console screen shots are not provided as part of the MAME artwork. The easiest way to get the required console screenshots is to use MAME itself on Windows:

1. Set up MAME with the same ROMs/artwork you will be converting.
2. Launch each game in MAME.
3. Once the game is displaying the handheld/console artwork, take a screenshot.
     - Default MAME hotkey is commonly `F12` (can vary by build/config).
4. Find the screenshot file in your MAME screenshot output folder (often a `snap/` folder under your MAME directory, or whatever `snapdir` is set to in `mame.ini`).
5. Rename the screenshot to match the ROM zip name in [GNW_LIST.md](/CONVERT_ROM/GNW_LIST.md) (example: `gnw_ball.png`) and place it into `rom/console/`.

## Build assets, roms, graphics and rompacks

Run these commands from the `CONVERT_ROM/` folder and will simultaneously build all assets for the embedded build and rompacks for rompack releases.

### 3DS

```powershell
cd CONVERT_ROM
python convert_original.py --target 3ds
python convert_3ds.py --target 3ds
```

Outputs:

- External rompack written into `CONVERT_ROM/` as `yokoi_pack_3ds.ykp` and `yokoi_pack_3ds_vX.X.ykp`. 
    - Both files are identical, but one includes versioning for convenience.

### Android (RGDS)

```powershell
cd CONVERT_ROM
python convert_original.py --target rgds
python convert_3ds.py --target rgds
```

Outputs:

- External rompack written into `CONVERT_ROM/` as `yokoi_pack_rgds.ykp` and `yokoi_pack_rgds_vX.X.ykp`.
    - Both files are identical, but one includes versioning for convenience.

#### Additional notes

- Rompacks are versioned, so if there are changes to the format you will need to build a new rompack
- Sorting order of games in the menu can be adjusted as desired in the rompack with the `--sort {none,key,display_name,date,ref}` (and `--sort-reverse`) option. The default is alphabetical order.
    - e.g. `python convert_3ds.py --target rgds --sort date`
- 3DS and Android assets are stored in separate folders so the scripts can be run safely while switching between targets

## Advanced script descriptions and options

### `convert_original.py`

End-to-end helper that prepares everything starting from raw MAME assets.

It performs the following steps in order:

1. Extracts assets from the ROMs, artwork and console files, creating per-game folders `rom/gnw_<game>` and unpacking/moving the needed files there.
2. Scans the unpacked assets and generates a basic `games_path.py` which is the main file the emulator uses to understand the assets.
3. Runs per-game post processors which can adjust settings (examples):
   - `CrabGrabGameProcessor` for `gnw_cgrab`.
   - `SpitballSparkyGameProcessor` for `gnw_ssparky`.

These post processors can:

- Tweak backgrounds (e.g. combine multiple PNG layers).
- Add colour indices into SVG segment titles.
- Update the corresponding entries in `games_path.py`.
- Set special flags if needed.

Typical usage:

```powershell
python convert_original.py --target 3ds
  
python convert_original.py --target rgds
```

### `convert_3ds.py`

Takes the prepared raw MAME assets (ROMs, PNG backgrounds, SVG visuals, `games_path.py`) and converts them into both assets for an embedded build and the rompack data.

The script supports two main targets: `3ds` and `rgds`.

Typical usage:

```powershell
# Original 3DS-sized outputs
python convert_3ds.py --target 3ds

# RGDS / 640x480-class: generates higher-res source PNGs + updated metadata
python convert_3ds.py --target rgds
```

Command-line arguments:

- `-h`, `--help`
  - Show help information and exit.
- `--game NAME`, `-g NAME`
  - Only rebuild a single game.
  - Use the key from `games_path.py` (for example `Crab_Grab` or `Fire`).
- `--parallel`
  - Enable multiprocessing when building multiple games. (Not well tested)
- `-c`, `--clean`
  - Delete and regenerate `./tmp/img/<game>` cache files before processing. If combined with `-g` only the single game data will be deleted.
- `--use-cache`
  - **Experimental:** enables a per-game “up-to-date” cache to skip rebuilding games whose inputs/options and expected outputs have not changed.
  - When enabled, the script prints why a game is being rebuilt (missing outputs, changed inputs/options, invalid cache data) and then continues.
  - Cache validation uses content checksums for inputs (so timestamp-only changes won’t force rebuilds).
  - Cache files are stored under `./tmp/cache/`.
  - Ignored when `-g/--game` is used (single-game runs always rebuild).
  - If you have problems with your rompack, rebuild **WITHOUT** this option!
- `--sort {none,key,display_name,date,ref}`
  - Optional deterministic ordering for games in the generated ROM pack.
  - This affects the pack entry order, which is the menu order on 3DS/Android pack-only builds.
  - Default is `none`, which preserves the iteration order from the `games_path_<target>.py` dict.
- `--sort-reverse`
  - Reverse the selected `--sort` ordering.

Example command line argument usage:

```powershell
# Convert only Crab Grab, and clean away files first
python convert_3ds.py --target 3ds -c -g Crab_grab

# Build an RGDS pack with games sorted by display name (menu order)
python convert_3ds.py --target rgds --sort display_name

# Build a 3DS pack sorted by ref, descending
python convert_3ds.py --target 3ds --sort ref --sort-reverse
```

## External ROM pack

An external ROM pack is built at the same time as the embedded asset files. This external ROM pack can be used with both Android and 3DS builds that do not include embedded assets.

The rom pack files are named with the pack version and content version (i.e. `vX.X`).

### Rom pack locations

- Android: the app asks to import/update the ROM pack when needed; select the file and it is copied into place.
- 3DS: pack-only builds require `sdmc:/3ds/yokoi_pack_3ds.ykp`.

## Developer SCripts

For dev-only utility scripts (e.g. `extract_games_from_mame_dat.py`), see [utils/UTILS_SCRIPTS.md](/CONVERT_ROM/utils/UTILS_SCRIPTS.md).
