/* fujiin.h -- controller input through cc65's joystick driver, and an
 * expansion-port keyboard (Family BASIC or Subor) when one answers.
 *
 * Edge-triggered events, one per call, with the ColecoVision CONFIG's
 * vocabulary so the screen modules port across: IN_FIRE is A, IN_BACK is B,
 * IN_SELECT and IN_START are what the keypad digits were there. On a
 * keyboard the arrows move, RETURN is A and ESC / DEL are B, so every screen
 * works from either; the text editor reads typed text through in_read_text.
 */

#ifndef FUJIIN_H
#define FUJIIN_H

#include <stdbool.h>

#define IN_NONE   0
#define IN_UP     1
#define IN_DOWN   2
#define IN_LEFT   3
#define IN_RIGHT  4
#define IN_FIRE   5         /* A */
#define IN_BACK   6         /* B */
#define IN_SELECT 7
#define IN_START  8
/* in_read_text only: a typed character (in_char()), and the editor's keys */
#define IN_CHAR   9
#define IN_ENTER  10
#define IN_ESC    11
#define IN_BS     12

void in_init(void);

/* Vblanks since start-up, wrapping at 256: for the short waits. */
unsigned char in_frames(void);

/* One event, or IN_NONE. Auto-repeats a held direction so paging a long
 * directory does not need 40 separate pushes. Waits for vblank first, so a
 * loop around it runs once per frame. */
unsigned char in_read(void);

/* The same, but a keyboard's typing comes back as IN_CHAR / IN_ENTER /
 * IN_ESC / IN_BS instead of being folded into the pad's events. */
unsigned char in_read_text(void);

/* The character behind the last IN_CHAR. */
char in_char(void);

/* Whether a keyboard answered at start-up. */
bool in_has_keyboard(void);

#endif /* FUJIIN_H */
