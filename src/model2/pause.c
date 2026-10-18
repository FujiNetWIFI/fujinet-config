#ifdef BUILD_MODEL2

/**
 * @file pause.c
 * @brief Waiting - TRS-80 Model II.
 */

#include "../pause.h"
#include "con.h"
#include "vid.h"

/** Wait delay 60ths of a second, on P&T's 1/100 s clock */
void pause(unsigned char delay)
{
  static uint16_t t, ticks;

  vid_flush();
  ticks = (uint16_t)delay * 5 / 3;
  t = (uint16_t)con_ticks();
  while ((uint16_t)((uint16_t)con_ticks() - t) < ticks)
    ;
}

#endif /* BUILD_MODEL2 */
