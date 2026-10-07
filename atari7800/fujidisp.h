/* fujidisp.h -- the small display layer every screen shares, on the MARIA
 * engine (maria.h): the NES CONFIG's calls, so its screens port across.
 *
 * Text is white on black, the frames light blue, and the selection bar is
 * palette 1, so reverse video costs no tiles: the font's glyphs are ink
 * (colour 1) on paper (colour 2), and palette 1's paper is the bar's blue.
 * A tile written now shows on the next frame; a palette change shows after
 * the next mt_sync(), which in_read() and wait_frames() run.
 */

#ifndef FUJIDISP_H
#define FUJIDISP_H

#include <stdbool.h>

#include "maria.h"

#define DISP_COLS 32
#define DISP_ROWS 24

/* The frame every menu screen sits in, and the 28-column text area inside
 * it. Rows and clears stay inside IN_L..IN_R so the frame's sides survive. */
#define FRAME_L   1
#define FRAME_R   30
#define FRAME_T   0
#define FRAME_B   23
#define IN_L      2
#define IN_R      29
#define IN_W      (IN_R - IN_L + 1)

/* Tiles below $20 (font.txt). */
#define T_HLINE   0x01
#define T_VLINE   0x02
#define T_TL      0x03
#define T_TR      0x04
#define T_BL      0x05
#define T_BR      0x06
#define T_CURSOR  0x07
#define T_BAR0    0x10          /* T_BAR0 + n: n/8 of the cell filled */

void disp_init(void);
void disp_cls(void);
/* Write `s` at (col,row), clipped at IN_R. Not padded: callers that need a
 * clean row call disp_row_clear first. */
void disp_at(unsigned char col, unsigned char row, const char *s);
/* Blank IN_L..IN_R of a row, and take its bar away. */
void disp_row_clear(unsigned char row);
/* At most `max` characters of a NUL-terminated string in the reply window,
 * clipped at IN_R: names painted straight out of the cartridge. */
void disp_text(unsigned char col, unsigned char row,
               volatile unsigned char *s, unsigned char max);
/* One character. */
void disp_char(unsigned char col, unsigned char row, char c);
void disp_at_hi(unsigned char col, unsigned char row, const char *s, bool hi);
void disp_char_hi(unsigned char col, unsigned char row, char c, bool hi);
/* A raw tile, anywhere on screen: the frame, cursor and bar pieces. */
void disp_tile(unsigned char col, unsigned char row, unsigned char t);

/* A double-line box with `title` (or none) centred in its top edge. */
void disp_box(unsigned char l, unsigned char t, unsigned char r,
              unsigned char b, const char *title);
/* cls + the full-screen menu frame. */
void disp_frame(const char *title);

/* IN_L..IN_R of a row on the selection bar, or off it. */
void disp_row_invert(unsigned char row, bool on);

/* A `cells`-wide progress bar at (col,row) showing num/den, to the pixel.
 * Call disp_bar_reset() before drawing a new bar. */
void disp_bar_reset(void);
void disp_bar(unsigned char col, unsigned char row, unsigned char cells,
              unsigned char num, unsigned char den);

/* The blinking block cursor: disp_cursor_at() puts it (lit) at a new place,
 * restoring what it covered; disp_cursor_tick() blinks it, once a frame;
 * disp_cursor_off() takes it away. */
void disp_cursor_at(unsigned char col, unsigned char row);
void disp_cursor_tick(void);
void disp_cursor_off(void);

/* Unsigned decimal and two hex digits, for counts and error codes. */
void disp_at_u16(unsigned char col, unsigned char row, unsigned int v);
void disp_at_hex8(unsigned char col, unsigned char row, unsigned char v);

#endif /* FUJIDISP_H */
