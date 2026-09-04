# FujiNet CONFIG for the Bally Astrocade

A Z80 program that sets up WiFi, manages the 8 FujiNet host slots, browses
directories on a mounted host, copies files between hosts, and boots a
selected ROM over the cartridge's DBC side-channel — the Astrocade
counterpart of the CONFIG program on every other FujiNet platform, and a
feature port of `fujinet-config/intv`.

It grows the bring-up client at
`fujinet-firmware/pico/astrocade/testrom/fujicfg.asm` (root browse + boot)
to full parity, and is meant to replace it as the RP2040 cartridge's baked-in
boot ROM.

## Layout

| File | Purpose |
|---|---|
| `config.asm` | Entry point, menued-cart header, screen/RAM setup, `ST_CHECK_WIFI`, shared fail/message helpers, `INCLUDE` ordering |
| `fujicmd.inc` | Fuji-device (`0x70`) command wrappers on the mailbox transport (`CGO`, `CODIR`, `CDENT`, `CMOUNT`, …) |
| `input.inc` | Hand-controller + 24-key keypad polling, edge detection, auto-repeat, event codes |
| `font.inc` | Byte-aligned 5×7 blitter with a custom lowercase set (the BIOS font is uppercase-only) |
| `ui.inc` | Rows, chevron cursor, footer, tail-anchored paths, decimal %, `memset` |
| `edit.inc` | On-screen character-grid text editor (SSID / password / rename / filter) |
| `wifi.inc` | `ST_CHECK_WIFI` / `ST_CONNECT_WIFI` / `ST_SET_WIFI` (scan, custom SSID, password, connect) |
| `hosts.inc` | Host slots: list, mount, rename (streamed `WRITE_HOST_SLOTS`), lobby, copy-mode skin |
| `browse.inc` | `ST_SELECT_FILE`: paging, subfolders, `.cfg` suppression, filter, copy hooks |
| `copy.inc` | `COPY_FILE` between hosts, `.cfg` sibling, return-to-source |
| `info.inc` | Adapter info (SSID / IP / firmware version) |
| `boot.inc` | `SET_DEVICE_FULLPATH` + `MOUNT_IMAGE` with a progress bar, then the ROM swap |
| `lib/` | Vendored copies of the bring-up's `fujilib.inc` (mailbox transport), `fujidisp.inc` and `HVGLIB.H` — keep in step with `fujinet-firmware/pico/astrocade/testrom/` |
| `tools/` | Vendored `checkrom.py` (layout enforcement) and `mkromh.py` (firmware packer) |

## Scope

WiFi setup (scan, custom SSID, on-screen password entry, connect); the 8
host slots (list, mount, rename); directory browsing (paging, subfolder
navigation, `.cfg`-sibling suppression, filtering); copying a file between
hosts; an info screen; the FujiNet Game Lobby shortcut; and boot with a
progress bar. Out of scope, as on the Intellivision: the device-slot screen,
mount/eject, read/write toggles, new-disk, and appkeys.

## Controls

Player-1 hand controller plus the 24-key keypad.

| Screen | Stick | Keypad |
|---|---|---|
| everywhere | ↑/↓ move, trigger selects | ↑/↓ keys mirror the stick |
| hosts | | `1`–`8` jump to a slot, `=` rename, `9` info, `0` lobby |
| file browser | ←/→ = previous / next page | `1` up a dir, `2`/`3` pages, `4` filter, `5` mark / copy-here, `CE` back |
| text editor | move the grid cursor, trigger places | digits type directly, `CE` = backspace, `=` = accept; CASE toggles lowercase |

Copying reuses the host-slot screen as its destination picker: after `5`,
that screen re-renders as "COPY TO HOST", and picking a slot drops you into
its root to choose a destination directory.

## How it boots a ROM

`MOUNT_IMAGE` makes the ESP32 pull the file and push it to the cartridge
over the DBC side-channel while the transaction is still outstanding; the
cartridge stages it, and the client polls `FN_R_BOOT_PCT` for the progress
bar, then does the standard `BOOTLOCK` → screen-RAM stub → `JP 0` swap. A
staged image carrying the `"FUJI"` claim keeps the mailbox after the swap;
a plain commercial dump runs as a plain ROM. This is the same path the
bring-up's `fujiboot` verified byte-for-byte — this client adds the browser
that reaches it and the progress bar that shows it.

## Build

```sh
make            # build/config.bin (8192 bytes, "FUJI" claim stamped, checkrom-clean)
make run        # launch in MAME against a live fujinet-pc (see run.sh)
make rom.h      # fujiconfigrom.h, to bake into the cartridge firmware
```

The assembler is any `zmac` 1.3 on `PATH`, else the standalone build in
`~/Workspace/zmac-1.3`, else the bring-up's `tools/zmac`. Override the
bring-up location with `FUJI_PICO=...` if it isn't at
`~/Workspace/fujinet-firmware/pico/astrocade`.

## Test in MAME

`run.sh` builds if needed and launches the FujiNet-patched MAME
(`astrocde` with the `fujinet` cart device applied — see the bring-up's
`emu/apply.sh`) against a real fujinet-pc over BoIP:

```sh
./run.sh                                # 127.0.0.1:9995 by default
FUJINET_TCP=host:port ./run.sh
FUJINET_DEBUG=1 ./run.sh                # per-transaction stderr log
FUJINET_BOOTDUMP=/tmp/boot ./run.sh     # capture the pushed ROM (+ .cfg) for cmp
```

CONFIG registers as the first entry on the console's SELECT GAME menu;
press keypad `1` to launch it.

## Flash to hardware

```sh
make rom.h
cp fujiconfigrom.h /path/to/fujinet-firmware/pico/astrocade/firmware/include/fujiconfigrom.h
cd /path/to/fujinet-firmware/pico/astrocade
PICO_SDK_PATH=/usr/share/pico-sdk ./build-cart.sh    # -> fujicade.uf2
```

## Constraints (why the code looks the way it does)

- **6912-byte client budget** (`0x0000-0x1AFF`); the image is exactly 8192
  bytes with a `0x55` sentinel, the menued-cart header, and the `"FUJI"`
  claim at `0x1CFC`, enforced by `tools/checkrom.py` on every build.
- **Screen RAM is the only RAM.** Variables live above the 80 visible
  lines; persistent cursor/input state sits just below the stack (as the
  bring-up's fujicfg does), the big path/entry buffers lower down.
- **The BIOS font is uppercase-only,** so case-sensitive SSIDs and
  passwords are drawn with `font.inc`'s own lowercase glyphs.
- **Sequence numbers always derive from the cart's ACKSEQ,** never a local
  counter — console RESET restarts the program but not the cartridge.

## Bank switching

Firmware protocol v2 supports banked carts: `fujilib.inc` now carries the
`FNBKSEL`/`FNBKMAX` equates (one read maps a 4K image page into
2000H-2FFFH with the mailbox fully live; the high half never moves). This
client still fits the single 8K window and does not use them -- see
`firmware/include/fuji_mailbox.h` in fujinet-firmware for the scheme.
