# FujiNet CONFIG for the Fairchild Channel F

Standalone, like `../intv/` and `../coleco/`: it mirrors `src/`'s state machine
without sharing its code, because the shared code assumes a keyboard, four
device slots and kilobytes of buffers.

    make            # -> build/config.bin, 16384 bytes

Macro Assembler AS with `CPU F3850`, the same assembler the Arcadia (2650) and
Odyssey² (8048) ports use. The image must be exactly 16K with the `FUJI` claim
at `$47FC`, or the cartridge boots it with the mailbox dead;
`tools/checkrom.py --claim` enforces that.

## Scope

Astrocade parity: WiFi (status, scan, custom SSID, on-screen keyboard,
connect), the eight host slots (list, mount, rename), directory browsing
(paging, subfolder navigation), copying a file between host slots, adapter
info, and boot. Deliberately out of scope, as on every console port: the
device-slot screen, mount/eject, read/write toggles, new-disk and appkeys --
none is needed to get from power-on to a booted cartridge, which is this
program's whole job. `DEVICE_SLOT` is hardwired to 0; the Channel F has exactly
one thing to mount into. `CONFIG_BOOT` (`0xD9`) is never sent: CONFIG lives in
cart flash, and the flag would only unmount device slot 0.

**5,875 bytes** in a 16K window -- within 400 bytes of the Astrocade port's
5,501 for the same feature set.

## Screen

23 columns x 9 rows over the ~95x58 safe area, from a 3x5 font in a 4x6 cell:
a title row, seven list rows, a footer. Column 0 of each list row is a cursor
gutter, so a name gets 22.

The cursor is a chevron redrawn on its own, so moving it never refetches the
row text and costs no network round trip. The ColecoVision inverts the selected
row instead, which needs VRAM readback -- and this console cannot read VRAM at
all.

Lowercase matters in exactly one place: SSIDs and passwords are case-sensitive,
so the keyboard's CASE cell has to be visible in the result. Three pixels wide
leaves no room for real lowercase letterforms, so `a-z` are the uppercase
glyphs squashed into four rows. That is not pretty, but the x-height difference
is unambiguous, which is the only thing the user actually needs.

## Controls

| | |
|---|---|
| stick up/down | move the cursor |
| plunger (push down) or START | select |
| MODE | back, or rename on the host list |
| TIME / HOLD | page up / page down |
| twist counter-clockwise | adapter info |
| twist clockwise | copy this file to another host |

## What the F8 makes you do differently

**One return address.** `PI` copies PC0 into PC1 and `POP` restores it, so a
routine reached by `PI` cannot call anything. `f8call.inc`'s `ENTER`/`LEAVE`
push it onto the BIOS's own K stack at r40-r58 -- nine levels, the same
mechanism the console's `clrscrn` uses. Everything that can avoid nesting does:
`UIROW` inlines its row arithmetic and tail-jumps into `DSTR`.

**`PI` and `JMP` clobber the accumulator.** Both stash the target address's high
byte in A. No argument or result may travel in A across a call -- which also
means `LEAVE` has to carry the return value across its own `PI POPK` by hand,
or every command wrapper hands back the high byte of `$011E`.

**Drawing eats the register file.** `DSTR` and `DCLRR` between them write r0 and
r2-r8. Loop counters live in RAM; three separate loops were written with a
counter in a register and either never terminated or drew exactly one cell.

**RAM is free here, and nowhere else in the family.** The Arcadia client lives
in 84 bytes and the Odyssey² client in 39, because those cartridge edges cannot
accept a write cycle. The Channel F's can, so the cart hands the console 30K --
and the four strings that dominate every other port's RAM ledger are just
buffers.
