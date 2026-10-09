/* fujidisp.h -- 32x24 text on the SMS VDP in Mode 4.
 *
 * The font is z88dk's 8x8 standard_font, loaded three times: white at tiles
 * 32-127, magenta at 160-255 and blue at 288-383. A RAM shadow of the screen
 * lets the selection bar re-draw a row without reading VRAM back; highlight
 * is the sprite palette, which is the background palette inverted.
 */

#ifndef FUJIDISP_H
#define FUJIDISP_H

#include <stdbool.h>

#define DISP_COLS 32
#define DISP_ROWS 24

#define DISP_WHITE   0
#define DISP_MAGENTA 1
#define DISP_BLUE    2

void disp_init(void);
void disp_cls(void);
/* Clipped to the row; whatever was there past the string stays. */
void disp_at(unsigned char col, unsigned char row, const char *s);
void disp_at_color(unsigned char col, unsigned char row, const char *s, unsigned char color);
void disp_row_clear(unsigned char row);
void disp_char(unsigned char col, unsigned char row, char c);
void disp_at_hi(unsigned char col, unsigned char row, const char *s, bool hi);
void disp_char_hi(unsigned char col, unsigned char row, char c, bool hi);
/* The selection bar: the whole row inverse, or back again. */
void disp_row_invert(unsigned char row, bool on);
void disp_at_u16(unsigned char col, unsigned char row, unsigned int v);
void disp_at_hex8(unsigned char col, unsigned char row, unsigned char v);

/* Raw VDP access, for the splash. */
void disp_vram_addr(unsigned int a);
void disp_vdp_reg(unsigned char r, unsigned char v);
void disp_cram(unsigned char index, unsigned char value);

#endif /* FUJIDISP_H */
