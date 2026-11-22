# Yokoi conversion profiles compared

Run the commands below from `CONVERT_ROM` with the corresponding inputs
already prepared. All values are defaults. SVG export resolution, final
artwork size, and display resolution are three different things.

| What | 3DS | RGDS | RGDS Plus | RGDS hi-res |
| --- | --- | --- | --- | --- |
| Build command | `python convert_3ds.py --target 3ds` | `python convert_3ds.py --target rgds` | `python convert_3ds.py --target rgds --plus` | `python convert_3ds.py --target rgds --hires` |
| Design target | 3DS: 400x240 top, 320x240 bottom | Regular RGDS: two 640x480 panels; also the standard Android/Linux pack | RGDS Plus: two 1024x768 panels | More artwork detail for Android/Linux; no fixed physical display resolution |
| Default Inkscape SVG export | 50 DPI | 100 DPI (shared across RGDS variants) | 100 DPI (shared across RGDS variants) | 100 DPI (shared across RGDS variants) |
| LCD sizing before per-game aspect fit | Default top 400x240, bottom 320x240; side-by-side slot 200x240; clamshell slot 320x240 | Default 640x480 per LCD; side-by-side slot 320x480 | Starts with the standard RGDS fitted LCDs, then projects their frames to 1024x768 and snaps edges to pixels. Side-by-side LCDs share one panel | Doubles the RGDS size limits **before** aspect fitting: typically 1280x960 per LCD or 640x960 per side-by-side slot |
| Console artwork target | 320x240 in a 512x256 atlas | 640x480; standalone PNG | Standard RGDS crop, fitted to 1024x768; standalone PNG | 1280x960; standalone PNG |
| Integer artwork divisor (`segment_info[2]`) | `1` | `1` | `1`; fitted art coordinates are stored in pixels | `2`; the runtime divides artwork sizes and coordinates by two |
| Runtime coordinates | 3DS screen-sized drawing coordinates | Linux uses a 640x480 logical frame and fits the art. Android uses art-derived game canvases and a 640x480 top menu canvas | Same logical UI as standard RGDS. On separate 1024x768 Linux panels, artwork is drawn in physical pixel coordinates after fitting | Same runtime layout rules as standard RGDS; the divisor keeps the larger art near the standard logical size |
| Segment / background atlas candidate ceilings | 1024 / 1024 | 2048 / 2048 | 8192 / 4096 | 8192 / 8192 |
| Generated artwork folder (repo root) | `gfx`; `tex3ds` compiles textures to `romfs/gfx` | `gfx2x` | `gfx2.6x` | `gfx3x` |
| Texture assets in pack | `.t3x` | `.png` | `.png` | `.png` |
| Embedded C++ by default | Yes: `GW_ALL.h` and `GW_ROM` | Yes: `GW_ALL_rgds.h` and `GW_ROM_RGDS` | No, pack-only | No, pack-only |
| Canonical pack (in `CONVERT_ROM`) | `yokoi_pack_3ds.ykp` | `yokoi_pack_rgds.ykp` | `yokoi_pack_rgds2.6x.ykp` | `yokoi_pack_rgds3x.ykp` |
| Pack platform ID | `1` (3DS) | `2` (RGDS) | `2` (RGDS) | `2` (RGDS) |

The atlas ceilings are packing limits, not the dimensions of each LCD or
every texture. All four profiles also write a versioned copy with `_v3.3`
before `.ykp` (current format version 3, content version 3). For PortMaster,
install whichever RGDS variant you want as `ports/yokoi/yokoi_pack_rgds.ykp`.

## Example final LCD sizes

These are converter-calculated artwork dimensions before atlas padding,
not intermediate SVG export sizes or measurements of a running pack.
Backgrounds and segments align on the same LCD canvas.

| Game | 3DS | RGDS | RGDS Plus | RGDS hi-res |
| --- | --- | --- | --- | --- |
| Ball (one LCD) | 365x240 | 639x420 | 1024x674 | 1279x840 |
| Donkey Kong (clamshell, per LCD) | 318x201 | 639x403 | 1024x646 | 1278x806 |
| Mario Bros. (side-by-side, per LCD) | 198x127 | 319x204 | 512x328 | 639x409 |

On the Plus, Ball leaves 47 pixels above and below its LCD. Donkey Kong
leaves 61 pixels above and below on each panel. Mario Bros. puts both LCDs
side by side in a single 1024x328 frame with 220 pixels above and below.
The hi-res sizes are not always exactly double the standard fitted sizes:
doubling the size limit before fitting changes where integer rounding happens.

## What to check when choosing a pack

- **DPI controls the intermediate SVG raster, not the artwork size preset.**
  All three RGDS variants share intermediate PNGs when the source SVGs and
  DPI match, even without `--use-cache`. The first run fills the new shared
  cache; old exports with unknown DPI are not reused. `--export-dpi` selects
  a separate RGDS cache entry, and `--clean` forces a fresh export.
  The 3DS workflow keeps its separate 50 DPI default and existing export
  cache; use `--clean` when changing its DPI. `--use-cache` is separate
  from SVG reuse: it can also skip rebuilding unchanged final artwork.
- **Plus is a physical-pixel target, not the hi-res preset.** `gfx2.6x` and
  `gfx3x` are names, not scale factors stored in the pack. The Plus renderer
  path preserves standard RGDS framing and targets 1:1 drawing for LCD
  backgrounds, segments, and the console on separate 1024x768 Linux panels.
  Other resolutions, the stacked Linux layout, and Android can resample the
  artwork. The Plus pack uses the same RGDS format as the standard pack.
- **Hi-res adds larger output textures, not a higher Inkscape DPI.** If an
  input raster is smaller than the requested output, the converter resizes
  it; a larger texture does not guarantee more source detail.

See [the converter README](README.md) for setup and build instructions. The
sizing comes from [target profiles](source/target_profiles.py) and
[the converter](convert_3ds.py); runtime fitting comes from the
[Linux renderer](../linux/render/yokoi_render.cpp) and
[Android layout](../android/app/src/main/cpp/core/yokoi_layout.cpp).
