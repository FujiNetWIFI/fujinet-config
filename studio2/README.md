# FujiNet CONFIG for the RCA Studio II

Standalone, like `../channelf/`: it mirrors `src/`'s screens without sharing its code, because
the shared code assumes a keyboard, four device slots and kilobytes of buffers, and this console
has 512 bytes of RAM.

    make            # -> build/config.st2
    make install    # copies it to fujinet-firmware/pico/studio2/build/, where run.sh serves it

Macroassembler AS with `CPU 1802`, run case-sensitive (`-U`). `build.sh` then runs
`tools/checkrom.py` (no 3-cycle opcodes, RF read only with `LDN`), `tools/fitcheck.py` (every
`RFIT` covers its routine) and `tools/mkst2.py` (AS `.p` to ST2). Output goes only to `build/`.

## Scope

Channel F parity: WiFi status, scan, a network by name and connect, through an on-screen
keyboard; the eight host slots (list, open, rename); directory browsing with paging and
subdirectories; copying a file to another host's root; adapter info; and boot. Left out, as on
every console port: the device-slot screen, mount/eject, read/write toggles, new disk and appkeys.
Device slot 0 is the only one, and `CONFIG_BOOT` is never sent: CONFIG lives in the cart's flash.

**4,152 bytes** of CONFIG on top of the 836-byte runtime: 25 of the 64 pages an image may
claim.

## Screen

The cart's text engine, 16 columns x 10 rows: a title, eight list rows, and a legend of keys
(`5:GO 0:UP 7:COPY`). The selected row is an inverse bar; a name too long for it scrolls there.
Names are printed by the cart straight from its 1K reply window, so a long one never crosses the
CPU, and the cart draws the boot progress bar in its own status strip.

## Controls

Both keypads work alike.

| Key | |
|---|---|
| 2 / 8 | move; past the top or bottom row, the previous or next page |
| 4 / 6 | previous / next page |
| 5 | select: open a host or folder, boot a file, pick a network |
| 0 | back: up a folder, to the host slots from the root, to the status screen from the host slots |
| 7 | rename (host slots), copy this file (browser) |
| 1 | WiFi networks (status screen, host slots) |
| 3 | adapter info (status screen, host slots, browser) |

On the keyboard: 2/4/6/8 move, 5 types the selected key, 0 deletes, 9 is done, 1 cancels. Its
six rows are all of `$20-$7F`, so there is no shift key; the key that 5 would type also shows as
the cursor at the end of the text.

At power-on a FujiNet that is already on a network shows it for three seconds, then goes on to the
host slots by itself.

## Layout

| File | Defines |
|---|---|
| `src/config.asm` | Entry, `main`, the include order |
| `src/cfgdefs.inc` | Commands, the RAM ledger, `LDV`/`STV`/`STATE`/`RFIT` and the other macros |
| `src/ui.inc` | Inline text, legends, `key_wait`, the `kdisp` key table, the error line, list moves, path and TX helpers |
| `src/fujicmd.inc` | One wrapper per FujiNet command |
| `src/wifi.inc` | Status, network list, connect |
| `src/edit.inc` | The on-screen keyboard |
| `src/hosts.inc` | The host slots, also the copy's destination picker |
| `src/browse.inc` | The directory browser |
| `src/copy.inc`, `src/info.inc`, `src/boot.inc` | Copy, adapter info, mount and boot |
| `src/fujinet.inc`, `s2macro.inc`, `s2call.inc`, `fujilib.inc`, `fujidisp.inc`, `input.inc` | The client runtime, vendored from `fujinet-firmware/pico/studio2/testrom/`: keep in step |
| `tools/checkrom.py`, `mkst2.py`, `st2.py` | Vendored from `fujinet-firmware/pico/studio2/tools/` |
| `tools/fitcheck.py` | Checks each `RFIT` size against the listing |

## Notes

- **One round trip per directory page.** `READ_DIR_ENTRY` in block mode (`$C0` + group size)
  returns a page of entries with full names and a directory flag in one 1K reply; moving the bar
  reprints from that reply. The directory stays open until the next `OPEN_DIRECTORY`.
- **RAM.** `$0800-$080F` runtime, `$0810-$082F` state, `$0830` the page's name offsets, `$0840`
  the SSID (33), `$0870` the keyboard's text (65), `$0900` the path (192), `$09C0-$09FF` the stack
  (64; the emulated feature tour peaks at 12). 96 bytes are free.
- **Code** starts at `$0C00`; `RFIT` puts each routine where it does not cross a page (a short
  branch reaches only its own) and hops `$x000-$x3FF`, which the BIOS may mirror.
- **AS:** `!` is XOR and `^` is power.
- Tested in MAME against fujinet-pc: `fujinet-firmware/pico/studio2/emu/cfgtest.lua` boots a game
  from `/studio2/` (`./run.sh config cfgtest`), renames a host slot and back (`CFG_EXTRA=1`), and
  tours info, networks, connect, a refused boot and copy (`CFG_EXTRA=2`). No hardware exists.
