/**
 * @file die.c
 * @brief MS-DOS fatal-error halt and quit stubs.
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 */
#ifdef BUILD_MSDOS
#include "../die.h"

/**
 * @brief Halt execution permanently by spinning in an infinite loop.
 */
void die(void)
{
  while(1) {}
}

/**
 * @brief Quit stub — no-op on MS-DOS.
 */
void quit(void)
{
}
#endif
