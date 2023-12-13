#ifdef _CMOC_VERSION_

/**
 * Functions to display a selection bar
 */

#include "bar.h"
#include "stdbool.h"

unsigned char bar_y=0;

void bar_clear(bool old)
{
}

/**
 * Draw bar at y
 */
void bar_draw(int y, bool clear)
{
  char *sp = (unsigned char *)0x400;
  int o = y << 5;

  sp += o;

  for (int i=0;i<32;i++)
    {
      if (clear)
	{
	  sp |= 0x40; // Set bit 6
	}
      else
	{
	  sp &= 0xBF; // Clear bit 6
	}
    }
}

/**
 * Get current bar position
 * @return bar index
 */
unsigned char bar_get()
{
  return bar_y;
}

void bar_up()
{
  bar_y--;

  bar_show(bar_y);
}

void bar_down()
{
  bar_y++;

  bar_show(bar_y);
}


/**
 * Update bar display
 */
void bar_update(void)
{  
  bar_clear(true); 
}

/**
 * Show bar at Y position
 */
void bar_show(unsigned char y)
{
  bar_y = y;
}


#endif
