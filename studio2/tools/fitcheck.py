#!/usr/bin/env python3
"""fitcheck.py -- every RFIT's size must cover the routine after it.

    tools/fitcheck.py build/config.lst

RFIT n moves a routine to the next page (and out of $x000-$x3FF) unless n
bytes still fit. AS catches an undersized n only when a short branch then
lands off its page; this catches it while the routine happens to fit, and a
routine that cannot fit a page at all. A routine runs from its RFIT to the
next one, or to CFG_END.
"""
import re
import sys

LINE = re.compile(r"^(?:\(\d+\))?\s*\d+/\s*([0-9A-F]+) :(.*)$")
RFIT = re.compile(r"\(MACRO\)\[\d+\]\s+RFIT\s+(\d+)")
CODE = re.compile(r"\s*((?:[0-9A-F]{2} )+)")
LABEL = re.compile(r"\s([A-Za-z_][A-Za-z0-9_]*):")


def main():
    lines = open(sys.argv[1], errors="replace").read().splitlines()
    fits, code, end = [], [], len(lines)
    for i, raw in enumerate(lines):
        if "CFG_END:" in raw:
            end = i
        m = LINE.match(raw)
        if not m:
            continue
        rest = m.group(2)
        r = RFIT.search(rest)
        if r:
            fits.append((i, int(r.group(1))))
        c = CODE.match(rest)
        if c and not rest.lstrip().startswith("=>"):
            code.append((i, int(m.group(1), 16), len(c.group(1).split())))
    bad = []
    for k, (i, size) in enumerate(fits):
        stop = fits[k + 1][0] if k + 1 < len(fits) else end
        body = [c for c in code if i < c[0] < stop]
        if not body:
            continue
        used = max(a + n for _, a, n in body) - body[0][1]
        if used > size or used > 0x100:
            name = next((LABEL.search(l).group(1) for l in lines[i + 1:i + 12] if LABEL.search(l)), "?")
            bad.append("$%04X %s: RFIT %d, but %d bytes" % (body[0][1], name, size, used))
    for b in bad:
        print("fitcheck: %s: %s" % (sys.argv[1], b), file=sys.stderr)
    if not bad:
        print("fitcheck: %s: %d routines, every RFIT covers its routine" % (sys.argv[1], len(fits)))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
