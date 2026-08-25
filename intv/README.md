# FujiNet CONFIG for Intellivision

An IntyBASIC program that sets up WiFi, manages the 8 FujiNet host slots,
browses directories on a mounted host, and boots a selected ROM — the
Intellivision counterpart to the CONFIG program on every other FujiNet
platform. It's meant to replace the placeholder demo currently baked into
`fujinet-firmware/pico/intellivision/firmware/rom.h` as the RP2040
cartridge's boot ROM.

## Layout

| File | Purpose |
|---|---|
| `config.bas` | Entry point and top-level state dispatcher |
| `constants.bas` | Screen/color/input constants, scratch-RAM map, RAM budget notes |
| `fujinet.bas` | Mailbox transport (`fn_transact`, `fn_param`, ...) |
| `fujicmd.bas` | Fuji-device (`0x70`) command wrappers built on `fujinet.bas` |
| `screen.bas` | Low-level text drawing (`scr_puts`, `scr_recolor`, ...) |
| `input.bas` | Edge-detected controller input + the character-grid text editor |
| `scroll.bas` | Bounce-scrolling for filenames too long to fit on screen |
| `st_wifi.bas` | `ST_CHECK_WIFI` / `ST_CONNECT_WIFI` / `ST_SET_WIFI` |
| `st_hosts.bas` | `ST_HOSTS` — the 8 host slots, list/select/edit |
| `st_file.bas` | `ST_SELECT_FILE` — directory browsing, paging, `.cfg` suppression |
| `st_copy.bas` | Filter (keypad 4) and copy-file (keypad 5) for the file browser |
| `st_info.bas` | `ST_INFO` — SSID/IP/firmware version display |
| `st_boot.bas` | `ST_BOOT` — `SET_DEVICE_FULLPATH` + `MOUNT_IMAGE`, progress bar |
| `mkromh.py` | Packs `config.bin` into firmware's `_bootrom[]` array format |

## Scope

Covers WiFi setup (scan, custom SSID, on-screen character-grid password
entry, connect), the 8 host slots (list, mount, rename), directory
browsing (paging, subfolder navigation, `.cfg`-sibling suppression,
bounce-scrolling long filenames, filtering), copying a file between host
slots, and boot. Deliberately out of scope: the device-slot screen,
mount/eject, read/write mode toggles, new-disk, and appkeys — none of them
are needed to get from power-on to a booted game, which is this program's
whole job.

### File browser keys

| Key | Action |
|---|---|
| disc ↑/↓ | move the highlight |
| BTN | enter folder / boot file (or, mid-copy, copy here) |
| `1` | up one directory |
| `2` / `3` | previous / next page |
| `4` | set the filter (a `!name` pattern searches subdirectories on TNFS) |
| `5` | mark the highlighted file for copying; press again in the destination directory to copy |
| CLEAR | back to the host slots (cancels an in-progress copy) |

Copying reuses the host-slot screen as its destination picker: after `5`,
that screen re-renders as "COPY TO HOST", and picking a slot mounts it and
drops you into its root to choose a destination directory.

## Build

```sh
export PATH=/path/to/intybasic:/path/to/as1600:$PATH   # if not already on PATH
make            # config.bin (+ .cfg) and config.rom
```

`config.bin`/`config.cfg` are the PiRTO II / SD-flashable format; `config.rom`
is Intellicart format, for jzIntv.

## Test in jzIntv

`run.sh` builds if needed and launches the FujiNet-patched jzIntv
(`~/Workspace/jzintv-20200712-src`) against a real `fujinet-firmware`
instance over BoIP:

```sh
./run.sh                                    # localhost:9995 by default
FUJINET_TARGET=host:port ./run.sh
./run.sh --fujinet-debug                    # trace mailbox/FujiBus frames
```

Everything up through choosing a file to boot is fully testable this way —
`fujinet.c` (the jzIntv side) proxies the mailbox exactly like the real
RP2040 does. The boot step itself (`MOUNT_IMAGE`) triggers the ESP32
pushing a ROM to `FUJI_DEVICEID_DBC`, which the emulator can't act on the
way real hardware does (it can't reload the running cart), but it can
still capture what gets pushed:

```sh
./run.sh --fujinet-bootdump=/tmp/boot-test
# after choosing a file: /tmp/boot-test.rom (+ .cfg, if the file has one)
diff /tmp/boot-test.rom /path/to/source/game.bin   # should be identical
```

That validates the whole ESP32-side media-type path (extension detection,
`.cfg` sibling lookup, chunking) byte-for-byte without hardware.

## Flash to hardware

```sh
make rom.h
cp config_rom.h /path/to/fujinet-firmware/pico/intellivision/firmware/include/fujiconfigrom.h
cd /path/to/fujinet-firmware/pico/intellivision/firmware
cmake -B build -DPICO_BOARD=fujicard -DCMAKE_BUILD_TYPE=Release -G Ninja
ninja -C build
# flash the resulting Minty_fujicard.uf2 to the RP2040 in BOOTSEL mode
```

This is the **only** boot ROM the Minty-based FujiNet cartridge firmware
runs — `RunFujiConfig()` in `src/fujiboot.c` loads it unconditionally, in
place of Minty's own SD/flash launcher (which this firmware build doesn't
include at all). There's no menu, no chord, no fallback.

## Known limitations

- **WiFi network list is capped at 9 entries** (`WIFI_SCAN_SHOWN` in
  `st_wifi.bas`) — there's no paging for scan results the way there is for
  the file browser. A network past the 9th can still be reached via
  "OTHER (ENTER SSID)".
- **Network-pushed `.cfg` mapping supports `[mapping]` segments only** —
  no `[memattr]` RAM segments, no `p`-prefixed HACK/MACRO lines (see
  `parse_cfg_mapping()` in `inty_cart.c`). The local-SD PiRTO menu's own
  parser (`load_cfg()`) still supports the full format; this is specific
  to the network path.
- **`.rom` (Intellicart) bank-switched images are not supported** — a
  non-bankswitched `.rom` (the common case) decodes and boots correctly;
  one that relies on live bank-switching to reach code outside its listed
  segments will not run correctly. See the "ROM boot receiver" section
  comment in `inty_cart.c`.
- **A failed or interrupted network boot needs a power cycle.** The
  original design staged the whole transfer separately from the live
  `ROM[]` so a failure could leave CONFIG running to report it; a real
  build against the RP2040's actual ~264KB SRAM budget showed that doesn't
  fit (a 64KB scratch buffer alone overflowed `.bss` by 54KB). ROM data is
  instead decoded directly into the live `ROM[]` as it streams in — the
  same tradeoff the existing local-SD boot path already accepts.
