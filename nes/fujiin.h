/* fujiin.h -- controller input through cc65's joystick driver.
 *
 * Edge-triggered events, one per call, with the ColecoVision CONFIG's
 * vocabulary so the screen modules port across: IN_FIRE is A, IN_BACK is B,
 * IN_SELECT and IN_START are what the keypad digits were there.
 */

#ifndef FUJIIN_H
#define FUJIIN_H

#define IN_NONE   0
#define IN_UP     1
#define IN_DOWN   2
#define IN_LEFT   3
#define IN_RIGHT  4
#define IN_FIRE   5         /* A */
#define IN_BACK   6         /* B */
#define IN_SELECT 7
#define IN_START  8

void in_init(void);

/* Vblanks since start-up, wrapping at 256: for the short waits. */
unsigned char in_frames(void);

/* One event, or IN_NONE. Auto-repeats a held direction so paging a long
 * directory does not need 40 separate pushes. Waits for vblank first, so a
 * loop around it runs once per frame. */
unsigned char in_read(void);

#endif /* FUJIIN_H */
