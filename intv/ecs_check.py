#!/usr/bin/env python3
"""Validate ecs.bas's keyboard tables and scan against jzIntv's own emulation.

ecs.bas encodes the ECS matrix as 112 bare numbers. Nothing in a build checks
them: a wrong entry compiles fine and just types the wrong letter. This does
three things, all against jzIntv's source as ground truth rather than against
a second copy of the same assumptions:

  1. every matrix position in ecs_normal vs the KEYB_* bindings in
     jzIntv's src/cfg/mapping.c;
  2. every shifted symbol in ecs_shifted vs mapping.c's "ECS Keyboard
     'Shifted' Keys" block, plus A-Z being the case-flip of a-z;
  3. a port of pad_eval_keyboard() from src/pads/pads.c -- ghost-path
     masking included -- driven by the exact port writes and reads
     ecs_scan performs, checking that all 47 keys decode correctly both
     plain and with SHIFT held.

Check 3 is the one that matters: the diode-less matrix means a normal scan
loses A/D/G/J/L while SHIFT is down, which is the whole reason ecs_scan makes
a second, transposed pass. Run it after touching the tables or the scan.

    ./ecs_check.py [path-to-jzintv-src]      (default ~/Workspace/jzintv-20200712-src)
"""
import os, re, sys

JZ = sys.argv[1] if len(sys.argv) > 1 else os.path.expanduser("~/Workspace/jzintv-20200712-src")
BAS = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ecs.bas")
SENT = {'KB_LEFT': 1, 'KB_ESC': 2, 'KB_ENTER': 3, 'KB_DOWN': 4,
        'KB_UP': 5, 'KB_RIGHT': 6, 'KB_CTRL': 7, 'KB_SHIFT': 8}
REV = {v: k.replace('KB_', '').lower() for k, v in SENT.items()}


def table(bas, label):
    """Pull one DATA table out of ecs.bas as 56 ints, resolving EK_* names."""
    body, vals = bas.split(label + ':\n', 1)[1], []
    for line in body.splitlines():
        line = line.strip()
        if not line.startswith('DATA'):
            if vals:
                break
            continue
        for tok in line[4:].split("'")[0].split(','):
            tok = tok.strip()
            vals.append(SENT[tok] if tok in SENT else int(tok))
    if len(vals) != 56:
        sys.exit(f"{label}: expected 56 entries, parsed {len(vals)}")
    return vals


def show(v):
    return REV.get(v) or (chr(v) if v else '--')


# ---- jzIntv's pad_eval_keyboard, both scan directions --------------------
def _transpose(rows):
    cols = [0] * 8
    for r in range(8):
        for c in range(8):
            if rows[r] >> c & 1:
                cols[c] |= 1 << r
    return cols


def pad_read(k, io0_out, drive):
    """Returns the byte the CP1610 sees: $00FF when driving rows, else $00FE.
    Both active low. The second loop is pads.c's ghost-path masking -- a key
    is erased whenever another pressed key shares its column with an
    undriven row (a buffer fight on the real, diode-less keyboard)."""
    rows = [k[r] & 0xFF for r in range(8)]
    cols = _transpose(rows)
    inner, outer = (rows, cols) if io0_out else (cols, rows)
    merged = 0
    for i in range(8):
        if not (drive >> i & 1):
            merged |= inner[i]
    for i in range(8):
        if outer[i] & drive:
            merged &= ~(1 << i)
    return 0xFF & ~merged


def ecs_scan(k):
    """The exact sequence ecs.bas's ecs_scan performs. Returns (code, shift)."""
    code = shift = 0
    for r in range(7):                                    # pass 1: normal
        bits = pad_read(k, True, 0xFF & ~(1 << r)) ^ 0xFF
        if not bits:
            continue
        if r == 6:
            if bits & 128:
                shift = 1
        else:
            for c in range(7):
                if bits & (1 << c):
                    code = r * 8 + c + 1
    bits = pad_read(k, False, 0x7F) ^ 0xFF                # pass 2: transposed
    if bits:
        if bits & 64:
            shift = 1
        for r in range(1, 6):
            if bits & (1 << r):
                code = r * 8 + 8
    return code, shift


def main():
    mapping = os.path.join(JZ, 'src', 'cfg', 'mapping.c')
    if not os.path.exists(mapping):
        sys.exit(f"jzIntv source not found: {mapping}\nusage: {sys.argv[0]} [path-to-jzintv-src]")
    src, bas = open(mapping).read(), open(BAS).read()
    normal, shifted = table(bas, 'ecs_normal'), table(bas, 'ecs_shifted')
    fails = 0

    # 1. plain matrix
    plain = re.compile(r'\{\s*"KEYB_(\w+)"\s*,\s*W\(pad1\.k\[\s*(\d+)\s*\]\s*\)\s*,\s*\{\s*~\s*(\d+)\s*,')
    alias = {'SEMI': ';', 'PERIOD': '.', 'COMMA': ',', 'SPACE': ' ', 'CTRL': 'ctrl'}
    n = 0
    for nm, row, mask in plain.findall(src):
        mask, row = int(mask), int(row)
        if mask not in (1, 2, 4, 8, 16, 32, 64, 128):
            continue
        want = alias.get(nm, nm.lower())
        got = show(normal[row * 8 + (mask.bit_length() - 1)])
        n += 1
        if got != want:
            print(f"  ecs_normal row {row} bit {mask.bit_length()-1}: "
                  f"jzIntv KEYB_{nm} wants {want!r}, ecs.bas has {got!r}")
            fails += 1
    print(f"ecs_normal:  {n} positions checked against mapping.c")

    # 2. shifted layer
    shift_re = re.compile(r'\{\s*"KEYB_(\w+)"\s*,\s*W\(pad1\.k\[\s*(\d+)\s*\]\s*\)\s*,\s*\{\s*~\(\s*(\d+)\s*<<\s*8\)')
    CH = {'EQUAL': '=', 'QUOTE': '"', 'HASH': '#', 'DOLLAR': '$', 'PLUS': '+',
          'MINUS': '-', 'SLASH': '/', 'STAR': '*', 'LPAREN': '(', 'RPAREN': ')',
          'CARET': '^', 'QUEST': '?', 'PCT': '%', 'SQUOTE': "'", 'COLON': ':',
          'GREATER': '>', 'LESS': '<'}
    n = 0
    for nm, row, mask in shift_re.findall(src):
        if nm not in CH:
            continue
        idx = int(row) * 8 + (int(mask).bit_length() - 1)
        n += 1
        if shifted[idx] != ord(CH[nm]):
            print(f"  ecs_shifted {idx}: jzIntv KEYB_{nm} wants {CH[nm]!r}, "
                  f"ecs.bas has {chr(shifted[idx])!r}")
            fails += 1
    letters = 0
    for i in range(56):
        if 97 <= normal[i] <= 122:
            letters += 1
            if shifted[i] != normal[i] - 32:
                print(f"  ecs_shifted {i}: {chr(normal[i])} should shift to "
                      f"{chr(normal[i]-32)}, not {chr(shifted[i])}")
                fails += 1
    print(f"ecs_shifted: {n} shifted symbols + {letters} letter cases checked")

    # 3. the scan itself, through the emulated matrix
    positions = [(r, c) for r in range(6) for c in range(8) if normal[r * 8 + c]]
    for (r, c) in positions:
        for held in (False, True):
            k = [0] * 8
            k[r] |= 1 << c
            if held:
                k[6] |= 128
            code, got_shift = ecs_scan(k)
            want = (shifted if held else normal)[r * 8 + c]
            got = 0 if code == 0 else (shifted if got_shift else normal)[code - 1]
            if got != want:
                print(f"  scan row {r} bit {c} shift={int(held)}: "
                      f"want {show(want)!r}, decoded {show(got)!r}")
                fails += 1
    print(f"ecs_scan:    {len(positions)} keys x (plain, SHIFT) "
          f"= {len(positions)*2} combinations simulated")

    print("FAIL" if fails else "OK -- all tables and the scan agree with jzIntv")
    return 1 if fails else 0


if __name__ == '__main__':
    sys.exit(main())
