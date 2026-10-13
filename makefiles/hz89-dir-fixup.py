#!/usr/bin/env python3
# Fixes up the hz89 disk image appmake +cpmdisk makes for h89-cpm.mk.
#
# appmake gives a file of exactly 16 1K blocks a second, all-zero directory
# entry, which DIR lists as a nameless file. Mark such entries erased the way
# ERA does, E5 in the first byte only: Heath's CP/M 2.2.04 ends a directory
# search at an entry that is all E5 (never used), so a fully blanked one would
# hide every file after it. The directory is the first 8 logical sectors after
# the 3 boot tracks of the hz89 image (10 x 256-byte sectors a track, skewed).
import sys

SKEW = [0, 4, 8, 2, 6, 1, 5, 9, 3, 7]

with open(sys.argv[1], 'r+b') as f:
    for ls in range(8):
        off = (3 * 10 + SKEW[ls]) * 256
        f.seek(off)
        sec = bytearray(f.read(256))
        for e in range(0, 256, 32):
            if not any(sec[e:e + 32]):
                sec[e] = 0xe5
        f.seek(off)
        f.write(sec)
