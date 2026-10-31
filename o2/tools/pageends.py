#!/usr/bin/env python3
"""pageends.py -- where each page section of an AS listing ends, from its
`if $ > X00h` fences: how full each 256-byte page is.

    pageends.py LISTING
"""
import re
import sys

FENCE = re.compile(r"^(?:\(\d+\))?\s*\d+/\s*([0-9A-F]{3,4}) :\s*(?:=>\w+\s+)?if \$ > 0?([0-9A-F]{1,2})00h", re.I)
for line in open(sys.argv[1], errors="ignore"):
    m = FENCE.match(line)
    if m:
        at = int(m.group(1), 16)
        lim = int(m.group(2), 16) << 8
        print("$%03X-$%03X: ends $%03X, %+d free" % (lim - 0x100, lim - 1, at, lim - at))
