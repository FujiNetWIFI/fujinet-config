/**
 * @file vid.h
 * @brief The Model I's 64x16 screen, drawn by cells and sent by difference.
 *
 * A cell is one byte of video memory: ASCII, upper case only (vid_text
 * converts), or a graphics block 80h-BFh. Screens draw into a buffer
 * (vid_put and friends); vid_flush() writes the cells that differ from what
 * the screen shows. A stock Model I has no inverse video, so the selection
 * bar is a > in its first cell and the cursor is a solid block.
 */

#ifndef TRS80M1_VID_H
#define TRS80M1_VID_H

#include <stdint.h>
#include <stdbool.h>

#define VID_COLS 64
#define VID_ROWS 16

#define VID_NONE 0xFF

#define G_RULE   0x8C /* a thin line across the cell's middle */
#define G_BLOCK  0xBF

/** Clear the screen and start drawing */
void vid_init(void);

/** Blank the buffer and drop the bar and the cursor */
void vid_begin(void);

/** Set one cell */
void vid_put(uint8_t col, uint8_t row, uint8_t code);

/** Set width cells from col to one code */
void vid_fill(uint8_t col, uint8_t row, uint8_t width, uint8_t code);

/** Write text from col, at most width cells, in upper case, padding with
    spaces to width when pad is set. Returns the column after the text. */
uint8_t vid_text(uint8_t col, uint8_t row, const char *text, uint8_t width, bool pad);

/** Mark line row with a > at col_first; row = VID_NONE takes the bar away */
void vid_bar(uint8_t row, uint8_t col_first);

/** Show the block cursor at col,row; col = VID_NONE hides it */
void vid_cursor(uint8_t col, uint8_t row);

/** Write what changed in lines row_first-row_last */
void vid_flush_rows(uint8_t row_first, uint8_t row_last);

/** Write everything that changed */
void vid_flush(void);

#endif /* TRS80M1_VID_H */
