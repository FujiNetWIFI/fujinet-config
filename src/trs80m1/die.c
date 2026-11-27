#ifdef BUILD_TRS80M1

/**
 * @file die.c
 * @brief Giving up, and quitting - TRS-80 Model I.
 */

#include "../die.h"
#include "../screen.h"
#include "con.h"
#include "kbd.h"
#include "ui.h"

/** After screen_error(): wait for a key, then start the ROM over */
void die(void)
{
  ui_hints("Press any key to restart.", NULL);
  kbd_wait();
  quit();
}

/** There is no DOS to go back to: the ROM starts over from drive 0 */
void quit(void)
{
  con_reboot();
}

#endif /* BUILD_TRS80M1 */
