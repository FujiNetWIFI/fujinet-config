#ifdef _CMOC_VERSION__

/**
 * Functions to display a selection bar
 */

#include "bar.h"

/**
 * Clear bar from screen
 */
void bar_clear(bool old)
{
}

/**
 * Set bar color
 */
void bar_set_color(unsigned char c)
{
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
