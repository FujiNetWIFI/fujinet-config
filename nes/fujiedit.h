/* fujiedit.h -- the on-screen keyboard.
 *
 * The editor is a Family BASIC prompt ("HOST NAME?" over the value and its
 * blinking cursor). An NES controller has a d-pad and four buttons, so any
 * text a user has to type -- a host name, a WiFi password -- gets typed on
 * screen: the same 16 by 4 grid the Intellivision, Astrocade and ColecoVision clients use, where
 * the cursor position IS the character. A: pick the cell. B: backspace.
 * SELECT: toggle case. START: accept. Cancel is the ESC cell and nothing
 * else, so a reflexive extra press while correcting a typo never throws
 * the edit away.
 *
 * The editor runs NO mailbox transactions. Callers hold data in the
 * cartridge's reply window across the call and any transaction would
 * repaint it.
 */

#ifndef FUJIEDIT_H
#define FUJIEDIT_H

#include <stdbool.h>

#define FN_ENTRY_MAX 64
extern char fn_entry[FN_ENTRY_MAX];

/* Run the editor over fn_entry. `maxlen` is the longest string to allow, not
 * counting the NUL. Returns true on accept, false on cancel. */
bool fn_edit(const char *title, unsigned char maxlen);

#endif /* FUJIEDIT_H */
