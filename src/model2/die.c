#ifdef BUILD_MODEL2

/**
 * @file die.c
 * @brief Giving up, and quitting - TRS-80 Model II.
 */

#include <stdlib.h>
#include "../die.h"
#include "../screen.h"
#include "kbd.h"
#include "ui.h"

/** After screen_error(): wait for a key, then back to CP/M */
void die(void)
{
  ui_hints("Press any key to return to CP/M.", NULL);
  kbd_wait();
  quit();
}

/** Back to CP/M without mounting anything */
void quit(void)
{
  screen_end();
  exit(0);
}

#endif /* BUILD_MODEL2 */
