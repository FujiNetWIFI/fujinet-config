#!/usr/bin/env python3
"""checksize.py -- per-module ROM budgets from the AS listing.

The astrocade port's mechanism, retargeted to Macroassembler AS: the
source brackets every module with MB_* fence labels, this parses them
out of the listing's symbol table and prints a size table, so ROM growth
is visible per module long before the hard limits. Two regions:

  block 1: CPU $0000-$0FFF (fenced by MB_END1)
  block 2: CPU $2000-$2AFF (fenced by MB_END2; $2B00+ is the mailbox)

Fails the build if either region overflows; grumbles when block 1 has
less than 200 bytes of headroom.

  checksize.py build/5card.lst
"""

import re
import sys

BLK1_TOP = 0x1000
BLK2_TOP = 0x2B00

SYM_RE = re.compile(r"\*?(MB_\w+)\s*:\s+([0-9A-F]+)\s+C\b")


def main() -> int:
    if len(sys.argv) != 2:
        print(__doc__)
        return 2
    text = open(sys.argv[1]).read()
    syms = {}
    for m in SYM_RE.finditer(text):
        syms[m.group(1)] = int(m.group(2), 16)
    if "MB_END1" not in syms or "MB_END2" not in syms:
        print("checksize: MB_END1/MB_END2 fences not found in listing")
        return 1

    fail = False
    for blk, lo, top, end in (
        (1, 0x0000, BLK1_TOP, "MB_END1"),
        (2, 0x2000, BLK2_TOP, "MB_END2"),
    ):
        mods = sorted(
            (a, n) for n, a in syms.items()
            if lo <= a < lo + 0x1000 and not n.startswith("MB_END")
        )
        endaddr = syms[end]
        print(f"block {blk}:")
        for i, (addr, name) in enumerate(mods):
            nxt = mods[i + 1][0] if i + 1 < len(mods) else endaddr
            print(f"  {name[3:]:<10} {nxt - addr:5d}")
        used = endaddr - lo
        avail = top - lo
        spare = avail - used
        print(f"  total       {used:5d} of {avail} ({spare} spare)")
        if endaddr > top:
            print(f"checksize: block {blk} overflows by {endaddr - top} bytes")
            fail = True
        elif blk == 1 and spare < 200:
            print(f"checksize: block 1 headroom is thin ({spare} bytes)")
    return 1 if fail else 0


if __name__ == "__main__":
    sys.exit(main())
