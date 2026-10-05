#!/usr/bin/env python3
"""mkfont.py -- turn font.txt (and logo.txt) into the CHR-ROM.

Emits a ca65 source with a CHARS segment exporting NESfont, which stands in
for cc65's own neschar.s (crt0 .forceimports the symbol; an object file wins
over the library).

The background pattern table is 256 tiles, 4K: $00-$7F from font.txt, $80-$FF
their reverse-video copies (plane 0 inverted), which is what cc65's revers()
picks. Only text is ever reversed, so the copies of $00-$1F are never shown:
font.txt may define $80-$9F itself, and those replace the copies.

With --sprites, the sprite pattern table follows at $1000. cc65's crt0 runs
the PPU in 8x16 sprite mode, where an odd tile number picks the $1000 table,
so logo.txt (any size, '.' clear and 1-3 a palette colour) is cut into 8x16
cells; the empty ones are dropped and repeats share a pair. Pair k lands at
tiles 2k/2k+1 and is drawn with tile number 2k+1. An RODATA table says where
each cell goes: _logo_spr, {dy, dx, tile} per sprite, and _logo_count.

    mkfont.py font.txt build/font.s [--sprites logo.txt] [--png sheet.png]
"""

import sys

TILES = 128
EXPLICIT_HI = range(0x80, 0xA0)     # reverse copies of $00-$1F: never shown


def glyph_row(line, colour):
    """A row of '#' (the block's plane colour), '1'-'3' or '.' -> 8 colours."""
    return [colour if c == "#" else int(c) if c in "123" else 0
            for c in line.ljust(8, ".")]


def parse(path):
    tiles = {}
    cur = None
    colour = 1
    for lineno, raw in enumerate(open(path, encoding="ascii"), 1):
        line = raw.rstrip("\n")
        if line == "" or line == "#" or line.startswith("# "):
            cur = None
            continue
        if line.startswith("= "):
            words = line[2:].split()
            name = line[2:3] if len(line) == 3 else words[0]
            if len(name) == 1:
                code = ord(name)
            elif name.startswith("$"):
                code = int(name[1:], 16)
            else:
                sys.exit(f"{path}:{lineno}: bad tile name {name!r}")
            plane = 0
            if len(words) == 3 and words[1] == "plane":
                plane = int(words[2])
            colour = 1 << plane
            if not (0 <= code < TILES or code in EXPLICIT_HI) \
                    or code in tiles or code in (0x0A, 0x0D):
                # $0A/$0D are conio's LF/CR: cputc would never draw them
                sys.exit(f"{path}:{lineno}: tile ${code:02X} unusable or repeated")
            cur = tiles[code] = []
            continue
        if cur is None or len(line) > 8 or set(line) - set("#.123") \
                or len(cur) == 8:
            sys.exit(f"{path}:{lineno}: bad glyph row {line!r}")
        cur.append(glyph_row(line, colour))
    return tiles


def tile_bytes(rows):
    """8 rows of 8 colours (0-3) -> the 16 bytes of one NES tile."""
    rows = rows + [[0] * 8] * (8 - len(rows))
    p0 = bytes(sum((c & 1) << (7 - x) for x, c in enumerate(r)) for r in rows)
    p1 = bytes(sum((c >> 1) << (7 - x) for x, c in enumerate(r)) for r in rows)
    return p0 + p1


def chr_bytes(tiles):
    out = bytearray()
    for code in range(TILES):
        out += tile_bytes(tiles.get(code, []))
    for code in range(TILES):
        if code + TILES in tiles:
            out += tile_bytes(tiles[code + TILES])
            continue
        base = code * 16
        out += bytes(b ^ 0xFF for b in out[base:base + 8]) + out[base + 8:base + 16]
    return out


def parse_art(path):
    rows = []
    for lineno, raw in enumerate(open(path, encoding="ascii"), 1):
        line = raw.rstrip("\n")
        if line == "" or line == "#" or line.startswith("# "):
            continue
        if set(line) - set(".123"):
            sys.exit(f"{path}:{lineno}: bad art row {line!r}")
        rows.append([int(c) if c != "." else 0 for c in line])
    width = -(-max(len(r) for r in rows) // 8) * 8
    rows = [r + [0] * (width - len(r)) for r in rows]
    rows += [[0] * width] * (-len(rows) % 16)
    return rows


def sprite_table(art):
    """Cut the art into 8x16 cells -> (4K pattern table, [(dy, dx, tile)])."""
    pairs, where, spr = [], {}, []
    for dy in range(0, len(art), 16):
        for dx in range(0, len(art[0]), 8):
            cell = [art[dy + y][dx:dx + 8] for y in range(16)]
            if not any(any(r) for r in cell):
                continue
            data = tile_bytes(cell[:8]) + tile_bytes(cell[8:])
            if data not in where:
                where[data] = len(pairs)
                pairs.append(data)
            spr.append((dy, dx, where[data] * 2 + 1))
    if len(pairs) > 128:
        sys.exit(f"sprites: {len(pairs)} distinct 8x16 cells, the table holds 128")
    table = b"".join(pairs)
    return table + bytes(0x1000 - len(table)), spr


def write_png(data, path):
    from PIL import Image
    pal = [(0, 0, 0), (255, 255, 255), (60, 190, 255), (0, 0, 170)]
    tables = len(data) // 0x1000
    img = Image.new("RGB", (128 * tables, 128))
    for t in range(256 * tables):
        ox, oy = (t // 256) * 128 + (t % 16) * 8, ((t % 256) // 16) * 8
        for y in range(8):
            lo, hi = data[t * 16 + y], data[t * 16 + 8 + y]
            for x in range(8):
                c = ((lo >> (7 - x)) & 1) | (((hi >> (7 - x)) & 1) << 1)
                img.putpixel((ox + x, oy + y), pal[c])
    img.resize((512 * tables, 512), Image.NEAREST).save(path)


def main():
    args = sys.argv[1:]
    opts = {}
    for opt in ("--png", "--sprites"):
        if opt in args:
            i = args.index(opt)
            opts[opt] = args[i + 1]
            del args[i:i + 2]
    if len(args) != 2:
        sys.exit(__doc__)
    data = chr_bytes(parse(args[0]))
    spr = None
    if "--sprites" in opts:
        table, spr = sprite_table(parse_art(opts["--sprites"]))
        data += table
    with open(args[1], "w", encoding="ascii") as f:
        f.write("; generated by tools/mkfont.py -- do not edit\n\n")
        f.write("        .export NESfont\n")
        if spr is not None:
            f.write("        .export _logo_spr, _logo_count\n")
        f.write('\n.segment "CHARS"\n\nNESfont:\n')
        for i in range(0, len(data), 16):
            f.write("        .byte " + ",".join(f"${b:02X}" for b in data[i:i + 16]) + "\n")
        if spr is not None:
            f.write('\n.segment "RODATA"\n\n_logo_count:\n')
            f.write(f"        .byte {len(spr)}\n_logo_spr:\n")
            for dy, dx, t in spr:
                f.write(f"        .byte {dy},{dx},${t:02X}\n")
    if "--png" in opts:
        write_png(data, opts["--png"])


if __name__ == "__main__":
    main()
