# FujiNet CONFIG for the NES

The NES counterpart of this repository's CONFIG: a 32K NROM client, baked
into the FujiNet NES cartridge's RP2354B firmware as the resident image the
loader copies into the PRG SRAM at power-on. Standalone like `../coleco/` and
`../intv/`, and ported from the ColecoVision one: same state machine, same
"draw straight out of the cartridge's reply window" discipline, with cc65's
NES conio and joystick driver as the platform layer.

Built with cc65's `nes` target and **fujinet-lib-experimental's `nes`
target**, which carries the cartridge-mailbox transport, the FujiNet linker
config (it reserves `$FFF0` for the "FUJI" claim) and the claim stamp. Every
FujiNet transaction goes through the lib's `fuji_*`/`FUJICALL_*` API except
four that stream their payloads byte by byte (`fujiraw.c`): a 256-byte path
built from a RAM prefix plus a filename that only exists in the reply window.

## Layout

| file | purpose |
|---|---|
| `config.c` | entry point, state dispatcher, shared drawing helpers |
| `st_wifi.c` | CHECK_WIFI / CONNECT_WIFI / SET_WIFI (scan, pick, password, custom SSID) |
| `st_hosts.c` | host slots: open, rename, game lobby |
| `st_files.c` | file browser: paging, descend/devance, filter |
| `st_info.c` | adapter info (GET_ADAPTERCONFIG_EXTENDED) |
| `st_boot.c` | mount into device slot 0 (a browsed file or the lobby), progress, jump to the loader ROM |
| `fujiraw.c` | the four streamed transactions |
| `fujidisp.c` `fujiin.c` `fujiedit.c` | the platform layer: conio text with a WRAM shadow for the reverse-video bar, frames, the progress bar and the blinking cursor; joystick with auto-repeat; the on-screen keyboard |
| `disp_pal.s` | one palette entry through cc65's vblank write buffer |
| `sfx.c` | the sounds: key click, BEEP, the accept arpeggio, the typing blip |
| `font.txt` `tools/mkfont.py` | the character set and frame/bar tiles, and the script that makes them the CHR-ROM (`build/font.s`, replacing cc65's `neschar.s`) |
| `constants.h` | states, geometry, command payload shapes |

## Scope

Parity with the ColecoVision CONFIG minus host-to-host copy: WiFi setup,
eight host slots with rename, file browser with paging and filter, mount &
boot, adapter info, and the FujiNet Game Lobby. `DEVICE_SLOT` is hardwired
to 0.

The lobby is the ninth row on HOST SLOTS, `PLAY GAME LOBBY`. It boots
`TNFS://ec.tnfs.io/nes/lobby.nes` through the same LOADING screen as a
browsed file. It uses the host slot that already holds `ec.tnfs.io` (any
case). If no slot does, it writes `ec.tnfs.io` into the first empty slot, or
into slot 8 when all eight are taken.

## Look and sound

CONFIG looks and sounds like the Famicom's Family BASIC cartridge:

- **Screen.** White text on black, using Family BASIC's palette (`0F 30 21 02`).
  - Every menu sits in the GAME BASIC menu's light-blue double frame, with the title set into its top edge.
  - Inside the frame is the same 28-column text area BASIC uses.
- **Words.** Legends read `A--OPEN  B--BACK`. Errors read `?MOUNT ERROR 8A`, the way BASIC prints `?SN ERROR`.
- **Power-on.** A screen in the style of BASIC's boot screen: the name types itself out, then `OK` and a blinking block cursor.
- **Text editor.** A BASIC prompt: `HOST NAME?` with a blinking cursor. The pad's keyboard grid sits in a frame below it.
- **Booting.** A `LOADING` screen with the file's name and a progress bar the cartridge's transfer fills.
- **Sound.**
  - Every keypress clicks with the same short pulse-1 tick as Family BASIC.
  - Errors BEEP with that tone held for ten frames.
  - Successful joins, mounts and boots play a short triangle arpeggio.

The font, tiles and sounds are this project's own work. They are drawn and
written to match the style of the original, not copied from its ROM.
`font.txt` is plain ASCII art, so you can edit it directly.

## Controls

| screen | d-pad | A | B | SELECT | START |
|---|---|---|---|---|---|
| HOSTS | move | open host / boot lobby | WiFi setup | rename slot | adapter info |
| FILES | up/down move (page-crossing), left/right page | open dir / boot file | up a directory (at `/`: hosts) | filter | hosts |
| WIFI | move | pick network (password follows) | back to hosts | | rescan |
| INFO | | back | back | | |
| LOADING | | | | | |
| keyboard | grid | pick | backspace | case | OK (cancel is the ESC cell) |

## The one rule

Only plain or indexed stores to `$5500-$57FF`, never a read-modify-write:
the 6502 writes the old value back first and the cartridge sees it as an
event. In C, never `++` or `|=` a mailbox address; everything here goes
through the lib's `fn_regwr()`/`fn_tx()`. `nes-romstamp.py` and the cart's
`checkrom.py` both scan the built image for the opcodes.

## Build

Needs: cc65 2.19 (`cl65` on PATH), fujinet-lib-experimental on its `add-nes`
branch (its nes lib is built automatically via `make -C $FNLIB nes`), and
the cartridge tree for `checkrom.py`.

```sh
make                # -> build/config.nes, stamped and checked
make install        # copy into $PICO_NES/build/, where build-cart.sh
                    # prefers it over the testrom stand-in
make cart           # install + build the RP2354B firmware (fujines.uf2)
make run            # MAME (-nes_slot fujinet) against fujinet-pc's BoIP
```
