#ifdef BUILD_H89

/**
 * @file pause.c
 * @brief Waiting - Heathkit H89.
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 */

#include "../pause.h"
#include "con.h"
#include "h19.h"

/** Wait delay 60ths of a second, on the H89's 2 ms clock */
void pause(unsigned char delay)
{
  static uint16_t t, ticks;

  h19_flush();
  ticks = (uint16_t)delay * 25 / 3;
  t = TICKS;
  while ((uint16_t)(TICKS - t) < ticks)
    ;
}

#endif /* BUILD_H89 */
