/**
 * @file vid.h
 * @brief The Model II's 80x24 screen through the P&T video driver.
 */

#ifndef MODEL2_VID_H
#define MODEL2_VID_H

#include <stdint.h>
#include <stdbool.h>

#define VID_COLS 80
#define VID_ROWS 24

/* Model II graphics characters, from the Operation Manual's chart */
#define G_CORNER_UL      0x00 /* ┏ */
#define G_CORNER_UR      0x01 /* ┓ */
#define G_CORNER_LR      0x02 /* ┛ */
#define G_CORNER_LL      0x03 /* ┗ */
#define G_TEE_DOWN       0x04 /* ┳ */
#define G_TEE_LEFT       0x05 /* ┫ */
#define G_TEE_UP         0x06 /* ┻ */
#define G_TEE_RIGHT      0x07 /* ┣ */
#define G_TEE_DOWN_THIN  0x08 /* ┯ heavy across, thin stem */
#define G_TEE_LEFT_THIN  0x09 /* ┨ heavy upright, thin stem */
#define G_TEE_UP_THIN    0x0A /* ┷ */
#define G_TEE_RIGHT_THIN 0x0B /* ┠ */
#define G_CROSS          0x0C /* ╋ */
#define G_CROSS_UPRIGHT  0x0D /* ╂ heavy upright */
#define G_CROSS_ACROSS   0x0E /* ┿ heavy across */
#define G_CROSS_THIN     0x0F /* ┼ */
#define G_BAR_1          0x10 /* rising bars, shortest first */
#define G_BAR_2          0x11
#define G_BAR_3          0x12
#define G_BAR_4          0x13
#define G_VLINE          0x14 /* ┃ */
#define G_VLINE_THIN     0x15 /* │ */
#define G_HLINE          0x16 /* ━ */
#define G_HLINE_THIN     0x17 /* ─ */
#define G_FILL_1         0x18 /* fills from the bottom, lowest first */
#define G_FILL_2         0x19
#define G_FILL_3         0x1A
#define G_FILL_4         0x1B
#define G_EDGE_LEFT      0x1C
#define G_BAR_LEFT       0x1D
#define G_BLOCK          0x1E
#define G_ARROW_UP       0x1F

/** Clear the screen, hide the cursor, and leave graphics and reverse off */
void vid_init(void);

/** Clear the screen, show the cursor, and leave graphics and reverse off */
void vid_exit(void);

void vid_goto(uint8_t x, uint8_t y);

void vid_reverse(bool on);

/** A graphics character at the cursor */
void vid_glyph(uint8_t g);

/** n graphics characters at the cursor */
void vid_glyphs(uint8_t g, uint8_t n);

/** Text at the cursor */
void vid_text(const char *s);

void vid_clear_eol(void);

#endif /* MODEL2_VID_H */
