# FujiNet CONFIG for the Atari 5200

The 5200 counterpart of this repository's CONFIG: a 32K client that the
FujiNet 5200 cartridge serves at power-on. Ported from `../atari7800/`
(itself the NES CONFIG with the Master System's host-to-host copy): the same
state machine and the same "draw straight out of the cartridge's reply
window" discipline, on an ANTIC text screen in the 8-bit Atari CONFIG's
colours.

Built with cc65's `atari5200` target and **fujinet-lib-experimental's
`atari5200` target**: the mailbox transport, a crt0 that stands in for the
BIOS, the linker config and the romstamp. Every transaction goes through the
lib's `fuji_*`/`FUJICALL_*` API except the few that stream a payload
assembled from pieces (`fujiraw.c`).

## Layout

| file | purpose |
|---|---|
| `config.c` | entry point, state dispatcher, shared drawing helpers |
| `st_wifi.c` | CHECK_WIFI / CONNECT_WIFI / SET_WIFI (scan, pick, password, custom SSID) |
| `st_hosts.c` | host slots: open, rename, the lobby row; the copy destination picker |
| `st_files.c` | file browser: paging, descend/devance, filter |
| `st_copy.c` | host-to-host copy: mark a file, pick a host and directory, copy |
| `st_lobby.c` | PLAY GAME LOBBY: find or add the lobby host, mount, boot |
| `st_info.c` | adapter info |
| `st_boot.c` | mount into device slot 0, progress, hand the console to the image |
| `fujiraw.c` `fujiraw.h` | the streamed transactions |
| `antic.c` `antic.h` `antic_dl.s` | the text screen: ANTIC mode 2, 40 x 24 at `$3C00`, the display list and font in ROM |
| `fujidisp.c` `fujidisp.h` | the NES CONFIG's display calls on it: frames, the bar, the progress bar, the cursor |
| `fujiin.c` `fujiin.h` | the first controller: stick, fire buttons and keypad, with auto-repeat |
| `fujiedit.c` `fujiedit.h` | the on-screen keyboard |
| `sfx.c` `sfx.h` | the sounds, on POKEY channel 1 |
| `font.txt` `tools/mkfont.py` | the character set and frame/bar tiles, and the script that makes them a mode 2 font (`build/font.inc`) |
| `constants.h` `state.h` | states, geometry, command payload shapes, the shared globals |

## The screen

ANTIC mode 2: 40 x 24 cells of 8 x 8 pixels, one colour on the 8-bit
CONFIG's blue. The screen is the top 1K of RAM (`$3C00`, reserved by the
link), the display list and font are ROM. Characters go in as the 7800's
tile numbers (ASCII, with the frame, cursor and progress-bar tiles below
`$20`) and are stored as ANTIC's internal codes; the selection bar is
reverse video.

## Controls

The top fire button and `#` do the same thing, as do the lower button, `*`
and RESET. The keypad's 2, 4, 6 and 8 move as the stick does.

| screen | stick / 2 4 6 8 | fire / `#` | lower / `*` | PAUSE | START |
|---|---|---|---|---|---|
| HOSTS | move | open host / boot the lobby | WiFi setup | rename slot | info |
| HOSTS (copying) | move | pick the destination | cancel the copy | | |
| FILES | up/down move (page-crossing), left/right page | open dir / boot file | up a directory (at `/`: hosts) | filter | copy this file |
| FILES (copying) | as above | open dir | up (at `/`: cancel) | filter | copy here |
| WIFI | move | pick network | back to hosts | | rescan |
| INFO | | back | back | | |
| keyboard | grid | pick | delete | case | OK (cancel is the ESC cell) |

## Rules for 5200 FujiNet programs

- Never store into the cart window `$4000-$BFFF`: the edge has no R/W
  line, so the cart drives the bus on every access there.
- Touch the mailbox's hotspot pages `$B500-$B7FF` only through the lib's
  `fn_tx()` and `fn_regwr()`; the cart counts every read there.
- Never point ANTIC at the arena `$B000-$B7FF`.

The lib's `atari5200-romstamp.py` and the cart's `checkrom.py` both check a
built image for these.

## Build

Needs cc65 2.19 (`cl65` on PATH), fujinet-lib-experimental with the
`atari5200` target (built on demand) and the cartridge tree for
`checkrom.py`.

```sh
make                # build/config.bin, for the cart and MAME's -cart
make install        # copy it into $PICO_5200/build/
make cart           # install, then $PICO_5200/build-cart.sh
make run            # MAME against a live fujinet-pc (run.sh config)
```

`pico/atari-5200/emu/cfgtest.lua` in the firmware tree drives this CONFIG
through hosts, a directory and a boot in MAME; see its header.
