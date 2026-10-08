/* fujiin.h -- the joypad in port 1, and Pause.
 *
 * Button 1 picks, button 2 goes back, and Pause opens the screen's action
 * menu. There are no frame interrupts: the VDP's frame flag is polled, so
 * nothing ever interrupts a VDP address write or a mailbox transaction. Each
 * new frame the poll ticks the sound and the cursor blink.
 */

#ifndef FUJIIN_H
#define FUJIIN_H

#include <stdbool.h>

#define IN_NONE    0
#define IN_UP      1
#define IN_DOWN    2
#define IN_LEFT    3
#define IN_RIGHT   4
#define IN_FIRE    5        /* button 1 */
#define IN_KEYSTAR 0x1A     /* button 2: back / cancel / delete */
#define IN_MENU    0x1C     /* Pause */
/* The screens' own actions, as the Coleco keypad numbered them; the Pause
 * menu hands these back. */
#define IN_KEY0    0x10
#define IN_KEYHASH 0x1B

void in_init(void);

/* Frames since in_init(), counted off the VDP frame flag. Wraps at 256. */
unsigned char in_frames(void);

/* One event, or IN_NONE. A held direction auto-repeats: first after 24
 * frames, then every 5 (Phantasy Star's name-entry timing). */
unsigned char in_read(void);

/* Pop-up menus don't repeat, the way Phantasy Star's don't. */
void in_repeat(bool on);

/* Let n frames go by: the sound and the cursor blink keep running, input is
 * left for the next in_read(). */
void in_wait(unsigned char n);

#endif /* FUJIIN_H */
