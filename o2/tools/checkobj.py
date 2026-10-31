#!/usr/bin/env python3
"""checkobj.py -- fail a build when two pieces of code claim the same bytes.

Reads AS's object file rather than the listing: a long `db` string is listed a
few bytes to a line, so a page whose strings run on into the next `org`'s code
can look clean there -- and p2bin only warns "overlapping memory allocation!",
then writes one of them.
"""
import struct
import sys


def segments(path):
    d = open(path, "rb").read()
    if d[:2] != b"\x89\x14":
        sys.exit("checkobj: %s: not an AS object file" % path)
    i, segs = 2, []
    while i < len(d):
        h = d[i]
        i += 1
        if h == 0x00:
            break
        if h == 0x80:
            i += 4
            continue
        if h == 0x81:
            i += 3
        start, ln = struct.unpack("<IH", d[i:i + 6])
        i += 6 + ln
        segs.append((start, ln))
    return sorted(segs)


bad = 0
for path in sys.argv[1:]:
    segs = segments(path)
    for (a, n), (b, m) in zip(segs, segs[1:]):
        if b < a + n:
            print("%s: $%03X-$%03X runs into the code at $%03X" % (path, a, a + n - 1, b),
                  file=sys.stderr)
            bad = 1
sys.exit(bad)
