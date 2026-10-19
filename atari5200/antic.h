/* antic.h -- CONFIG's text screen on ANTIC mode 2: 40 x 24 cells of 8 x 8
 * pixels from a font in ROM (font.txt, tools/mkfont.py), the screen at
 * $3C00 in the RAM the linker reserves. The calls are the 7800 MARIA
 * engine's, so the screens above port unchanged: "palette" 1 is reverse
 * video, which is the selection bar.
 *
 * Characters go in as the 7800's tile numbers -- ASCII, with the frame,
 * cursor and progress-bar tiles below $20 -- and are stored as ANTIC's
 * internal codes.
 */

#ifndef ANTIC_H
#define ANTIC_H

#define MT_COLS 40
#define MT_ROWS 24

/* Frames since start-up, wrapping at 256: the VBI's RTCLOK. */
#define mt_frames (*(volatile unsigned char *)0x02)

void mt_init(unsigned char fill);
void mt_clear(unsigned char fill);
/* The background's colour and the text's luminance: mode 2 has no more. */
void mt_colours(unsigned char background, unsigned char text);

/* Move the write position. */
void mt_at(unsigned char col, unsigned char row);
/* One tile at the write position, which advances. */
void mt_put(unsigned char t);
/* `n` copies of a tile. */
void mt_fill(unsigned char t, unsigned char n);
/* A NUL-terminated string, at most `max` tiles; returns how many went. */
unsigned char mt_text(const char *s, unsigned char max);
/* The tile at a cell. */
unsigned char mt_get(unsigned char col, unsigned char row);
/* The `n` cells from the write position in reverse video (pal 1) or not;
 * the write position does not move. */
void mt_setpal(unsigned char pal, unsigned char n);

/* Wait for the next frame. */
void mt_sync(void);

#endif /* ANTIC_H */
