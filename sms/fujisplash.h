/* fujisplash.h -- the boot splash, staged like Phantasy Star's title: the
 * FujiNet wordmark (its glyphs coloured top to bottom from white to the
 * BIOS's SEGA blue) fades up out of black under the fanfare, a yellow
 * pointer blinks beside PRESS BUTTON 1, and it all fades away again. The
 * artwork and its data are ../coleco's; see fujisplash_data.c.
 */

#ifndef FUJISPLASH_H
#define FUJISPLASH_H

#define SPLASH_ROW      6       /* top row of the 10-row logo */
#define SPLASH_PATTERN  0x80    /* baked into the name-table data */

extern const unsigned char splash_patterns[768];
extern const unsigned char splash_nametable[320];

/* Take over the screen until button 1 or the fanfare's end; returns with
 * the screen faded to black and the sound off. The logo's name-table cells
 * are not in fujidisp's shadow, so run disp_init() before drawing anything
 * else. */
void splash_show(void);

#endif /* FUJISPLASH_H */
