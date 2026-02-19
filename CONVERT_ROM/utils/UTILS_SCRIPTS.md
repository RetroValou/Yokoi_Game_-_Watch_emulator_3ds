# CONVERT_ROM utils scripts

This folder includes utility scripts that are **not required** for normal conversion/build flows, but can be useful for development and maintenance.

## Links

Useful sites with large databases on 1980s and 1990s LCD Games:
- http://handheldempire.com/ - DB on LCD games
- https://electronicplastic.com/ - DB on LCD games
- https://www.lucklessheaven.com/gameandwatch-release-order - Details on Game & Watch releases

## Scripts

### extract_games_from_mame_dat.py

Purpose: Generates a Markdown table of handheld LCD systems from the ProgettoSnaps MAME DAT pack.

This script:
- Downloads the ProgettoSnaps `MAME_Dats_XXX.7z` pack for a hardcoded MAME version.
- Extracts the best matching DAT inside the pack (prefers `MAME <version>.dat`).
- Parses machines whose `machine@sourcefile` matches `handheld/hh_sm510.cpp`.
- Derives:
  - `Filename`: `machine@name + .zip`
  - `Game Title`: `<description>`
  - `CPU`: from `<device_ref name="...">`
  - `Model`: from the first `<rom name="...">` (with a small override mapping)
- Cross-references Nintendo release dates from Luckless Heaven and formats dates as `YYYY.MM.DD`.
- Ensures the `Model` column is unique: when multiple sets share the same ROM/model, non-original sets get a `-<setname>` suffix (using `machine@romof`).

Usage:

```powershell
python extract_games_from_mame_dat.py
```

Notes:
- Writes output to `CONVERT_ROM/utils/EXTRACTED_LIST_FROM_MAMEDAT.md`.
- Caches downloads/extracted files under `CONVERT_ROM/tmp/`.
- If you want to bump the version, edit `MAME_DAT_VERSION` at the top of `extract_games_from_mame_dat.py`.
- If you want to override model codes, edit the `SET_MODEL_OVERRIDES` dict at the top of `extract_games_from_mame_dat.py`.
- Downloads currently run with TLS verification disabled (`ALWAYS_INSECURE_TLS = True`) to work around Windows/Python CA issues.
