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
| `st_hosts.c` | host slots: open, rename |
| `st_files.c` | file browser: paging, descend/devance, filter |
| `st_info.c` | adapter info (GET_ADAPTERCONFIG_EXTENDED) |
| `st_boot.c` | mount into device slot 0, progress, jump to the loader ROM |
| `fujiraw.c` | the four streamed transactions |
| `fujidisp.c` `fujiin.c` `fujiedit.c` | the platform layer: conio text with a WRAM shadow for the reverse-video bar, joystick with auto-repeat, the on-screen keyboard |
| `constants.h` | states, geometry, command payload shapes |

## Scope

Parity with the ColecoVision CONFIG minus host-to-host copy and the lobby:
WiFi setup, eight host slots with rename, file browser with paging and
filter, mount & boot, adapter info. `DEVICE_SLOT` is hardwired to 0.

## Controls

| screen | d-pad | A | B | SELECT | START |
|---|---|---|---|---|---|
| HOSTS | move | open host | WiFi setup | rename slot | adapter info |
| FILES | up/down move (page-crossing), left/right page | open dir / boot file | up a directory (at `/`: hosts) | filter | hosts |
| WIFI | move | pick network (password follows) | back to hosts | | rescan |
| INFO | | back | back | | |
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
