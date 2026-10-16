/* fujiin.h -- the first controller: its stick, its two fire buttons and
 * its keypad.
 *
 * Edge-triggered events, one per call, with the NES CONFIG's vocabulary so
 * the screens port across. The top fire button and '#' are IN_FIRE, the
 * lower button, '*' and RESET are IN_BACK, PAUSE is IN_SELECT and START is
 * IN_START. The keypad's 2, 4, 6 and 8 move as the stick does, which is the
 * only steering a worn analog stick may leave.
 */

#ifndef FUJIIN_H
#define FUJIIN_H

#include <stdbool.h>

#define IN_NONE   0
#define IN_UP     1
#define IN_DOWN   2
#define IN_LEFT   3
#define IN_RIGHT  4
#define IN_FIRE   5         /* top fire, '#' */
#define IN_BACK   6         /* lower fire, '*', RESET */
#define IN_SELECT 7         /* PAUSE */
#define IN_START  8         /* START */

void in_init(void);

/* Frames since start-up, wrapping at 256: for the short waits. */
unsigned char in_frames(void);

/* One event, or IN_NONE. Waits for the next frame first, so a loop around
 * it runs once a frame. A held direction auto-repeats. */
unsigned char in_read(void);

#endif /* FUJIIN_H */
