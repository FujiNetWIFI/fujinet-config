#ifdef BUILD_TRS80M1

/**
 * @file bar.c
 * @brief Selection bar - TRS-80 Model I.
 *
 * The bar is a > in front of a line of the list; vid.c lays it over the
 * screen as it sends it. bar_get() is the index into the list, not a screen
 * line.
 */

#include "../screen.h"
#include "vid.h"

static uint8_t bar_y, bar_c, bar_m, bar_i;

void bar_update(void)
{
  vid_bar(bar_y + bar_i, bar_c);
}

void bar_set(uint8_t row, uint8_t col, uint8_t count, uint8_t idx)
{
  bar_y = row;
  bar_c = col;
  bar_m = count ? count - 1 : 0;
  bar_i = idx > bar_m ? bar_m : idx;
  bar_update();
}

void bar_clear(bool oldRow)
{
  (void)oldRow;
  vid_bar(VID_NONE, 0);
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

void bar_jump(uint_fast8_t idx)
{
  bar_i = (uint8_t)idx > bar_m ? bar_m : (uint8_t)idx;
  bar_update();
}

uint_fast8_t bar_get(void)
{
  return bar_i;
}

#endif /* BUILD_TRS80M1 */
