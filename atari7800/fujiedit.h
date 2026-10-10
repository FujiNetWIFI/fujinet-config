/* fujiedit.h -- the on-screen keyboard.
 *
 * The editor is a prompt ("HOST NAME?" over the value and its blinking
 * cursor) above the same 16 by 4 grid the Intellivision, ColecoVision and
 * NES clients use, where the cursor position IS the character. Button 1
 * picks the cell, button 2 (or RESET) deletes, SELECT toggles case and
 * PAUSE accepts; with a one-button joystick the CASE, DEL and OK cells do
 * the same. Cancel is the ESC cell and nothing else, so a reflexive extra
 * press while correcting a typo never throws the edit away.
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
