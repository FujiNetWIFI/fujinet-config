/* sfx.h -- CONFIG's sounds, on the cart's POKEY at $0450 (never the TIA:
 * its registers are INPTCTRL until locked). Every call is blocking and short.
 */

#ifndef SFX_H
#define SFX_H

void sfx_init(void);

/* The key click: a sub-millisecond tick, on every keypress. */
void sfx_click(void);

/* The BEEP: the click's tone held for ten frames. Errors. */
void sfx_beep(void);

/* A rising C-E-G-C: something worked. */
void sfx_accept(void);

/* The blip of text typing itself out: A5, two frames. */
void sfx_blip(void);

#endif /* SFX_H */
