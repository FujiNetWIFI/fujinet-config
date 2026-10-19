# FujiNet CONFIG for the Emerson Arcadia 2001

Signetics 2650 assembly (Macroassembler AS) for the FujiNet Arcadia cart
(`fujinet-firmware/pico/arcadia`). It sets up WiFi, manages the 8 host
slots, browses and filters directories, copies files between hosts, shows
adapter info, launches the Game Lobby, and boots a ROM with a progress bar.
It is a feature port of `fujinet-config/astrocade`, grown from the bring-up's
`testrom/fujicfg.asm`, and replaces that file as the cart's baked-in boot ROM.

## Scope

Included:
- WiFi: scan, custom SSID, password, connect
- Host slots: list, mount, rename
- Game Lobby shortcut
- Browser: paging, subfolders, `.cfg` suppression, filter, block-graphics
  scrollbar
- Copy between hosts, with the `.cfg` sibling
- Info screen
- Boot with a block-graphics progress bar
- Sound cues

Out of scope, as on every console port: the device-slot screen,
mount/eject, read/write toggles, new disk, and appkeys.

The ROM budget ruled out two things the plan had: a marquee for long names
(names show 14 characters, as on the Astrocade, NES and SMS) and a
got/total bytes line for the cart firmware's future protocol v2.

## Controls

Player-1 controller. FIRE is the same wire as keypad `2`, so `2` only ever
selects. Keypad `4`/`6`/`8` act as left/right/down, and lists wrap at the
bottom, since there is no keypad "up".

| Screen | Disc | Keypad |
|---|---|---|
| everywhere | up/down move, FIRE selects | `8` down (wraps), Enter selects |
| hosts | | `1` rename, `3` info, `7` WiFi, `9` lobby, Clear cancels a copy |
| browser | left/right page | `4`/`6` page, `1` up a folder, `5` copy (mark / here), `7` filter, Clear back to hosts |
| WiFi list | | Clear rescans, `0` skips setup |
| editor | moves the grid cursor | FIRE types, Clear deletes, Enter OK, `0` ESC |

Copying reuses the host page as the destination picker. After `5` it
re-skins as "COPY TO HOST"; pick a slot, browse to a folder, then press `5`
(or FIRE on a file) to copy there.

## The display

- **One mode for every screen.** 13-line doublescan multicolor, 16 × 13
  cells. It is the only mode that leaves `$1A00-$1AFF` free as RAM.
  - Page: white on blue. Accent: yellow.
  - The selected row is on black.
  - MAME paints the border in bg B, so the page uses bg B.
- **Block graphics.** A cell of exactly `$C0` turns the rest of its row into
  2637 mosaic cells (3 × 2 blocks; MAME `arcadia_v.cpp`).
  - The progress bar is 15 cells × 3 steps. It is used for boot (real
    percent), WiFi connect (poll count) and, as a busy sweep, the long
    single transactions (scan, copy).
  - The browser scrollbar is the column-15 mosaic. The thumb is
    proportional, at half-cell resolution, from an entry count found once
    per folder by probing SET_DIRECTORY_POSITION.
  - Each WiFi network has a one-cell signal staircase.
  - `bar.inc` keeps the rules: a mosaic run always reaches the end of its
    row, and no data cell is an exact `$C0` or `$40`.
- **Charset.** The charset has 64 glyphs, with no lowercase and few symbols.
  The user-defined bank adds `/ - _ ( ) % :` and a placeholder box.
- **Editor.** A 16 × 6 grid of ASCII `$20-$7F`.
  - Lowercase is drawn in the accent ink.
  - The 21 symbols the charset lacks draw as the box, except under the grid
    cursor. There, UDC `$3F` is reloaded with the real glyph, so you see
    exactly what FIRE will type.

## Layout

| File | Block | Purpose |
|---|---|---|
| `config.asm` | 1 | header, zero-page vectors, RAM ledger, START, includes |
| `macros.inc` | | DWBE, positioned strings, PRINT/SETSTR/DAT/DCOL (+ page-1 forms) |
| `disp.inc` | 1 | palette, UDC loader, clear, glyph writer with the `$C0` guard, printers |
| `input.inc` | 1 | two-axis disc + keypad, edge detect, auto-repeat (fujinet-battleship's) |
| `bar.inc` | 1 | progress bar, sweep, scrollbar, signal cell |
| `txlib.inc` | 1 | TX streaming from RAM strings, OPEN_DIRECTORY, positions, LCOMMIT |
| `ui.inc` | 1 | title, messages, pause, the list cursor |
| `edit.inc` | 1 | the grid editor |
| `hosts.inc` | 1 | host page, rename (WRITE_HOST_SLOTS streamed from the window), lobby |
| `browse.inc` | 1 | stateless page draw, page ring, subfolders, filter, entry-count probe |
| `copy.inc` | 1 | COPY_FILE + `.cfg` sibling |
| `mailbox.inc` | 2 | cart mailbox transport, command wrappers, page-1 pointer words |
| `wifi.inc` | 2 | power-on check, connect, scan/select/password/save, info |
| `boot.inc` | 2 | SET_DEVICE_FULLPATH, hand-committed MOUNT_IMAGE + bar, stub swap |
| `sound.inc` | 2 | 2637 tone/noise cues |
| `tables.inc`, `glyphs.inc`, `strings.inc` | 2 | XLAT, keypad tables, UDC bitmaps, every literal |
| `bartest.inc` | 1 | `BARTEST=1` widget test screens |
| `tools/` | | vendored `mkimage.py`, `checkrom.py`, `mkromh.py`, `checksize.py`; `checkdepth.py` plus a `--flow` mode |
| `emu/` | | `drive.lua` (scripted headless sessions), `shot.lua` (snapshots) |

## Constraints (why the code looks the way it does)

- **ROM.** The client gets 6,912 bytes: CPU `$0000-$0FFF` plus
  `$2000-$2AFF`, with no banking (A14 is not on the connector). It uses
  3,955 + 2,546 = 6,501, leaving 411 spare. `tools/checksize.py` prints
  the per-module ledger on every build.
- **RAM.** There are 292 bytes, ledgered in `config.asm`.
  - Nothing is buffered. Names stream straight from the reply window, which
    holds still until the next commit, into TX or into the path buffers.
  - Path cap is 87, filter cap 15.
- **2650 pages.** Non-branch memory instructions only reach their own 8K
  page.
  - Block 1 (page 0) reaches the mailbox through pointer words in
    `txlib.inc`.
  - Block 2 (page 1) reaches RAM through the words at the end of
    `mailbox.inc`.
  - AS rejects a direct operand on the wrong page.
- **Zero-page vectors.** `$0004-$003F` holds 30 pointer words. `ZBSR *Zx` /
  `ZBRR *Zx` is a 2-byte call/jump from either page, used for the most-called
  routines.
- **Return stack.** It is the 2650's 8-entry on-chip stack.
  - States are entered with BCTA.
  - `checkdepth --flow` follows calls, branches, fall-through and the
    vectors. The worst chain is 5.
- **SEQ.** It always derives from the cart's ACKSEQ, because console RESET
  restarts CONFIG but not the cart.

## How it boots

1. `SET_DEVICE_FULLPATH` sends the folder plus the window name, padded to
   256.
2. `MOUNT_IMAGE` is committed by hand. The ESP32 pushes the image before
   it ACKs, so while ACKSEQ is pending only the status page is read and the
   bar repainted.
3. The verdict is read after the ACK:
   - READY: boot.
   - FAILED: named error.
   - Still IDLE: NOT A CART (the server mounted it as a plain disk).
   - A refused push NAKs; the cart's FAILED then names it (ROM TOO BIG).
4. On READY: the FUJI fanfare, silence, BOOTLOCK, and the 11-byte stub at
   `$18E0`, which reads the swap hotspot and jumps to `$0000`.

Known limits, until the cart firmware syncs `fujimail.c` from SMS:
- The firmware doesn't reset the boot registers when a mount starts, so a
  READY left by a RESET between READY and the swap reads as fresh.
- COPY_FILE gets the cart's 5 s ordinary deadline.

## Build

```sh
make                     # build/config.bin (8192 bytes, FUJI claim, all checks)
WIFIFAKE=1 make          # always run WiFi setup at power-on (fujinet-pc is always connected)
LOBBY_HOST=SD make       # point the lobby at another host (also LOBBY_PATH)
make rom.h               # fujiconfigrom.h for the cart firmware
make install             # into fujinet-firmware/pico/arcadia/build/ for build-cart.sh
```

## Test in MAME

The tests need the FujiNet-patched MAME (`arcadia` with the `fujinet` cart
device; see the bring-up's `emu/apply.sh`) and a fujinet-pc with BoIP.

```sh
make run                                             # interactive
make smoke FUJINET_TCP=127.0.0.1:9995                # headless: hosts + first folder
make drive STEPS='t:HOSTS;k:fire;t:1 UP;k:6;d;q'     # any scripted session
make shot                                            # BARTEST snapshots in build/snap
```

Verified in MAME against a private fujinet-pc:
- the power-on check and WiFi
  - scan, select, password, SET_SSID (97 bytes), connect
  - OTHER with a custom SSID
- host list, rename (written to `fnconfig.ini`), lobby (found, and the
  slot-8 claim)
- browsing: 70-entry paging forward and back, subfolders, up, `.cfg` hidden,
  filter set and cancel, PATH TOO LONG
- copy plus its `.cfg` sibling (byte-identical)
- info
- boots
  - a commercial ROM (mailbox gone) and a FUJI-claimed client (mailbox kept)
  - NOT A CART, ROM TOO BIG, then a good boot right after
- console RESET mid-browse
- sound

Not verifiable in MAME, which pushes synchronously: the bar's intermediate
percentages. BARTEST covers their rendering. Real hardware is untested,
including the mosaic encoding, the analog disc thresholds and UDC writes
during vblank.
