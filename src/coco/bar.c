#ifdef _CMOC_VERSION_

/**
 * Functions to display a selection bar
 */

#include <cmoc.h>
#include "bar.h"
#include "stdbool.h"

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
  unsigned char *sp = (unsigned char *)0x400;
  int yo = bar_y * 32;
  int io = (old ? bar_oldi*32 : bar_i*32);
  
  if (old)
    {
      bar_draw(bar_y+bar_oldi,true);
    }
  else
    {
      bar_draw(bar_y+bar_i,true);
    }

  if (bar_c != 0)
  {
      sp += yo+io;
      *sp &= 0xBF;
  }
  
}

/**
 * Draw bar at y
 */
void bar_draw(int y, bool clear)
{
  unsigned char *sp = (unsigned char *)0x400;
  int o = y << 5;

  sp += o;

  for (int i=0;i<32;i++)
    {
      if (clear)
	{
	  *sp |= 0x40; // Set bit 6
	  sp++;
	}
      else
	{
	  *sp &= 0xBF; // Clear bit 6
	  sp++;
	}
    }
}

/**
 * Get current bar position
 * @return bar index
 */
int bar_get()
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
void bar_jump(int i)
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

#endif
