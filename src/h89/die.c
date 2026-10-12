#ifdef BUILD_H89

/**
 * @file die.c
 * @brief Giving up, and quitting - Heathkit H89.
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 */

#include <stdlib.h>
#include "../die.h"
#include "../screen.h"
#include "kbd.h"
#include "ui.h"

/** After screen_error(): wait for a key, then back to the OS */
void die(void)
{
  ui_hints("Press any key to return to the OS.", NULL);
  kbd_wait();
  quit();
}

/** Back to the OS without mounting anything */
void quit(void)
{
  screen_end();
  exit(0);
}

#endif /* BUILD_H89 */
