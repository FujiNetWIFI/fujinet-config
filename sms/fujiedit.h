/* fujiedit.h -- the on-screen keyboard.
 *
 * A Master System pad has two buttons and a d-pad, so any text a user has
 * to type -- a host name, a WiFi password -- gets typed on screen. This is
 * Phantasy Star's name-entry screen: characters spaced out on every other
 * column and row, an underline sprite for the cursor, the action words
 * along the bottom. The 63 characters from '!' to '_' fill 13 columns by 5
 * rows (space is an action), so there is no shift key and no paging.
 *
 * Two rules inherited deliberately from the Intellivision client's
 * grid_entry and the Astrocade's edit.inc:
 *
 *   - CASE is a toggle that redraws the grid in real lowercase (and @[\]^
 *     as `{|}~), not a shift modifier. You can see what you are about to
 *     type.
 *   - Cancel is the ESC cell and nothing else. The backspace key must never
 *     discard the edit, because reaching for it repeatedly while correcting a
 *     typo is exactly when a reflexive extra press happens.
 *
 * The editor runs NO mailbox transactions. That is a contract, not an
 * accident: callers hold data in the cartridge's reply window across the call
 * -- the host-slot writer reads 8 slots, edits one here, and streams all 8
 * back out of that same window -- and any transaction would repaint it.
 */

#ifndef FUJIEDIT_H
#define FUJIEDIT_H

#include <stdbool.h>

/* The edit buffer. The caller preloads it NUL-terminated and reads it back
 * afterwards; it is mutated either way, so copy out only on accept. Sized for
 * the longest thing anyone types here, a 63-character WiFi password. */
#define FN_ENTRY_MAX 64
extern char fn_entry[FN_ENTRY_MAX];

/* Run the editor over fn_entry: `title` goes in the window's top border,
 * `prompt` is typed into the message window. `maxlen` is the longest string
 * to allow, not counting the NUL. Returns true on accept, false on cancel. */
bool fn_edit(const char *title, const char *prompt, unsigned char maxlen);

#endif /* FUJIEDIT_H */
