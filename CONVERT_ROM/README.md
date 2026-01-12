# CONVERT_ROM tools

This folder contains helper scripts for converting original MAME artwork and ROM data into the format required by the 3DS Yokoi Game & Watch emulator.

## Overview

### Summary

There are two main scripts:

* `convert_original.py` - This is used first to convert the MAME original files to the data files this emulator needs.
* `convert_3ds.py` - This then takes the prepared files, and converts them for use on the 3DS

### What do you need

You will need:

* ROM and Artwork files from MAME. (Version 0.282 data was used at the time of writing this)
  * ROM zip files are placed in the `CONVERT_ROM\rom\roms` folder
  * Artwork zip files are placed in the `CONVERT_ROM\rom\artwork` folder
* A screenshot of each console. The easiest way to get this is to use MAME on the PC with the ROMs and Artwork above. Then start each game and take a screenshot. Save each screenshot using the same name as the ROM zip file. (e.g. gnw_ball.png)
  * Place all of these screenshots into the `CONVERT_ROM\rom\console` folder

It is recommended to use a Python virtual environment, and you can install the required python modules with the requirements.txt

## Main scripts

### `convert_original.py`

End‑to‑end helper that prepares everything starting from raw MAME assets.

It performs the following steps in order:

1. Extracts all the assets from the ROMs, Artwork and console files, creating per‑game folders `rom/gnw_<game>` and unpack and move the needed files there.
2. Scans all the unpacked assets and generates a basic `games_path.py` which is the main file the emulator uses to understand the assets. 
3. Runs a post processor which can make adjustments to any settings for the games. e.g: 
   - `CrabGrabGameProcessor` for `gnw_cgrab`.
   - `SpitballSparkyGameProcessor` for `gnw_ssparky`.
   
   These post processors:
   - Tweak backgrounds (e.g. combine multiple PNG layers).
   - Add colour indices into SVG segment titles.
   - Update the corresponding entries in `games_path.py`.
   - Set special flags if needed

Typical usage from this folder:

```powershell
python convert_original.py
```

Use the `--target` command line to generate the correct settings.

```
# Original 3DS-sized outputs
python convert_original.py --target 3ds

# RG DS / 640x480-class: generates higher-res source PNGs + updated metadata
python convert_original.py --target rgds
```

If any files are missing, `generate_games_path()` prints details and exits with an error.

### `convert_3ds.py`

This script takes the prepared assets (ROMs, PNG backgrounds, SVG visuals, `games_path.py`) and converts them into the packed binary data used by the 3DS emulator build.

> Note: run `python convert_3ds.py -h` for the help information and command line options.

Common command‑line arguments:

- `-h`, `--help`
  - Show help information and exit.
- `--game NAME`, `-g NAME`
  - Only rebuild a single game.
  - Use the key from `games_path.py` (for example `Crab_Grab` or `Fire`).
- `--parallel`
  - Enable multiprocessing when building multiple games. (Not well tested)
- `-c`, `--clean`
  - Delete and regenerate `./tmp/img/<game>` before processing. If combined with `-g` only the single game data will be deleted.

Example invocation:

```powershell
python convert_3ds.py

# Convert only Crab Grab, and clean away files first
python convert_3ds.py -c -g Crab_grab
```

### Targets / screen profiles

The script supports two main targets, 3DS and RG DS scaling the graphics to suit each device.
The settings for each are now centralized in `CONVERT_ROM/source/target_profiles.py` and selected via `convert_3ds.py --target`.

Common usage:

```powershell
# Original 3DS-sized outputs
python convert_3ds.py --target 3ds

# RG DS / 640x480-class: generates higher-res source PNGs + updated metadata
python convert_3ds.py --target rgds

```

### External ROM pack

An external rom pack is now build at the same time as the embedded rom files. This external rom pack can be used with both the Android and 3DS versions of the application that do not include any embedded assets.

#### Android RGDS pack:

```powershell
# Writes into the CONVERT_ROM folder as "yokoi_pack_rgds_vX.X.ykp".
python convert_3ds.py --target rgds
```

#### 3DS pack (v1):

```powershell
# Writes into the CONVERT_ROM folder as "yokoi_pack_3ds_vX.X.ykp".
python convert_3ds.py --target 3ds
```

The rom pack files above are named with the pack version and content version (i.e. vX.X)

Note: the 3DS pack bundles `.t3x` textures. `convert_3ds.py` will invoke `tex3ds` to build them, so devkitPro/devkitARM must be installed and `TEX3DS_PATH` (near the top of `convert_3ds.py`) must point to your `tex3ds` executable (or `tex3ds` must be on PATH).

#### Rom pack locations

Android will ask to import/update the rom pack when needed on start up, just select the file and it will be copied into place

3DS will inform you if the file is missing and it must be located in `sdmc:/3ds/yokoi_pack_3ds.ykp`

---

If you adjust per‑game processors or add new games, re‑run `convert_original.py` first so that `games_path.py` and the processed assets are up to date, then run `convert_3ds.py` again to regenerate the 3DS build data.
