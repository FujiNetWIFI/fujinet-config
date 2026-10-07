# FujiNet CONFIG for the Atari 7800

The 7800 counterpart of this repository's CONFIG: a 32K client that the
FujiNet 7800 cartridge loads into its SRAM at power-on, with the cart's 16K
of RAM at `$4000`. Ported from `../nes/`, with host-to-host copy from the
Master System CONFIG: the same state machine and the same "draw straight out
of the cartridge's reply window" discipline, on a MARIA text and tile engine
written for it (and for the FujiNet games, which copy it).

Built with cc65's `atari7800` target and **fujinet-lib-experimental's
`atari7800` target**: the mailbox transport, a crt0 that never locks
INPTCTRL, the linker config and the romstamp. Every transaction goes through
the lib's `fuji_*`/`FUJICALL_*` API except the few that stream a payload
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
| `st_info.c` | adapter info, the TV standard, INPTCTRL's lock and the High Score Cart switch |
| `st_boot.c` | mount into device slot 0, progress, hand over to the loader; the HSC ROM offer |
| `fujiraw.c` `fujiraw.h` | the streamed transactions |
| `maria.s` `maria.h` | the MARIA text and tile engine (below) |
| `fujidisp.c` `fujidisp.h` | the NES CONFIG's display calls on the engine: frames, the bar, the progress bar, the cursor |
| `fujiin.c` `fujiin.h` | the left joystick and the console switches, with auto-repeat |
| `fujiedit.c` `fujiedit.h` | the on-screen keyboard |
| `sfx.c` `sfx.h` | the sounds, on the cart's POKEY |
| `font.txt` `tools/mktiles.py` | the character set and frame/bar tiles, and the script that makes them MARIA tiles (`build/tiles.s`) |
| `constants.h` `state.h` | states, geometry, command payload shapes, the shared globals |

## The display engine

32 x 24 cells of 8 x 8 pixels (256 x 192), centred, in **320B with
indirect two-byte characters**: every pixel is 2 bits, so a tile has three
colours and the background, which is what the NES ports' 2bpp CHR art needs
(`mktiles.py --chr` converts it directly). A cell holds a tile number
(0-127) and a palette select; 320B honours only a palette's high bit, so
the select is 0 or 1 (MARIA palettes 0 and 4). Each cell row is one 8-line
zone whose display list has a 5-byte header per run of cells sharing a
palette, reading the row's 32 map bytes in place. Writing a tile is one
store; changing a palette marks the row, and `mt_sync()` rebuilds its list
into a spare slot and publishes it with one DLL store. `maria.h` documents
the API for the games.

320B costs more DMA than 320A's one-byte characters but gives a tile three
colours; per displayed line it takes 16 + 10 per run + 9 per cell of MARIA's
426 clocks (314 for a one-run row).

On PAL the engine leaves 25 more blank lines at the top, and `fujidisp`
moves its hues up one, as a PAL console's are.

The map, palette selects, lists and DLL are in the cart's RAM, in
the lib's `VIDEO` segment; the 2K of tiles are ROM, in `TILES`.

## Controls

Button 1 is the left button of a ProLine joystick, or a 2600 joystick's
only button. RESET works as button 2 everywhere, so one button is enough.

| screen | joystick | 1 | 2 / RESET | SELECT | PAUSE |
|---|---|---|---|---|---|
| HOSTS | move | open host / boot the lobby | WiFi setup | rename slot | info |
| HOSTS (copying) | move | pick the destination | cancel the copy | | |
| FILES | up/down move (page-crossing), left/right page | open dir / boot file | up a directory (at `/`: hosts) | filter | copy this file |
| FILES (copying) | as above | open dir | up (at `/`: cancel) | filter | copy here |
| WIFI | move | pick network | back to hosts | | rescan |
| INFO | | back | back | High Score Cart on/off | |
| LOADING the HSC ROM | | use as High Score Cart | back | | |
| keyboard | grid | pick | delete | case | OK (cancel is the ESC cell) |

## Rules for 7800 FujiNet programs

- Never store to `$00-$1F` or its mirrors. Until locked, every TIA write
  replaces INPTCTRL, and a game is handed to the console's own BIOS only
  while it is unlocked. So: no TIA sound, MARIA's WSYNC (`$24`) not the
  TIA's, and the lib's crt0 instead of cc65's. Reading `$08-$0D` is fine.
- Sound goes through the cart's POKEY at `$0450`.
- Only plain stores to the mailbox's write pages `$0D00-$0FFF`, never a
  read-modify-write: the 6502 writes the old value first.

The lib's `atari7800-romstamp.py` and the cart's `checkrom.py --notia`
both check a built image for these.

## Build

Needs cc65 2.19 (`cl65` on PATH), fujinet-lib-experimental with the
`atari7800` target (built on demand) and the cartridge tree for
`checkrom.py` and `mka78.py`.

```sh
make                # build/config.bin (for the cart) and build/config.a78 (for MAME)
make install        # copy both into $PICO_7800/build/
make cart           # install, then $PICO_7800/build-cart.sh if it exists
make run            # MAME against a live fujinet-pc (run.sh config)
```

`pico/atari-7800/emu/cfgtest.lua` in the firmware tree drives this CONFIG
through hosts, a directory and a boot in MAME; see its header.
