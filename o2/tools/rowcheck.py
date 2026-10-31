#!/usr/bin/env python3
"""rowcheck.py -- count the frames of a burst that are drawn differently
from the burst's usual frame (a late band cuts the tops off a row's
characters, or moves the gap between two rows of cards).

    rowcheck.py DIR NAME

Each frame is reduced, per 8-line band, to how many pixels it has of each
colour but the background's -- the cards' white, the indices' red and grey,
the backs' blue, the text -- and a frame whose bands differ from the most
common frame's is listed. A blinking sprite or name shows up in its own band
and only there, so read the list, don't just count it.
"""
import collections
import glob
import os
import sys

from PIL import Image


def sig(path):
    im = Image.open(path).convert("RGB")
    w, h = im.size
    px = im.load()
    every = collections.Counter(px[x, y] for y in range(h) for x in range(w))
    bg = every.most_common(1)[0][0]
    bands = []
    for y0 in range(0, h, 8):
        c = collections.Counter()
        for y in range(y0, min(h, y0 + 8)):
            for x in range(w):
                p = px[x, y]
                if p != bg:
                    c[p] += 1
        bands.append(tuple(sorted(c.items())))
    return tuple(bands)


def main(d, name):
    files = sorted(glob.glob(os.path.join(d, name + "_*.png")))
    sigs = [sig(f) for f in files]
    common = collections.Counter(sigs).most_common(1)[0][0]
    odd = [(os.path.basename(f), s) for f, s in zip(files, sigs) if s != common]
    print("%d frames, %d differ" % (len(files), len(odd)))
    for f, s in odd:
        diff = []
        for k, (a, b) in enumerate(zip(s, common)):
            if a != b:
                da = sum(n for _, n in a)
                db = sum(n for _, n in b)
                diff.append("y%d:%+d" % (8 * k, da - db))
        print("  %s: %s" % (f, " ".join(diff)))
    return 1 if odd else 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:3]))
