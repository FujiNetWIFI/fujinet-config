#ifdef BUILD_CONFIG86

/**
 * Functions to display a selection bar
 */

#include <stdbool.h>
#include "bar.h"

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
}

/**
 * Draw bar at y
 */
void bar_draw(int y, bool clear)
{
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
}

/**
 * Move bar downward until index m
 */
void bar_down()
{
}

/**
 * @brief jump to bar position
 */
void bar_jump(int i)
{
}

/**
 * Update bar display
 */
void bar_update(void)
{  
}

#endif
