#!/usr/bin/env python3
"""pagemap.py -- bytes used and free in each 256-byte page of an AS listing.

    pagemap.py LISTING [FIRST LAST]     (pages as hex: 4 7 for the fixed half)
"""
import re
import sys

LINE = re.compile(r"^(?:\(\d+\))?\s*\d+/\s*([0-9A-F]{3,4}) : ((?:[0-9A-F]{2} )+)")


def main(path, first="4", last="F"):
    used = {}
    for line in open(path, errors="ignore"):
        m = LINE.match(line)
        if not m:
            continue
        a = int(m.group(1), 16)
        for i in range(len(m.group(2).split())):
            used.setdefault((a + i) >> 8, set()).add((a + i) & 0xFF)
    for pg in range(int(first, 16), int(last, 16) + 1):
        s = used.get(pg, set())
        top = max(s) + 1 if s else 0
        print("$%X00: %3d used, top $%X%02X, %3d free above" % (pg, len(s), pg, top & 0xFF if top < 256 else 0xFF, 256 - top))


if __name__ == "__main__":
    main(*sys.argv[1:])
