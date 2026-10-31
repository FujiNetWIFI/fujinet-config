#!/usr/bin/env python3
"""cfgtest.py -- run a scenario on the rig and check what the screen says.

    cfgtest.py SCENARIO OUTDIR [SECS [IMAGE]]

Two steps of its own, beside o2sim.py's:

    screen NAME             the screen's text now, rebuilt from the CONFIG's
                            own tables in cart RAM (T0, T1, the 8 rows:
                            fixed.inc), printed and kept as NAME
    expect NAME LINE TEXT   that screen's LINE (T0, T1 or 0-7) holds TEXT
    expectnot NAME LINE TEXT
    expectfile PATH[|TEXT]  after the run, the rig's fujinet-pc has PATH
                            (under build/rig/fn), holding TEXT if given

A character is shown as it would be typed: lower case where the 8244 drew
the upper-case glyph in the lower-case colour, '~' for a look-alike (a
character the font lacks). Its symbols' addresses come from the image's
page 0 listing. The scenario runs on test/rig.sh (its environment applies:
HOSTS, FNPC_BIN, O2SIM_FAST ...). Exit status 1 if an expectation fails.
"""
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROM = "0123456789:$ ?LP+WERTUIOQSDFGHJKAZXCVBM.-*%=YN/#"
TXORDER = [0, 2, 4, 6, 1, 3, 5, 7, 8, 10, 12, 14, 9, 11, 13, 15]
LCCOL, SUBCOL = 6, 5            # disp.inc
Y_ROW0, PITCH, NCOLS, ROW_TAB = 30, 24, 12, 25


def symbols(lst):
    out = {}
    for line in open(lst, errors="ignore"):
        for m in re.finditer(r"\*?\s*([A-Za-z_][A-Za-z0-9_]*) :\s+([0-9A-F]+) C", line):
            out[m.group(1)] = int(m.group(2), 16)
    return out


def char(ptr, ctrl, y):
    p9 = ptr | (ctrl & 1) << 8
    g = ((p9 + y // 2) & 0x1FF) // 8
    col = (ctrl >> 1) & 7
    if g == 59:
        return "\\"
    if g >= len(ROM):
        return "#"
    c = ROM[g]
    if c in "*%#":
        c = {"*": "x", "%": "/", "#": "#"}[c] if col != SUBCOL else c
    if col == SUBCOL:
        return "~"
    if col == LCCOL and c.isalpha():
        return c.lower()
    return c


def tline(b):
    y = b[0]
    cols = [" "] * 16
    for k in range(16):
        cols[TXORDER[k]] = char(b[1 + 2 * k], b[2 + 2 * k], y)
    return "".join(cols).rstrip()


def rows(b):
    out = []
    for r in range(8):
        t = b[r * ROW_TAB:(r + 1) * ROW_TAB]
        y = Y_ROW0 + PITCH * r
        out.append("".join(char(t[2 * k], t[2 * k + 1], y) for k in range(NCOLS)).rstrip())
    return out


def main(scen, out, secs="120", image="build/config.bin"):
    root = os.path.dirname(HERE)
    sym = symbols(os.path.join(os.path.dirname(os.path.abspath(image)), "config_p0.lst"))
    steps, expects, names, files = [], [], [], []
    for line in open(scen):
        s = line.strip()
        w = s.split(None, 3)
        if w and w[0] == "screen":
            names.append(w[1])
            steps.append("cart %X 33" % sym["T0"])
            steps.append("cart %X 33" % sym["T1"])
            steps.append("cart %X 200" % sym["ROWS"])
        elif w and w[0] in ("expect", "expectnot"):
            expects.append((w[0] == "expect", w[1], w[2], w[3] if len(w) > 3 else ""))
        elif w and w[0] == "expectfile":
            f = s.split(None, 1)[1]
            files.append(f)
        else:
            steps.append(s)
    os.makedirs(out, exist_ok=True)
    exp = os.path.join(out, "expanded.scen")
    open(exp, "w").write("\n".join(steps) + "\n")
    r = subprocess.run([os.path.join(root, "test", "rig.sh"), exp, out, secs, image],
                       cwd=root, capture_output=True, text=True)
    peeks = [bytes(int(x, 16) for x in l.split(":", 1)[1].split())
             for l in r.stdout.splitlines() if l.startswith("PEEK cart")]
    screens = {}
    for i, name in enumerate(names):
        if 3 * i + 2 >= len(peeks):
            print("cfgtest: screen %s never came (the run ended early?)" % name)
            break
        t0, t1, rw = peeks[3 * i], peeks[3 * i + 1], peeks[3 * i + 2]
        lines = {"T0": tline(t0), "T1": tline(t1)}
        for k, t in enumerate(rows(rw)):
            lines[str(k)] = t
        screens[name] = lines
        print("--- %s" % name)
        for k in ["T0"] + [str(n) for n in range(8)] + ["T1"]:
            print("%2s |%s|" % (k, lines[k]))
    bad = 0
    for want, name, line, text in expects:
        got = screens.get(name, {}).get(line)
        ok = got is not None and ((text in got) == want)
        if not ok:
            bad += 1
            print("FAIL %s %s %s %r: %r" % ("expect" if want else "expectnot", name, line, text, got))
    rig = os.environ.get("RIG", os.path.join(root, "build", "rig"))
    for f in files:
        path, _, text = f.partition("|")
        full = os.path.join(rig, "fn", path.strip())
        ok = os.path.exists(full) and (not text or text.encode() in open(full, "rb").read())
        if not ok:
            bad += 1
            print("FAIL expectfile %r" % f)
    print("cfgtest: %d expectations, %d failed" % (len(expects) + len(files), bad))
    return 1 if bad or r.returncode else 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:5]))
