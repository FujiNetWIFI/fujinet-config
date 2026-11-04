# FujiNet CONFIG for the ColecoVision

CONFIG for the FujiNet ColecoVision cartridge (`fujinet-firmware/pico/coleco`): a 32K Z80
assembly program the cartridge serves at power-on. It is the Master System CONFIG
(`../sms`, branch `sms-phantasy-star`) screen for screen, in its Phantasy Star style: windows
with rounded frames, a blinking pointer, a COMMAND menu, a progress window for mounting and
booting, long names that scroll, and the same sounds.

It runs bare metal: no OS7 calls and no interrupts. The VDP's frame flag is polled, so nothing
ever lands between a VDP address and its data, or inside a mailbox transaction. The console
has 1K of RAM, so host slots, directory pages, scan results and the adapter config are drawn
straight out of the cartridge's 1K reply window at F800H and streamed straight back into the
next payload. `path` and `src_spec` are the only copies in RAM.

## Layout

| file | purpose |
|---|---|
| `config.asm` | header, start, the RAM map, the state dispatcher, the COMMAND menu, shared helpers |
| `wifi.inc` | check / scan / pick / password / custom SSID / join |
| `hosts.inc` | host slots: open, rename, keypad jump, copy destination; the game lobby |
| `browse.inc` | the file browser: paging, descend and climb, filter, the long-name scroll |
| `copy.inc` | host-to-host copy |
| `info.inc` | adapter info (GET_ADAPTERCONFIG_EXTENDED) |
| `boot.inc` | mount into device slot 0 with the progress window, then the ROM swap |
| `edit.inc` | the on-screen keyboard |
| `fujicmd.inc` `mailbox.inc` | Fuji commands on the cartridge mailbox |
| `vdp.inc` `disp.inc` | Graphics I: windows, text, pop-ups, the pointer, colours and fades |
| `input.inc` `sound.inc` `sfx.inc` | the controller; the PSG sequencer and its sounds |
| `splash.inc` `logo.inc` | the splash and the FujiNet wordmark |
| `strings.inc` | every word on screen |
| `font.txt` `tools/mkfont.py` | the tiles, built into Graphics I patterns |
| `tools/checkrom.py` | the image layout check, vendored from `pico/coleco/tools` |
| `emu/drive.lua` | scripted MAME sessions |

## Controls

The left button is the Master System's button 1, the right button is button 2, and `*` is
Pause: it opens the screen's COMMAND menu. The keypad's shortcuts are the menu items' own.

| screen | stick | left | right | `*` menu | keypad |
|---|---|---|---|---|---|
| HOSTS | move | open host | WiFi setup | info, lobby, rename, WiFi | `1`-`8` open that slot, `9` rename, `0` lobby, `#` info |
| HOSTS (copy) | move | pick destination | cancel copy | | `1`-`8` pick that slot |
| FILES | move, left/right page | open folder / boot file | up a folder (at `/`: hosts) | filter, copy, hosts | `1` up, `4` filter, `5` copy, `#` hosts |
| FILES (copy) | as above | open folder | up (at `/`: cancel copy) | copy here, cancel copy | `1` up (at `/`: pick another host), `5` copy here |
| NETWORKS | move, left/right page | pick | hosts | rescan | `#` rescan |
| INFO | | back | back | | |
| keyboard | grid | type | rub out | end | digits type, `#` end; ESC is the cancel |

In a menu the stick moves, left picks, and right or `*` cancels.

## Build

    make            # build/config.bin: 32768 bytes, claim stamped, checkrom-clean
    make install    # into $PICO_COLECO/build, where build-cart.sh bakes it
    make cart       # install, then build the cartridge firmware

Needs zmac 1.3 (`ZMAC=`, else `zmac` on PATH, else `~/Workspace/zmac-1.3/zmac`) and python3.
The build reports the ROM used and the stack's room; the assembler fails if the RAM map grows
into the stack.

Test builds, never to ship: `BARTEST=1` runs the progress bar without a mount, `WIFIFAKE=1`
always goes through the WiFi screens, `LOBBY_HOST=` changes the lobby's host.

## Test in MAME

Needs a MAME with the cartridge device applied (`$PICO_COLECO/emu/apply.sh`) and a
fujinet-pc listening for BoIP.

    make run FUJINET_TCP=127.0.0.1:9995
    make drive FUJINET_TCP=... STEPS='t:SELECT A HOST.;k:fire;t:SELECT A FILE.;d;s:files;q'

`emu/drive.lua` lists the step language: waits, key presses, text checks, screen dumps and
PNG snapshots (in `build/snap`). MAME runs a mount inside a single mailbox read, so the
progress bar goes straight from 0 to 100 there; `BARTEST=1` shows it part-filled.

## Limitations

- The progress window's byte counts ("13K OF 32K") need cartridge firmware that reports them;
  on older carts it shows the percent alone.
- Paths are capped at 95 characters (`THE PATH IS TOO LONG.`).
- Cancelling the filter keeps its edits: the filter is the keyboard's buffer.
- Up a folder lands on page one of the parent.
- Paging one past the end of an exactly full directory shows an empty page; up or left goes
  back.
- The lobby is `/coleco/lobby.rom` on `ec.tnfs.io`, which is not there yet.
- After a game boots, only a power cycle brings CONFIG back.
