# CONFIG for the Magnavox Odyssey 2 / Philips Videopac

The FujiNet CONFIG for the O2 cartridge (`fujinet-firmware/pico/o2`): WiFi, hosts, a file browser, copying, info, the lobby, and booting a game with a progress bar. 8048 assembly (Macroassembler AS), built as a protocol 2 (APPBANK) image: a fixed 1K plus 12 pages of 2K (25.6K).

## What it does

| Screen | |
|---|---|
| Splash | The mountain is the grid in fill mode, the snow is two sprites, with a jingle. A key skips it. |
| WiFi | The boot checks the cart's link, then WiFi enabled, connected, and an SSID saved. The scan list shows SSIDs with a signal block (green, tan or red), then OTHER to type a name. Next comes the password, SET_SSID, then connecting with dots. R rescans, CLR skips. |
| Hosts | The 8 slots. Fire, ENT or 1-8 opens a host (an empty slot is named first). E renames, L the lobby, I info, W WiFi, ? help. The bar's long name scrolls. |
| Browser | One READ_DIR_ENTRY in block mode brings a page of whole names. A folder sprite sits beside folders, `.cfg` files are hidden, and the bar's long name scrolls. Paging is left/right, or off the top or bottom. The scrollbar is grid column 9. CLR goes up a folder (from the top: the hosts). F filters, C marks a file to copy, then C again in the destination folder. |
| Copy | COPY_FILE `name\|/` into the destination folder, then its `.cfg` (or `.CFG`) if it has one. Then back to the source. |
| Info | Three pages: SSID, host name, IP, FujiNet version / netmask, gateway, DNS, MAC / BSSID, CONFIG version, cart protocol. Long values scroll. |
| Lobby | `LOBBY_HOST` (default `ec.tnfs.io`) is found in a slot, or given the first empty slot (else slot 8). Then `LOBBY_PATH` (default `/o2/lobby.bin`) boots. |
| Boot | SET_DEVICE_FULLPATH, then MOUNT_IMAGE posted and watched each frame. The bar is grid row 4 (9 cells) plus sprites 2 and 3 for the last cell's fill: 144 steps, with the percent. Errors are named (TOO BIG, NOT A GAME, NO ANSWER, POWER OFF+ON, LOAD FAILED). Then a fanfare and a reset by hand (`jmp $000`), and the cart swaps the game in at the BIOS's fetch of `$400`. No RESET press is needed. |
| Help | `?` (or H) on any screen. Typing has two pages; the second is the shift chart. |

### Typing

The O2 has no shift key. The layout is netcat's O2 layout, so there is one O2 typing scheme to learn:

| | |
|---|---|
| plain | a-z, 0-9, space, `? + . - =` |
| `×` then a key | A-Z, `)!@#$%^&*(` on 0-9, `"` `;` `,` `_` `:` on `? + . - =`, `\` on `÷` |
| `÷` then a key | `'` `]` `>` `<` `[` on `? + . - =`, `\|` on `×`, `/` on `÷`, `` ` `` `~` `{` `}` on 1, 2, 9, 0 |
| `× ×` | shift lock (`×` again unlocks) |
| CLR | rub out; `× CLR` deletes at the cursor, `÷ CLR` clears the field |
| ENT, fire | accept; `× ENT` cancels |
| stick | moves the cursor |
| `?` on an empty field | help; closing the help with `?` types one |

The 8244's font has no lower case and lacks `! " # % & ' ( ) * , ; < > @ [ ] ^ _ { | } ~` and the backquote. Lower case is drawn as the upper-case glyph in cyan; a missing character is drawn as a look-alike in violet. The character before the cursor is also shown as it really is, zoomed in sprite 2 at the top right. Row 7 and T1 chart what the keys type under the current shift.

## The display

Each screen has T0 (16 characters, the quads), 8 rows of 12 singles, and T1 (16), re-pointed by line-counter bands (`src/fixed.inc`, 5 Card Stud's engine reshaped). Objects take writes only with the foreground off, so text lines need blanked gaps: 10 lines is the most that fits. A row band has 12-16 cycles of slack (`make shot`, PADN).

- **Selection bar:** the grid in fill mode, so a band never cuts or jitters it. The grid stays on while the foreground is off.
- **Folder:** sprite 1, re-placed for every row.
- **Colours:** per character. Key names are tan via bit 7 (TXCOL2).
- **PAL:** detected at boot by counting the main loop's passes in a frame (`p0.inc` pal_check). The grid band then gets a longer pad, because a PAL line is 2.5 cycles longer (`J_GRIDP`, `GRIDNOPP`).

## Layout

| Page | |
|---|---|
| 0 | boot, PAL check, splash |
| 1 | the library: character generator, lines, rows, screen, sprites, bar. Reached by farcall (`src/local.inc`'s doors) |
| 2 | hosts |
| 3 | the browser's page of entries (block parse, rows, marquee, paging) |
| 4 | boot (mount, progress bar, soft reset) |
| 5 | the editor |
| 6 | WiFi |
| 7 | info, lobby |
| 8 | help |
| 9 | folders (open, count by probing, title, up, in, filter, copy mark) |
| 10 | the editor's library (prompt, status, chart, magnifier glyphs) |
| 11 | copy |

Screens hand over one way (`far_go`), so each starts at stack depth 0. The current folder lives in FujiNet as the host's prefix (SET/GET_HOST_PREFIX), so the console keeps no path. The 1K reply buffer stays put until the next transaction, and serves as RAM: a page of names, the host slots, a name streamed into the next payload.

## Build

`make` (or `./build.sh`) needs Macroassembler AS (`~/asl/asl`, `p2bin`). The build checks:

- checkflags, checkobj, checklayout, checkmb, checkpage;
- checkstack (farcall only into pages 1 and 10);
- that every page has the same fixed half;
- the 15-page cap.

Build options: `WIFIFAKE=1` (the WiFi set-up even when connected: fujinet-pc always is), `LOBBY_HOST=`, `LOBBY_PATH=`.

The firmware's `pico/o2/build-cart.sh` bakes `build/config.bin` into the cart.

## Test

`make test` runs `test/t/*.scen` on a private rig (`test/rig.sh`): a copy of fujinet-pc-rs232 on BoIP 19976, with its SD from `test/mksd.sh`, and MAME's odyssey2 with the FujiNet cart device as the console. `tools/cfgtest.py` rebuilds each screen's text from CONFIG's own tables in cart RAM and checks it, and checks files on the rig.

Other targets:

| Target | |
|---|---|
| `make copytest` | Needs a tnfsd on localhost:16384 and a fujinet-pc with the COPY_FILE fix (`FNPC_BIN`). |
| `make progress` | A paced push (`FUJINET_PACE_MS`), throttled, with snapshots of the bar. |
| `make rows` | Frame stability on NTSC and PAL (`O2SIM_MACHINE=videopac`). |
| `make run` | A window. |

Verified in MAME only, never on hardware.

## Known limits

- **Same-host copies are refused.** A copy source is named by its host's prefix, which browsing the destination on the same host replaces.
- **COPY_FILE needs a firmware fix.** It fails under a non-empty prefix without the fix in `fujiDevice.cpp` (fnfile_open's buffer), which is in the firmware PR.
- **No O2 lobby client exists yet.** The lobby key gives LOAD FAILED until `/o2/lobby.bin` exists.
- **PAL jitter.** On PAL, the editor's row 0 can lose a few pixels of one scanline at its right edge.
- **G7400 and The Voice are not covered.** The G7400 / Videopac+ (`readkey_plus`) is not handled.
