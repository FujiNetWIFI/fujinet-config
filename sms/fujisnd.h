/* fujisnd.h -- the SN76489 on port $7F: a small sequencer for sound effects
 * and jingles in the manner of Phantasy Star's sound driver.
 *
 * Driven once a frame from fujiin's frame poll, so it runs only while
 * CONFIG is waiting on the user or on in_wait(); a mailbox transaction
 * freezes whatever is sounding. Hence snd_settle() before anything long.
 *
 * A sound is up to four voice streams (T0, T1, T2, noise). Playing one
 * replaces only the voices it has streams for. A "chorus" sound puts T2 on
 * T1's line a few period units flat and a step quieter -- the thick, beating
 * doubled tone of the original's effects.
 *
 * Stream bytes (snd_data.c has macros for them):
 *   00-5F n, len     note n semitones above A2, len frames; on the noise
 *                    voice n is the noise-control value instead
 *   60 len           rest
 *   64|hi lo len     a raw 10-bit tone period
 *   70|e             envelope e: per-frame attenuation steps (snd_envs)
 *   80|v             volume offset added to the envelope
 *   90|on            delayed vibrato on/off (after 10 frames, toward flat)
 *   FF               end; the voice falls silent
 */

#ifndef FUJISND_H
#define FUJISND_H

#include <stdbool.h>

enum {
    SND_MOVE,       /* cursor moved: a soft blip */
    SND_OK,         /* confirm: the blip, rising */
    SND_BACK,       /* back / cancel: the blip, falling */
    SND_TYPE,       /* a key typed on the keyboard: a noise tick */
    SND_ERROR,      /* a low, beating buzz */
    SND_CONNECT,    /* joined the network: a rising sparkle */
    SND_READY,      /* the game is loaded: a chime */
    SND_TITLE       /* the splash fanfare */
};

typedef struct {
    const unsigned char *v[4];  /* T0, T1, T2, noise; NULL = not used */
    unsigned char chorus;       /* nonzero: T2 doubles T1, this much flatter */
    unsigned char vol;          /* starting volume offset */
} snd_def;

extern const snd_def snd_defs[];
extern const unsigned char *const snd_envs[];

void snd_init(void);            /* silence, and forget everything */
void snd_play(unsigned char id);
bool snd_busy(void);
/* Before a long transaction: let a short sound finish (20 frames at most),
 * then silence, so nothing drones while the console is busy. */
void snd_settle(void);
void snd_tick(void);            /* once a frame, from fujiin */

#endif /* FUJISND_H */
