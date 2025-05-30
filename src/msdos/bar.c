#ifdef __WATCOMC__

/**
 * @brief   Bar routines for MS-DOS
 * @author  Thomas Cherryhomes
 * @email   thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 */

#include <stdlib.h> // for NULL.
#include <dos.h>
#include "bar.h"
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
  bar_y = y;
  bar_c = c;
  bar_m = (m == 0 ? 0 : m-1);
  bar_i = i;
  bar_oldi = bar_i;
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
 * Draw bar at y
 */
void bar_draw(int y, bool clear)
{
    int o = y * screen_cols * 2 + 1;
    int i=0;
    
    for (i=0;i<screen_cols;i++)
    {
        if (clear)
	{
            video[o] = 0x00;
	}
        else
	{
            video[o] = 0x10;
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

#endif /* __WATCOMC__ */
