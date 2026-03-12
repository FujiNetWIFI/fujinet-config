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
 * Set up bar and start display on row
 * @param y Y column for bar display
 * @param c column size in characters
 * @param m number of items
 * @param i item index to start display
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
 * Draw bar at y, respecting bar_c as left and right margin so box borders
 * at column 0 and screen_cols-1 are not overwritten when bar_c > 0.
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
unsigned char bar_get()
{
  return bar_i;
}

/**
 * Move bar upward until index 0
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
 * Move bar downward until index m
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
 * @brief jump to bar position
 */
void bar_jump(unsigned char i)
{
  bar_i=i;
  bar_update();
}

/**
 * Update bar display
 */
void bar_update(void)
{  
  bar_clear(true);
  bar_draw(bar_y+bar_i,false);  
}

#endif /* BUILD_MSDOS */
