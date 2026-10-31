#!/usr/bin/env python3
"""checkpage.py -- fail a build when a table is read from the wrong page.

MOVP A,@A reads the 256-byte page the PC is in, so a table has to sit, whole,
in the page of the code that reads it -- and nothing says so when it does not:
the code reads whatever the same offset of its own page holds. Every MOVP table
in this program is addressed as `#label&0FFh`, so each such reference from code
at $800 or above to a label at $800 or above has to be to a label that starts,
and whose bytes end, in the page that reads it: the page the next MOVP or JMPP
runs in (not the reference's: the two can straddle a page boundary), or the
page of the routine called first -- an offset handed to `say` is
read in say's page. (Below $800 the references
are deliberately to other pages' tables -- a writer in page $5 handed T0 by page
$7 -- and fixed.inc asserts its tables stay in their pages.)
"""
import re
import sys

LINE = re.compile(r"^(?:\(\d+\))?\s*\d+/\s*([0-9A-F]{3,4}) : ((?:[0-9A-F]{2} )*)\s*(.*)$")
LABEL = re.compile(r"^([a-z_][a-z_0-9]*):")
REF = re.compile(r"#\(?([a-z_][a-z_0-9]*)\)?&0FFh", re.I)


def check(path):
    labels = {}        # label -> address
    spans = {}         # label -> last byte address of its run
    refs = []          # (address, label, source)
    flow = []          # (address, target label) of each call and jmp
    current = None
    last = None
    for line in open(path, errors="ignore"):
        m = LINE.match(line)
        if not m:
            continue
        addr = int(m.group(1), 16)
        data = m.group(2).split()
        src = m.group(3)
        lm = LABEL.match(src.strip())
        if lm and lm.group(1) not in labels:
            current = lm.group(1)
            labels[current] = addr
        if data and current is not None:
            end = addr + len(data) - 1
            if last is not None and addr != last + 1 and addr != labels[current]:
                pass
            spans[current] = max(spans.get(current, addr), end)
            last = end
        code = src.split(";")[0]
        for r in REF.finditer(code):
            if data:
                refs.append((addr, r.group(1), src.strip()))
        cm = re.match(r"\s*(call|jmp)\s+([a-z_][a-z_0-9]*)\s*$", code)
        if cm and data:
            flow.append((addr, cm.group(2)))
        if data and re.match(r"\s*(movp|jmpp)\b", code):
            flow.append((addr, None))   # read right here
    bad = []
    for addr, lab, src in refs:
        if addr < 0x800 or lab not in labels:
            continue
        start = labels[lab]
        if start < 0x800:
            continue
        end = spans.get(lab, start)
        reader = addr >> 8
        for faddr, ftarget in flow:
            if addr < faddr <= addr + 12:
                if ftarget is None:
                    # the MOVP's own page: an `add` on one page's last
                    # bytes and the MOVP on the next's first reads the next
                    reader = faddr >> 8
                elif labels.get(ftarget, 0) >= 0x800:
                    reader = labels[ftarget] >> 8
                break
        if start >> 8 != reader or end >> 8 != reader:
            bad.append("  $%03X: %s -- '%s' is at $%03X-$%03X, not in page $%X"
                       % (addr, src, lab, start, end, reader))
    if bad:
        print("%s: A TABLE OUTSIDE ITS READER'S PAGE" % path, file=sys.stderr)
        print("\n".join(bad), file=sys.stderr)
        return 1
    return 0


sys.exit(max(check(p) for p in sys.argv[1:]))
