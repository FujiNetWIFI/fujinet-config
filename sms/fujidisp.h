/* fujidisp.h -- Phantasy Star-style windows on the SMS VDP in Mode 4.
 *
 * Everything on screen is a window: a two-pixel white line with rounded
 * corners around a black field, built from one corner and two edge tiles that
 * the name table flips into place. Text is the font from font.txt, one white
 * copy with tile number = ASCII code. A RAM shadow of the name table lets a
 * row be re-drawn and a pop-up window be closed without reading VRAM back.
 *
 * The selection cursor is a pointer notched into a window's left edge on the
 * selected row; it blinks, driven once a frame from fujiin's frame poll. A
 * selected row's text also carries the sprite-palette bit, whose colours
 * mirror the background's -- invisible, but it is what the MAME harness
 * (fn-sms/pico/sms/emu/smstext.lua) reads as "highlighted".
 *
 * INVARIANT: never poll the frame flag (in_read, in_frames, in_wait) between
 * disp_vram_addr() and the data writes it sets up. Reading the VDP status
 * resets the control port's byte latch, and the cursor blink sets its own
 * VRAM address from inside the poll.
 */

#ifndef FUJIDISP_H
#define FUJIDISP_H

#include <stdbool.h>

#define DISP_COLS 32
#define DISP_ROWS 24

/* Tiles below the font (font.txt). Below 32 they read as blanks to the
 * harness. */
#define T_CORNER  1
#define T_HEDGE   2
#define T_VEDGE   3
#define T_CURSOR  4
#define T_TRI     5             /* splash pointer, yellow */
#define T_ULINE   6             /* keyboard cursor sprite, cyan */
#define T_ULINE2  7             /* keyboard edit-point sprite, white */
#define T_BARCAP  8             /* progress bar end; A_HFLIP for the right */
#define T_BAR0    9             /* progress bar cell, 0-8 px filled: +n */

/* Name-table high-byte bits, as the shadow keeps them. */
#define A_HFLIP   0x02
#define A_VFLIP   0x04
#define A_SEL     0x08          /* sprite palette: the invisible highlight */

/* The two windows every screen is built from: the main window on rows 0-17
 * (its title set into the top border) and the message window on rows 18-23,
 * which is Phantasy Star's dialogue box widened to the full screen. Text
 * lives in columns 1-30 of either. */
#define MAIN_TOP  0
#define MAIN_BOT  17
#define MSG_TOP   18
#define MSG_BOT   23
#define MSG_LINE1 20            /* prompts, results, errors: typed */
#define MSG_LINE2 22            /* the button legend: instant */
#define TEXT_L    1
#define TEXT_R    30

void disp_init(void);           /* tiles, palette, blank screen, no sprites */

/* Text, clipped to columns <= TEXT_R; whatever was there past it stays.
 * Anything outside $20-$7F draws as '?'. */
void disp_at(unsigned char col, unsigned char row, const char *s);
void disp_char(unsigned char col, unsigned char row, char c);
void disp_at_u16(unsigned char col, unsigned char row, unsigned int v);
void disp_at_hex8(unsigned char col, unsigned char row, unsigned char v);
/* Any tile, unclipped: frames, cursors. */
void disp_tile(unsigned char col, unsigned char row, unsigned char t, unsigned char a);
/* Columns TEXT_L..TEXT_R of a row: blank, or re-drawn with attribute bits
 * set and cleared. */
void disp_row_blank(unsigned char row);
void disp_row_attr(unsigned char row, unsigned char set, unsigned char clr);

/* Windows. A wiped window opens top to bottom, a row a frame. */
void win_box(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, bool wipe);
void layout_show(void);         /* the two windows, wiped in unless already up */
void layout_drop(void);         /* something else took the screen */
void win_title(const char *s);  /* into the main window's top border */
void win_clear(void);           /* the main window's interior */

/* The message window's lines: msg_type types a character a frame, the way
 * Phantasy Star's dialogue prints; msg_put is instant. */
void msg_type(unsigned char row, const char *s);
void msg_put(unsigned char row, const char *s);

/* Pop-ups: while one is open, drawing goes to VRAM only, so the shadow still
 * holds what is underneath; closing puts that back, a row a frame. Nothing
 * may draw under an open pop-up. */
void ovl_open(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1);
void ovl_close(void);

/* The blinking cursor: one cell alternating between two tiles, 8 frames
 * each. Showing it somewhere new leaves the old place steadily "on" (a parent
 * menu's pointer stays lit under its pop-up). */
void cur_show(unsigned char col, unsigned char row, unsigned char on_t, unsigned char off_t);
void cur_hide(void);            /* back to its off tile, and stop */
void cur_steady(void);          /* on, until the next frame tick */
void cur_tick(void);            /* once a frame, from fujiin */

/* Sprites, for the keyboard's underlines. `row`/`col` are text cells; the
 * line lands one pixel under the glyph. */
void spr_cell(unsigned char n, unsigned char col, unsigned char row, unsigned char t);
void spr_end(unsigned char n);  /* sprites n.. are off */

/* The palette lives in RAM so fades can scale it. Level 9 is full colour,
 * 0 black; fading out takes red first, then green, then blue, 4 frames a
 * step, the way Phantasy Star leaves a scene. */
void pal_set(unsigned char index, unsigned char value);
void pal_level(unsigned char level);
void fade_in(void);
void fade_out(void);

/* Raw VDP access. */
void disp_vram_addr(unsigned int a);
void disp_vdp_reg(unsigned char r, unsigned char v);

#endif /* FUJIDISP_H */
