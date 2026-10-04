/* sfx.h -- the Family BASIC's sounds, made the way it makes them: one APU
 * channel, a few register stores, no driver. Every call is blocking and short.
 */

#ifndef SFX_H
#define SFX_H

void sfx_init(void);

/* The key click: a sub-millisecond tick on pulse 1, on every keypress. */
void sfx_click(void);

/* The BEEP: the click's tone held for ten frames. Errors. */
void sfx_beep(void);

/* A rising C-E-G-C on the triangle: something worked. */
void sfx_accept(void);

/* The blip of text typing itself out: pulse 1, A5, two frames. */
void sfx_blip(void);

#endif /* SFX_H */
