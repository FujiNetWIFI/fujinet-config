/* fujiin.h -- the left joystick and the console's switches.
 *
 * Edge-triggered events, one per call, with the NES CONFIG's vocabulary so
 * the screens port across. Button 1 is IN_FIRE. Button 2 and RESET are
 * IN_BACK, so a one-button 2600 joystick reaches everything. SELECT is
 * IN_SELECT and PAUSE is IN_START. A ProLine joystick is read in two-button
 * mode (enabled through the RIOT alone); a 2600 joystick's button arrives
 * through INPT4.
 */

#ifndef FUJIIN_H
#define FUJIIN_H

#include <stdbool.h>

#define IN_NONE   0
#define IN_UP     1
#define IN_DOWN   2
#define IN_LEFT   3
#define IN_RIGHT  4
#define IN_FIRE   5         /* button 1 */
#define IN_BACK   6         /* button 2, or RESET */
#define IN_SELECT 7
#define IN_START  8         /* PAUSE */

void in_init(void);

/* Frames since start-up, wrapping at 256: for the short waits. */
unsigned char in_frames(void);

/* One event, or IN_NONE. Waits for the next frame first, so a loop around
 * it runs once a frame. A held direction auto-repeats. */
unsigned char in_read(void);

#endif /* FUJIIN_H */
