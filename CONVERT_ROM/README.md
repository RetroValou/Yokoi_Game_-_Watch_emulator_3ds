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

If you adjust per‑game processors or add new games, re‑run `convert_original.py` first so that `games_path.py` and the processed assets are up to date, then run `convert_3ds.py` again to regenerate the 3DS build data.
