/* fujisplash.h -- the boot splash: the FujiNet wordmark over the Master
 * System BIOS's black, its glyphs coloured top to bottom from white to the
 * BIOS's SEGA blue. The artwork and its data are ../coleco's; see
 * fujisplash_data.c.
 */

#ifndef FUJISPLASH_H
#define FUJISPLASH_H

#define SPLASH_ROW      6       /* top row of the 10-row logo */
#define SPLASH_PATTERN  0x80    /* baked into the name-table data */

extern const unsigned char splash_patterns[768];
extern const unsigned char splash_nametable[320];

/* Take over the screen. The logo's tiles overlap the magenta font, so run
 * disp_init() again before drawing anything else. */
void splash_show(void);

#endif /* FUJISPLASH_H */
