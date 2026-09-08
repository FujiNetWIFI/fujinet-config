# FujiNet CONFIG for the ColecoVision

The ColecoVision counterpart of this repository's CONFIG: a 32K cartridge
client, baked into the FujiNet cartridge's RP2040 firmware as the boot ROM.
Standalone like `../intv/` — it mirrors `src/`'s state machine without sharing
its code, because the shared code assumes a keyboard, four device slots and
kilobytes of buffers, and this machine has a keypad, one cartridge slot, and
**1K of RAM**.

Built in C with z88dk's `+coleco` target, [os7lib](https://github.com/tschak909/os7lib)
(the OS7 BIOS bindings — display, controllers, everything goes through the
console's own BIOS), and **fujinet-lib-experimental's `coleco` target**, which
carries the cartridge-mailbox transport. Every FujiNet transaction goes
through the lib's `fuji_*`/`FUJICALL_*` API except five that must stream their
payloads byte-by-byte (`fujiraw.c`) because a 256-byte staging buffer does not
fit in RAM.

Grown from the bring-up stand-in at
`fujinet-firmware/pico/coleco/testrom/fujicfg.c`; the display/input/editor
layer here (`fuji*.c`) is the canonical copy of that testrom's, extended.

## Layout

| file | purpose |
|---|---|
| `config.c` | entry point, state dispatcher, shared drawing helpers |
| `st_wifi.c` | CHECK_WIFI / CONNECT_WIFI / SET_WIFI (scan, pick, password, custom SSID) |
| `st_hosts.c` | host slots: open, keypad jump, rename; doubles as the copy-destination picker |
| `st_files.c` | file browser: paging, descend/devance, filter |
| `st_copy.c` | host-to-host copy (mark → pick host → copy here) |
| `st_info.c` | adapter info (GET_ADAPTERCONFIG_EXTENDED) |
| `st_boot.c` | mount into device slot 0, progress, ROM swap |
| `st_lobby.c` | keypad 0: mount and boot the Game Lobby from ec.tnfs.io |
| `fujiraw.c` | the five streamed transactions the lib's buffer-shaped calls can't express here |
| `fujidisp.c` `fujiin.c` `fujiedit.c` `fujisnd.c` `fujisplash*.c` | the platform layer: OS7 Graphics-1 text + inverse-charset selection bar, POLLER input with vblank-paced repeat, the on-screen keyboard, PSG click, splash |
| `constants.h` | states, geometry, command payload shapes, **the RAM rules** |
| `bsscheck.py` | build guard: fails the link if statics eat into the stack |

## Scope

INTV parity. In: WiFi setup, eight host slots with rename, file browser with
paging and filter, mount & boot, host-to-host copy, adapter info, lobby boot.
Deliberately out (same as intv): the device-slot screen, mount/eject,
read/write mode toggles, new-disk, appkeys. `DEVICE_SLOT` is hardwired to 0 —
the ColecoVision has exactly one thing to mount into: the cartridge.

## What 1K of RAM means

The cartridge exposes its whole 1K reply as directly addressable ROM at
`$F800`, so host slots, directory pages, scan results and the adapter config
are **drawn straight out of the reply window and streamed straight back** into
the next payload — never copied into RAM. The only reply→RAM copies in the
program are the two bounded strings `path[96]` and `src_spec[96]`. The
browser's filter has no buffer at all: it aliases the on-screen keyboard's
`fn_entry`. Statics end around `$72C0`, leaving ~250 bytes of stack, and
`bsscheck.py` fails the build if that changes for the worse.

## Keys

| screen | joystick / fire | keypad |
|---|---|---|
| HOSTS | move bar / open host | `1-8` jump+open, `9` rename, `0` lobby, `#` info, `*` wifi setup |
| HOSTS (copy) | move / pick destination | `1-8` jump+pick, `*` cancel copy |
| FILES | move (page-crossing), left/right page / open dir or boot file | `1` up-dir (at `/`: back to hosts), `4` filter, `5` mark copy / copy here, `*` hosts (cancels copy) |
| WIFI | move / pick network (password follows) | `#` rescan, `*` back |
| INFO | fire = back | `*` back |
| keyboard | grid; fire picks | `0-9` type digits, `*` backspace, `#` OK; cancel is the ESC **cell** only |

## Build

Needs: z88dk, os7lib (built: `os7.lib`), fujinet-lib-experimental (its coleco
lib is built automatically via `make -C $FNLIB coleco/r2r`).

```sh
make                # -> build/config.bin, stamped and RAM-checked
make install        # copy into $PICO_COLECO/build/, where build-cart.sh
                    # prefers it over the testrom stand-in
make cart           # install + build the RP2040 firmware (fujicoleco.uf2)
```

Env overrides: `Z88DK`, `OS7LIB`, `FNLIB`, `PICO_COLECO` (defaults all under
`~/Workspace`).

## Test in MAME

Needs a MAME with the bring-up's cartridge device grafted in
(`$PICO_COLECO/emu/apply.sh`) and a fujinet-pc BoIP listener on
`127.0.0.1:9995`.

```sh
./run.sh                                   # windowed
SCREEN_AT=8 ./run.sh --headless "SELECT A HOST"
DRIVE_HOST=1 DRIVE_DOWN=9 ./run.sh --drive # browse and boot, headless
DRIVE_SCRIPT="hash,wait" DRIVE_EXPECT=VERSION ./run.sh --drive
```

`DRIVE_SCRIPT` can walk every screen including the on-screen keyboard (the
grid is deterministic). See `$PICO_COLECO/emu/drive.lua` for the vocabulary.

## Flash to hardware

`make cart`, then flash `$PICO_COLECO/firmware/build-fujicoleco/fujicoleco.uf2`
(BOOTSEL). After booting a game the only way back to CONFIG is a power cycle —
RESET does not reach the cartridge; the firmware's power-sense watchdog
reboots it into CONFIG.

## Known limitations

- WiFi scan shows at most 15 networks, no scan paging; row 16 is always
  "OTHER (TYPE AN SSID)".
- Paths are capped at 95 characters (`PATH TOO LONG` past that).
- Cancelling the filter editor keeps its edits — the editor mutates the
  filter in place, and there is no RAM for a scratch copy. `*` (backspace) it
  empty to clear.
- Going up a directory lands on page one of the parent (no page-position
  stack — matches `src/select_file.c`, drops intv's restore nicety).
- Paging one past the end of an exactly-full directory shows an empty page;
  up/left goes back.
- No `.cfg`-sibling copy (that is an Intellivision memory-map concern).
- Copy shows no progress bar: COPY_FILE is one host-side transaction whose
  ACK arrives when the copy is done. The cartridge grants it the 60-second
  mount budget (`fujimail.c`), and the client outwaits that.
- The lobby path is `/coleco/lobby.rom` on `ec.tnfs.io`, by symmetry with
  `/intv/lobby.rom` — **not provisioned on the server yet** (as of 2026-09).
  Until it is, keypad 0 shows `EMOUNT` and returns to the hosts screen.
- A fujinet-pc built from master won't push `.col` images (its
  `discover_mediatype()` gained COL on the `colecovision-bringup` branch);
  `.rom`/`.bin` boot fine either way.

## Provenance

The `fuji*.c/h` platform layer is canonical HERE; the copies in
`fujinet-firmware/pico/coleco/testrom/` are the bring-up's snapshot. Fix bugs
here and sync there. The mailbox transport itself lives in
fujinet-lib-experimental (`bus/coleco/`), mirrored from
`fujinet-firmware/pico/coleco/firmware/include/fuji_mailbox.h` — that header
is the wire-protocol source of truth.
