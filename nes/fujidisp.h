/* fujidisp.h -- the small display layer every screen shares.
 *
 * Everything here goes through cc65's NES conio, whose writes are queued
 * into a VRAM buffer the runtime flushes during vblank, so text can be put
 * on screen at any time with rendering on. A shadow copy of the screen in
 * WRAM is what lets a row be re-drawn in reverse video without reading VRAM
 * back: the lists are painted straight out of the cartridge's reply window
 * and keep no copy of their own.
 */

#ifndef FUJIDISP_H
#define FUJIDISP_H

#include <stdbool.h>

#define DISP_COLS 32
#define DISP_ROWS 28

void disp_init(void);
void disp_cls(void);
/* Write `s` at (col,row), clipped to the row. Not NUL-padded: whatever was
 * there stays, so callers that need a clean row call disp_row_clear first. */
void disp_at(unsigned char col, unsigned char row, const char *s);
void disp_row_clear(unsigned char row);
/* One character. Used to paint names straight out of the cartridge's reply
 * window, which is volatile and therefore not a `const char *`. */
void disp_char(unsigned char col, unsigned char row, char c);
void disp_at_hi(unsigned char col, unsigned char row, const char *s, bool hi);
void disp_char_hi(unsigned char col, unsigned char row, char c, bool hi);

/* Turn a whole row reverse video or back again: the selection bar. The row
 * is repainted from the shadow, so it costs no VRAM read-back and no network
 * traffic. Idempotent in both directions. */
void disp_row_invert(unsigned char row, bool on);

/* Right-aligned unsigned decimal, for lengths and error codes. */
void disp_at_u16(unsigned char col, unsigned char row, unsigned int v);
void disp_at_hex8(unsigned char col, unsigned char row, unsigned char v);

#endif /* FUJIDISP_H */
