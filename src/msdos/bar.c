#ifdef BUILD_MSDOS

/**
 * @brief   Bar routines for MS-DOS
 * @author  Thomas Cherryhomes
 * @email   thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 */

#include <stdlib.h> // for NULL.
#include <dos.h>
#include "screen.h"

/**
 * static local variables for bar y, max, and index.
 */
static int bar_y=3, bar_c=1, bar_m=1, bar_i=0, bar_oldi=0;

/**
 * @brief Set up bar and start display on row.
 * @param y Starting row for bar display.
 * @param c Column size in characters (left/right margin).
 * @param m Total number of items.
 * @param i Item index to start display at.
 */
void bar_set(unsigned char y, unsigned char c, unsigned char m, unsigned char i)
{
  bar_oldi = bar_i;
  bar_y = y;
  bar_c = c;
  bar_m = (m == 0 ? 0 : m-1);
  bar_i = i;
  bar_update();
}

/**
 * @brief Clear the highlight attribute from the current or previous bar row.
 * @param old If true, clear the previously highlighted row; if false, clear the current row.
 */
void bar_clear(bool old)
{
  if (old)
    {
      bar_draw(bar_y+bar_oldi,true);
    }
  else
    {
      bar_draw(bar_y+bar_i,true);
    }
}

/**
 * @brief Draw or clear the highlight bar at a given screen row.
 *
 * Respects bar_c as the left/right margin so box borders at column 0 and
 * screen_cols-1 are not overwritten when bar_c > 0.
 *
 * @param y     Absolute screen row to draw on.
 * @param clear If true, restore normal attribute; if false, apply selected attribute.
 */
void bar_draw(int y, bool clear)
{
    int o = (y * screen_cols + bar_c) * 2 + 1;
    int w = screen_cols - 2 * bar_c;
    int i=0;

    for (i=0;i<w;i++)
    {
        if (clear)
	{
            video[o] = ATTRIBUTE_NORMAL;
	}
        else
	{
            video[o] = ATTRIBUTE_SELECTED;
	}

        o += 2;
    }
}

/**
 * Get current bar position
 * @return bar index
 */
uint_fast8_t bar_get()
{
  return bar_i;
}

/**
 * @brief Move the bar highlight up by one item, stopping at index 0.
 */
void bar_up()
{
  bar_oldi=bar_i;
  
  if (bar_i > 0)
    {
      bar_i--;
      bar_update();
    }
}

/**
 * @brief Move the bar highlight down by one item, stopping at the last index.
 */
void bar_down()
{
  bar_oldi=bar_i;

  if (bar_i < bar_m)
    {
      bar_i++;
      bar_update();
    }
}

/**
 * @brief Jump the bar directly to a given index and redraw.
 * @param i Item index to jump to.
 */
void bar_jump(uint_fast8_t i)
{
  bar_i=i;
  bar_update();
}

/**
 * @brief Redraw the bar: clear the old highlight and draw the new one.
 */
void bar_update(void)
{  
  bar_clear(true);
  bar_draw(bar_y+bar_i,false);  
}

#endif /* BUILD_MSDOS */
