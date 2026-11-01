# Yokoi Linux/Portmaster Device Notes

Install types, setup steps and quirks for each firmware and device. Choose the
install type for your firmware, then check its notes below.

| Firmware | Tested on | Install type | What else you need to do |
| --- | --- | --- | --- |
| [Stock Linux on RGDS](#stock-linux-on-rgds) | RGDS | Normal | Set Controller layout to RGDS |
| [Knulli](#knulli) | RG40XX-H | Normal | Install PortMaster, then switch A/B and X/Y |
| [SpruceOS](#spruceos) | TrimUI Brick, TrimUI Smart Pro S | Normal | Set Controller Mode to Nintendo |
| [MustardOS (muOS)](#mustardos-muos) | RG CubeXX, RG40XX-H (older version) | Split |  |
| [ArkOS](#arkos) | RG351, RGB30 | Normal |  |
| [dArkOS](#darkos) | RGB30 | Normal | Install PortMaster |
| [ROCKNIX](#rocknix) | RG ARC-S | Normal |  |

## Install types

Both types need your own rompack. Generate an **Android/RGDS rompack** as described
in [CONVERT_ROM/README.md](../CONVERT_ROM/README.md), then copy
`yokoi_pack_rgds.ykp` into the `yokoi/` folder. Yokoi won't start without it, and
a 3DS rompack won't work. You can also use the hi-res ROM pack for devices with high resolution screens.

### Normal

`Yokoi.sh` and the `yokoi/` folder both go in the same `ports/` folder:

```text
ports/
├── Yokoi.sh
└── yokoi/
    └── yokoi_pack_rgds.ykp
```

### Split

`Yokoi.sh` and the `yokoi/` folder go in two different folders. On MustardOS,
both are on the same card:

```text
ROMS/
└── Ports/
    └── Yokoi.sh
ports/
└── yokoi/
    └── yokoi_pack_rgds.ykp
```

## Stock Linux on RGDS

Tested on an Anbernic RGDS.

The game folder is `/mnt/mmc/Ports/yokoi`. A launcher copied into
`/mnt/mmc/Roms/APPS` finds that same folder. If your device has both menu entries,
update both launcher copies.

SDL display 0 is the physical top panel and display 1 is the bottom panel. This
differs from Android's default display ordering but produces the same physical
top/bottom layout.

For a full shell trace over SSH:

```sh
bash -x /mnt/mmc/Ports/Yokoi.sh > /tmp/yokoi-launch.log 2>&1
cat /tmp/yokoi-launch.log
```

### Controller layout

The generic PortMaster mapping can interpret physical Select as SDL Guide and
physical Menu as SDL Back. Open settings with **R1** (or Select before correcting
this mapping), then set **Controller layout** to **RGDS**. The choice is saved
automatically; no file editing or launcher changes are needed.

RGDS layout exchanges the controller's SDL Back/Guide meanings so physical
**Menu opens settings**, **Select is Game B**, and **Start+Select saves and exits**.
Start, face buttons, shoulders, sticks and keyboard bindings remain unchanged.
This is an opt-in correction for the stock mapping: keep **Default** on other
handhelds, or on an RGDS whose firmware already maps Select/Menu correctly.
PortMaster's controller database is not modified.

## Knulli

Tested on an Anbernic RG40XX-H with the `scarab` build.

Knulli uses the [Normal](#normal) install type. Its `ports/` folder is `roms/ports`
on the `SHARE` partition, which is `/userdata/roms/ports` on the device.

**Install PortMaster first.** A freshly flashed card has PortMaster's installer but
not PortMaster itself, and nothing runs the installer for you. If `log.txt` says
`Missing PortMaster control.txt`, this step is still needed. Run **Install PortMaster**
from the **Additional Software** section of the Knulli menus.

**Switch the face buttons.** The controls are reversed until you change this
game option:

1. In the Ports list, highlight Yokoi.
2. Press and hold your confirm button (A or B) to open the per-game options.
3. Select **Advanced Game Options**.
4. Turn on **Switch A/B and X/Y**.

## SpruceOS

Tested on a TrimUI Brick and a TrimUI Smart Pro S.

SpruceOS uses the [Normal](#normal) install type. Its `ports/` folder is `Roms/ports`
on the card, which is `/mnt/SDCARD/Roms/ports` on the device.

Spruce's Ports controller mode defaults to X360. In Spruce's Ports list,
highlight Yokoi without launching it, press **X** to open its configuration,
then choose **Controller Mode → Nintendo** to match all printed face-button
labels. Yokoi uses the supplied SDL mapping without an additional button swap.

## MustardOS (muOS)

Tested on an Anbernic RG CubeXX and an Anbernic RG40XX-H (older MustardOS version).

MustardOS uses the [Split](#split) install type: `Yokoi.sh` goes in `ROMS/Ports`
and the game folder goes in `ports/yokoi`, including your rompack. The launcher
finds the game folder from there.

No extra setup needed.

## ArkOS

Tested on an Anbernic RG351 and a PowKiddy RGB30.

ArkOS uses the [Normal](#normal) install type. Its `ports/` folder is `/roms/ports`
on the device, or `/roms2/ports` if your games are on a second SD card.

No extra setup needed.

## dArkOS

Tested on a PowKiddy RGB30.

dArkOS uses the [Normal](#normal) install type. Its `ports/` folder is `ports` in
the `roms` folder.

**Install PortMaster first.** Run the PortMaster install from the Options menu,
then copy Yokoi across.

## ROCKNIX

Tested on an Anbernic RG ARC-S.

ROCKNIX uses the [Normal](#normal) install type. Its `ports/` folder is `roms/ports`,
which is `/storage/roms/ports` on the device.

ROCKNIX keeps PortMaster under `/storage` and may have no `/roms` path. The launcher
checks the `/storage` locations and a `PortMaster` folder alongside itself.

No extra setup needed.
