/* maria.h -- a text and tile screen for the Atari 7800 on MARIA, shared by
 * CONFIG and the FujiNet games (each copies maria.s, maria.h and
 * mktiles.py into its own tree).
 *
 * 32 x 24 cells of 8 x 8 pixels (256 x 192), centred, in 320B: every pixel
 * is 2 bits, so a tile has three colours and the background, the way NES
 * CHR tiles do. A cell holds a tile number (0-127) and a palette select (0
 * or 1, MARIA palettes 0 and 4: 320B honours only their high bit). Pixel
 * value 0 is BACKGRND in both; values 1-3 are the selected palette's C1-C3.
 *
 * How it is drawn. Each cell row is one 8-line zone whose display list
 * reads the row's 32 map bytes as indirect two-byte characters from the
 * program's tile set. The list has one 5-byte header per run of cells
 * sharing a palette, so writing tiles costs nothing but the store: MARIA
 * reads the map every line. Changing a palette marks the row, and mt_sync()
 * rebuilds that row's list into its spare slot and swaps one DLL byte, so
 * MARIA never reads a half-built list. Map, palettes, lists and the DLL are
 * in the cart's RAM (the VIDEO segment); the tiles are in ROM (TILES).
 * The atari7800-fujinet.cfg linker config places both, page aligned.
 *
 * DMA cost per line: 16 + 10 per run + 9 per cell (3 for the map byte, 6
 * for the two graphics bytes), of the 426 MARIA clocks a line allows: a
 * one-run row takes 314, which leaves the 6502 about a third of each
 * displayed line, about half of a whole frame. Rows are
 * capped at MT_MAXRUNS runs (434 would overrun); cells past the cap take the
 * last run's palette. No display-list interrupts are used: the lib's NMI
 * hook (fuji_a7800_nmi) is free for a game's own.
 *
 * The tile set: mt_tiles, 2K in segment TILES (page aligned), 8 pages of
 * 256 bytes, page 7 the top pixel row, tile t at bytes 2t and 2t+1 of each.
 * mktiles.py makes it from font.txt-style art or NES 2bpp CHR.
 *
 * Rules: never store to $00-$1F (it is INPTCTRL until locked). The engine
 * uses MARIA's own WSYNC ($24) and owns CTRL, CHARBASE, DPPH/DPPL, OFFSET,
 * BACKGRND and the palette registers through mt_palette().
 */

#ifndef MARIA_H
#define MARIA_H

#define MT_COLS     32
#define MT_ROWS     24
#define MT_MAXRUNS  12

/* What the cells hold. The map's bytes are tile * 2 (MARIA's character
 * code); use the calls below rather than storing into them. */
extern unsigned char mt_map[MT_ROWS * MT_COLS];
extern unsigned char mt_attr[MT_ROWS * MT_COLS];

/* 1 on a PAL console, measured by mt_init(). */
extern unsigned char mt_pal;

/* Vertical blanks seen by mt_sync(), wrapping at 256. */
extern unsigned char mt_frames;

/* The program's tile set (mktiles.py output). */
extern const unsigned char mt_tiles[];

/* Measure the TV standard, clear the screen to tile `blank` with palette 0,
 * build the lists and turn the display on at the next vertical blank. */
void __fastcall__ mt_init(unsigned char blank);

/* The display off (MARIA's DMA stops, the 6502 runs flat out) and on. */
void mt_off(void);
void mt_on(void);

/* Wait for the start of the next vertical blank, count it, and rebuild the
 * rows whose palettes changed. Call it once a frame. */
void mt_sync(void);

/* Colours: BACKGRND, and palette `p` (0 or 1) C1-C3. Values are MARIA's
 * hue << 4 | luminance. */
void __fastcall__ mt_background(unsigned char c);
void __fastcall__ mt_palette(unsigned char p, unsigned char c1,
                             unsigned char c2, unsigned char c3);

/* Every cell to tile `blank`, palette 0. */
void __fastcall__ mt_clear(unsigned char blank);

/* The write cursor. Writes advance it along the row and stop at its end. */
void __fastcall__ mt_at(unsigned char col, unsigned char row);
void __fastcall__ mt_put(unsigned char tile);
/* A NUL-terminated string at the cursor: character = tile number. */
void __fastcall__ mt_puts(const char *s);
/* Text: as mt_puts, but at most `max` characters, and any outside 32-126
 * shows as a space. Returns how many cells it wrote. */
unsigned char __fastcall__ mt_text(const char *s, unsigned char max);
/* `n` cells of `tile` at the cursor. */
void __fastcall__ mt_fill(unsigned char tile, unsigned char n);
/* `n` cells from the cursor to palette `p`, without moving the cursor. */
void __fastcall__ mt_setpal(unsigned char p, unsigned char n);

/* The tile at (col,row). */
unsigned char __fastcall__ mt_get(unsigned char col, unsigned char row);

#endif /* MARIA_H */
