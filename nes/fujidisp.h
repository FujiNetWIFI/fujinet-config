/* fujidisp.h -- the small display layer every screen shares.
 *
 * Everything here goes through cc65's NES conio, whose writes are queued
 * into a VRAM buffer the runtime flushes during vblank, so text can be put
 * on screen at any time with rendering on. A shadow copy of the screen in
 * WRAM is what lets a row be re-drawn in reverse video without reading VRAM
 * back: the lists are painted straight out of the cartridge's reply window
 * and keep no copy of their own.
 *
 * The look is the Famicom Family BASIC's: white 7x7 text on black, and the
 * GAME BASIC menu's light-blue double frame with the title set into its top
 * edge. The font and frame tiles are this project's own (font.txt).
 */

#ifndef FUJIDISP_H
#define FUJIDISP_H

#include <stdbool.h>

#define DISP_COLS 32
#define DISP_ROWS 28

/* The frame every menu screen sits in, and the 28-column text area inside
 * it -- Family BASIC's own width. Rows and clears stay inside IN_L..IN_R so
 * the frame's sides survive them. */
#define FRAME_L   1
#define FRAME_R   30
#define FRAME_T   2
#define FRAME_B   25
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

/* Tiles in $80-$9F, where the never-shown reverse copies of the tiles
 * above would be (font.txt). */
#define T_SB_FULL  0x80         /* the scroll-bar thumb fills the cell */
#define T_SB_START 0x81         /* + n-1: the thumb starts at pixel row n */
#define T_SB_END   0x88         /* + n-1: the thumb fills the first n rows */
#define T_WORD0    0x8F         /* FUJI NET: a top and a bottom tile each */

void disp_init(void);
void disp_cls(void);
/* Write `s` at (col,row), clipped at IN_R. Not NUL-padded: whatever was
 * there stays, so callers that need a clean row call disp_row_clear first. */
void disp_at(unsigned char col, unsigned char row, const char *s);
/* Blank IN_L..IN_R of a row. */
void disp_row_clear(unsigned char row);
/* One character. Used to paint names straight out of the cartridge's reply
 * window, which is volatile and therefore not a `const char *`. */
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

/* Turn IN_L..IN_R of a row reverse video or back again: the selection bar.
 * The row is repainted from the shadow, so it costs no VRAM read-back and no
 * network traffic. Idempotent in both directions. */
void disp_row_invert(unsigned char row, bool on);

/* Copy IN_L..IN_R of a row's shadow (IN_W bytes) out to `buf`, or paint
 * `buf` back over it, reverse video if `hi`. */
void disp_row_save(unsigned char row, char *buf);
void disp_row_restore(unsigned char row, const char *buf, bool hi);

/* A `cells`-wide progress bar at (col,row) showing num/den, to the pixel.
 * Only the cells that changed since the last call are rewritten; call
 * disp_bar_reset() before drawing a new bar. */
void disp_bar_reset(void);
void disp_bar(unsigned char col, unsigned char row, unsigned char cells,
              unsigned char num, unsigned char den);

/* A scroll bar on the frame's rail: `cells` rows down from (col,row), for a
 * list of `count` entries showing `cells` of them from `first`. The thumb is
 * placed to the pixel and at least a cell long; with nothing to scroll, the
 * rail is left plain. Only the cells that changed are rewritten. */
void disp_scroll(unsigned char col, unsigned char row, unsigned char cells,
                 unsigned int first, unsigned int count);

/* The FujiNet logo, in sprites (fujilogo.c): LOGO_HOSTS beside its FUJI NET
 * wordmark on HOST SLOTS, LOGO_MARK faded behind the file browser's list.
 * Every disp_cls() takes it away again. */
enum { LOGO_OFF = 0, LOGO_HOSTS, LOGO_MARK };
void disp_logo(unsigned char mode);
/* Copy the sprite shadow into the PPU at the next quiet vblank (oam.s). */
void oam_sync(void);

/* The blinking block cursor: 12 frames on, 12 off. disp_cursor_at() puts
 * it (lit) at a new place, restoring whatever it covered; disp_cursor_tick()
 * blinks it, once per frame; disp_cursor_off() takes it away. */
void disp_cursor_at(unsigned char col, unsigned char row);
void disp_cursor_tick(void);
void disp_cursor_off(void);

/* Right-aligned unsigned decimal, for lengths and error codes. */
void disp_at_u16(unsigned char col, unsigned char row, unsigned int v);
void disp_at_hex8(unsigned char col, unsigned char row, unsigned char v);

#endif /* FUJIDISP_H */
