# CONFIG for the Atari 2600

Standalone, like `../intv/`, `../coleco/` and `../channelf/`: the shared `src/`
core assumes a keyboard, four device slots and kilobytes of buffers, and this
console has a joystick, one device slot and 128 bytes of RAM.

Feature parity with `../intv/` — WiFi setup, host slots, browsing, copy, the
Game Lobby and boot — on a machine with a twelve-column display and no text
mode at all.

## Building

    make                # build/config.bin, 16384 bytes
    make rom.h          # bake it into the cartridge firmware
    ./run.sh <harness>  # run it in MAME

Needs [Macroassembler AS](http://john.ccac.rwth-aachen.de:8000/as/) (`asl` and
`p2bin`, on `PATH` or in `~/asl`) — the same assembler the Channel F, Arcadia
and Odyssey² ports use. Not dasm: it is not installed in CI, and `src/vcs.inc`
is written from the hardware register map rather than copied from the dasm
world's `vcs.h`, which is not redistributable.

`$FN2600` (default `~/Workspace/fujinet-firmware/pico/atari-2600`) is the
cartridge's own tree. Two things live there and must not be forked: the mailbox
header this port's equates mirror, and the MAME cartridge device.

## The machine

|  |  |
|---|---|
| Display | **12 columns × 21 rows.** A 3×5 glyph in a 4×6 cell. |
| Console RAM | 128 bytes, of which the stack takes the top. About forty are free. |
| ROM | Seven 2K banks plus a fixed 2K half. Only zero page crosses a bank switch. |
| Input | Joystick, one button, SELECT and RESET. |

**The cartridge composes the glyphs.** There is no framebuffer and no character
generator; the RP2040 renders ASCII into the exact shape a 48-pixel player
kernel wants and publishes six 128-byte planes, and the 6507 does nothing but
stream them into `GRP0`/`GRP1` on a cycle-exact schedule (`src/fujidisp.inc`,
which is transcribed and must not be touched).

**There is no inverse video.** `vcs_render_row()` takes no attribute, so a
selected row cannot be highlighted. Every list marks its selection with a `>`
in column 0 — which is also why content gets eleven columns, not twelve.

**Almost nothing lives in console RAM.** The working directory, the filter, a
pending copy's source path and the text being typed are all 256-byte strings
held in the cartridge. The console holds a cursor and a handful of flags.

## Screens

| Bank | Screen | Keys |
|---|---|---|
| 0 | Host slots | ↑↓ move, FIRE mount and browse, SELECT menu |
| 1 | Browser | ↑↓ move, ←→ page, ← at page 0 goes up, FIRE descend or boot, SELECT menu |
| 2 | Adapter info | FIRE or SELECT back |
| 3 | Keyboard | ↑↓←→ move the grid, FIRE type, SELECT menu |
| 4 | WiFi | ↑↓ move, FIRE pick, SELECT skip |
| 5 | Boot | progress bar; FIRE dismisses a failure |
| 6 | Copy | FIRE returns early |

**SELECT opens an action menu.** The Intellivision reaches info, the lobby,
rename, filter, copy and up-a-level with a twelve-key keypad; this console has
three buttons and twelve columns to name anything in. A chord or a long press
would fit, but neither prints itself, and on a console with no manual an
undiscoverable binding is the same as no binding. The hosts menu carries INFO,
RENAME, WIFI and LOBBY; the browser's carries UP DIR, FILTER, INFO, COPY and
COPY HERE; the keyboard's carries ACCEPT, CANCEL and CLEAR.

## The keyboard

Twelve columns by eight rows is 96 cells and ASCII 32..127 is 96 characters, so
**the cursor position is the character**: `ch = 32 + row*12 + col`, with the
last cell as backspace. No shift key, no paging. The Intellivision uses six
rows of sixteen for the same trick; sixteen columns do not exist here, and the
transpose is what makes it fit.

The cursor is a `^` on the blank line under its row, because there is no
highlight to use. The double spacing that makes room for it also makes 4×6
cells far easier to read.

## What the cartridge had to learn

This port needed six additions to `$FN2600`, all in the mailbox:

- **Four path buffers** (`FN_PATH_SEL0..SEL3`) rather than one. `OPEN_DIRECTORY`
  carries a path *and* a filter in one payload; a pending copy's source has to
  survive the user browsing away to another host; and an edit has to be
  cancellable.
- **`FN_PATH_POPCH`** — drop one character. `FN_PATH_POP` drops a whole
  component, which is right for `..` and useless for typing.
- **`FN_PATH_SEED` / `FN_PATH_COMMIT`** — the edit cycle. Cancel is simply never
  committing, so the original is never written to. The Intellivision needs a
  256-byte scratch copy in console RAM to get the same guarantee.
- **`FN_BLIT_PATH`** — render a buffer into a text row. These pages are
  write-only, so this is the only way to see what has been typed. It is also
  what finally gives the browser a path row.
- **`FN_BLIT_TCELL`** — poke one cell. Moving a list cursor changes two
  characters, and recomposing the rows to do it means having their text — which
  on the WiFi screen arrives one `GET_SCAN_RESULT` at a time. Without this,
  every cursor step costs two round trips.

## Scope

Implemented: WiFi (status, scan, custom SSID, passphrase, connect), eight host
slots (list, mount, rename), browsing (paging, subfolders, filter, `.cfg`
suppression), host-to-host copy including the `.cfg` sibling, the Game Lobby,
adapter info, mount and boot with a progress bar.

Deliberately out of scope, matching `../intv/` and `../channelf/`: the
device-slot screen, mount/eject, read/write mode toggles, new-disk and appkeys.
Device slot 0 is the only one used. None of them are needed to get from
power-on to a booted game, which is this program's whole job.

`CONFIG_BOOT ($D9)` is never sent. CONFIG is the cartridge's own flash boot ROM,
not a served image, so the flag would only unmount device slot 0 — which
`MOUNT_IMAGE` has just used.

## Known limitations

- **The Game Lobby has no content yet.** `ec.tnfs.io` serves `APPLE2/ ATARI/
  C64/ COCO/ INTV/ MSDOS/ UTILS/` and no 2600 directory. The client follows the
  family convention and asks for `/a2600/lobby.bin` (the Intellivision uses
  `/intv/lobby.rom`); it mounts the host and claims a slot correctly, and will
  boot as soon as that directory exists. The path is one literal in
  `src/hosts.asm`.
- **Long names are truncated, not scrolled.** Eleven visible columns against a
  30-character `READ_DIR_ENTRY`. The Intellivision bounce-scrolls the
  highlighted name; this is the one parity item outstanding.

  It needs about 180 bytes and the browser bank has 96. More banks would not
  help — a bank is 2K whatever the image size. The way in is to move the action
  menu into a bank of its own: it is modal already, it costs the hosts, browser
  and keyboard banks about 250 bytes each in `menu.inc` plus their tables, and
  one shared table with a per-caller mask would pay for all three. The scroll
  itself is then cheap, because the mechanism exists: load the full name once
  into a cartridge buffer when the cursor goes idle, then slide a window over
  it with `FN_BLIT_PATH` and re-poke the gutter with `FN_BLIT_TCELL` — no round
  trip per frame, only one when the cursor settles.
- **The WiFi list is capped at twelve** with no paging, as on the Intellivision.

## Tests

`emu/*.lua` drive MAME against a live fujinet-pc. They compare **rendered glyph
forms, not decoded text** — at 3×5, `S` and `5` are one picture, so decoding is
lossy and would fail on real filenames.

    ./run.sh menutest     # the action menu opens, walks, cancels, dispatches
    ./run.sh cfgreset     # the sequence survives the RESET switch
    ./run.sh wifitest     # scan; the cursor moves without a round trip
    ./run.sh edittest     # typing, backspace, and that CANCEL does not write
    ./run.sh accepttest   # ACCEPT really writes a host slot (and restores it)
    ./run.sh pagetest     # .cfg suppression, the path row, the filter
    ./run.sh copytest     # mark, pick a host, copy, verify by re-listing
    BOOT_IMAGE=... CFG_DIR=VCS/ CFG_NAME=DEEP.BIN ./run.sh cfgtest

`accepttest`, `copytest` and `lobbytest` write to a real FujiNet and put back
what they touched. `emu/setslot.lua`, `emu/listhost.lua` and `emu/probe.lua`
are tools rather than tests: they set a host slot, list one, and report where
the keyboard's caret actually is. The last found a cursor that tracked
perfectly and still typed the wrong character -- the action menu and the grid
were sharing a zero-page byte.

The cartridge side has its own gates: `make -C $FN2600/firmware/host_test`,
`tools/checkdefs.py` (the equates must match the firmware header, checked before
anything is assembled) and `tools/checkrom.py` (no reads of the write-only
pages, no read-modify-write on them, no indirect stores).

## Notes for anyone changing this

**A bank that grows past `$17FF` is silently truncated** by `p2bin -r`, and the
client then jumps into whatever the next bank has at that offset. `build.sh`
asks p2bin how many bytes landed in the half that belongs to the mailbox and
fails the build if the answer is not zero. It also prints every bank's headroom
on every build; watch it shrink.

**Equates go in a different file from code.** `src/fujinet.inc` is included
before the `ORG`, `src/fujilib.inc` inside it. Getting that wrong assembles the
whole transport at `$0000` and every `jsr` becomes `20 00 00` — which assembles
cleanly and fails at run time.

**The text planes survive a bank switch**, so a screen that does not paint a row
inherits whatever was there. Every `DRAW` starts with `FNCLS`.

**The sequence number comes from the cartridge**, never a counter in RAM: this
connector has no reset line, so the RESET switch restarts the 6507 and leaves
the cartridge running.

**The path buffers survive a console reset too.** `CFCOLD` empties all four and
selects buffer 0.
