# FujiNet CONFIG for the Sega Master System

CONFIG for the FujiNet Master System cartridge (`fujinet-firmware/pico/sms`): a 32K z88dk `+sms`
client the cartridge serves at power-on. Ported from `../coleco/`: same state machine, same
"draw straight out of the cartridge's reply window" discipline, with a Mode 4 platform layer.

Built with z88dk's `+sms` target and **fujinet-lib-experimental's `sms` target**, which carries the
mailbox transport and the claim stamp (`makefiles/sms-romstamp.py`, which also re-stamps the SEGA
header's checksum).

## Layout

| file | purpose |
|---|---|
| `config.c` | entry point, state dispatcher, drawing helpers, the Pause COMMAND window |
| `st_wifi.c` | check / scan / pick / password / custom SSID |
| `st_hosts.c` | host slots: open, rename, game lobby, adapter info |
| `st_files.c` | file browser: paging, descend, climb, filter |
| `st_copy.c` | host-to-host copy |
| `st_info.c` | adapter info (GET_ADAPTERCONFIG_EXTENDED) |
| `st_lobby.c` | the FujiNet Game Lobby |
| `st_boot.c` | mount into device slot 0, progress, jump to the cart's loader page |
| `fujiraw.c` | the transactions that stream their payload byte by byte |
| `fujidisp.c` | the windows, text, blinking cursor, pop-ups, sprites and fades, with a RAM shadow of the name table |
| `fujiin.c` | the pad and Pause, polled once a frame (which also ticks the sound and the cursor) |
| `fujisnd.c` `snd_data.c` | the PSG sequencer, and the sounds it plays |
| `fujiedit.c` | the on-screen keyboard |
| `font.txt` `tools/mkfont.py` | the frame, cursor and font tiles as ASCII art, and the script that turns them into `build/font_data.c` |
| `fujisplash.c` `fujisplash_data.c` | the splash: the FujiNet wordmark from `../coleco`, coloured for Mode 4 |
| `constants.h` `state.h` | states, geometry, shared globals |

## Look and sound

CONFIG looks and sounds like *Phantasy Star*'s menus:

- **Windows.** Everything sits in Phantasy Star's windows: a two-pixel white line with rounded
  corners around a black field. Every screen has a main window, with its title set into the top
  border, and below it a message window shaped like the game's dialogue box. Prompts and results
  type themselves into the message window a character a frame; the button legend sits under them.
- **Lists.** Entries go on every other row, 8 to a page. The selection is a pointer notched into
  the window's left edge, blinking 8 frames on and 8 off. There is no inverse bar.
- **Pause menu.** Pause opens a COMMAND window at the top right. It wipes open a row a frame, and
  closing it puts back exactly what was underneath.
- **Keyboard.** Laid out like the game's name-entry screen: characters on every other column and
  row, a cyan underline under the cursor and a white one where the next character goes. CASE
  shows the grid in lowercase.
- **Splash.** The FujiNet wordmark fades up out of black under a short fanfare, with a yellow
  pointer blinking beside PRESS BUTTON 1, then fades away the way the game leaves a scene: red
  first, then green, then blue. Booting a game fades out the same way.
- **Sound.** The sound effects are doubled on two PSG channels a few period units apart, the
  thick, beating tone of the game's effects. The game's own menus are silent; CONFIG adds a soft
  blip for moves, a rising one to confirm and a falling one to go back. There is also a noise tick
  for typing, a low buzz for errors, a rising sparkle when WiFi connects, a chime when a game is
  ready, and the splash fanfare, whose lead decays and then wavers like the game's music.

The font, tiles and sounds are this project's own work. They are drawn and written to match the
style of the original, not copied from its ROM. `font.txt` is plain ASCII art you can edit
directly, and the sounds in `snd_data.c` are written with note macros.

## Controls

| screen | d-pad | 1 | 2 | Pause |
|---|---|---|---|---|
| HOSTS | move | open host | WiFi setup | COMMAND: info, lobby, rename, WiFi |
| FILES | up/down move, left/right page | open dir / boot file | up a directory (at `/`: hosts) | COMMAND: filter, copy, hosts |
| WIFI | up/down move, left/right page | pick network | back to hosts | rescan |
| INFO | | back | back | |
| keyboard | grid | type | rub out | END (cancel is the ESC cell) |

In a COMMAND window, up/down choose, 1 runs it, 2 or Pause cancels. A press made while CONFIG is
typing a message or opening a window still counts.

## Build

    make            # build/config.sms
    make install    # into ~/Workspace/fujinet-firmware/pico/sms/build, where build-cart.sh bakes it
    make cart       # install, then build the cartridge firmware

`Z88DK`, `FNLIB` and `PICO_SMS` override the default paths.
