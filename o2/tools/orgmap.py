#!/usr/bin/env python3
"""orgmap.py -- each `org` section of an AS listing: where it starts, where
its bytes end, and what's free before the next 256-byte boundary.

    orgmap.py LISTING
"""
import re
import sys

LINE = re.compile(r"^(?:\(\d+\))?\s*\d+/\s*([0-9A-F]{3,4}) : ((?:[0-9A-F]{2} )*)\s*(.*)$")
secs = []
cur = None
for line in open(sys.argv[1], errors="ignore"):
    m = LINE.match(line)
    if not m:
        continue
    addr = int(m.group(1), 16)
    data = m.group(2).split()
    src = m.group(3)
    if re.match(r"\s*org\b", src):
        cur = [addr, addr]
        secs.append(cur)
        continue
    if cur is not None and data:
        cur[1] = max(cur[1], addr + len(data))
for a, e in secs:
    nxt = (e + 0xFF) & ~0xFF if e & 0xFF else e
    print("$%03X: ends $%03X (%d bytes), %d free to $%03X" % (a, e, e - a, nxt - e, nxt))
