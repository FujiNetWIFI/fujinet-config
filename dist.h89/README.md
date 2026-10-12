# CONFIG for the Heathkit H89

CONFIG runs under HDOS 2.0 (`CONFIG.ABS`) or CP/M 2.2 (`CONFIG.COM`, on a 64K
system) and talks to the H89-ESP32 FujiNet card (an 82C55 at 0xD0) through
fujinet-lib's `h89-hdos` / `h89-cpm` targets. The screen is drawn on the H19
with its own escape codes: direct cursor addressing, reverse video, the
graphics characters for the frame, and the 25th line for the status.

## Building

```sh
make FUJINET_LIB=~/Workspace/fujinet-lib h89
```

`FUJINET_LIB` can be left out: the Makefile then fetches the fujinet-lib
branch named in `FUJINET_LIB_H89`. Each build leaves the program and a 100K
H17 disk with it on in `r2r/`:

| | |
|---|---|
| `r2r/h89-hdos/config.abs`, `config-hdos.h8d` | `MOUNT SY1:` then `RUN SY1:CONFIG.ABS` |
| `r2r/h89-cpm/config.com`, `config-cpm.h8d` | `B:CONFIG` |

## Keys

The arrows are the keypad's (CONFIG puts the keypad in shifted mode, so the
unshifted 8, 2, 4 and 6 move). IC/HOME, IL, DC and DL work as Home, End,
Page Up and Page Down. `<` and `>` turn pages too. ESC goes back, and on the
host and drive lists it mounts the drives and returns to the OS; Q (or ^C)
returns to the OS without mounting.

## What it can't do yet

- **Boot from a FujiNet drive.** Neither OS has a FujiNet disk driver yet, so
  Mount & Exit mounts the drives and returns to HDOS or CP/M, listing what it
  mounted.
- **Create a disk image.** The firmware refuses NEW DISK for now (the RS232
  build's `MediaTypeImg::create`); CONFIG says so, and the empty file the
  firmware leaves behind still lands in the drive.
