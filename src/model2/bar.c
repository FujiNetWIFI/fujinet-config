#ifdef BUILD_MODEL2

/**
 * @file bar.c
 * @brief Selection bar - TRS-80 Model II.
 *
 * The bar is a line of the list shown in reverse video, with a > at its
 * left; vid.c lays it over the screen as it sends it. bar_get() is the
 * index into the list, not a screen line.
 */

#include "../screen.h"
#include "vid.h"

static uint8_t bar_y, bar_c, bar_m, bar_i;

void bar_update(void)
{
  vid_bar(bar_y + bar_i, bar_c, VID_COLS - 1 - bar_c);
}

void bar_set(uint8_t y, uint8_t c, uint8_t m, uint8_t i)
{
  bar_y = y;
  bar_c = c;
  bar_m = m ? m - 1 : 0;
  bar_i = i > bar_m ? bar_m : i;
  bar_update();
}

void bar_clear(bool oldRow)
{
  (void)oldRow;
  vid_bar(VID_NONE, 0, 0);
}

void bar_up(void)
{
  if (bar_i > 0)
  {
    bar_i--;
    bar_update();
  }
}

void bar_down(void)
{
  if (bar_i < bar_m)
  {
    bar_i++;
    bar_update();
  }
}

void bar_jump(uint_fast8_t i)
{
  bar_i = (uint8_t)i > bar_m ? bar_m : (uint8_t)i;
  bar_update();
}

uint_fast8_t bar_get(void)
{
  return bar_i;
}

#endif /* BUILD_MODEL2 */
