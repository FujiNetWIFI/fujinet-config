#ifdef BUILD_C64
/**
 * Bar routines
 */

#include "bar.h"

/**
 * static local variables for bar y, max, and index.
 */
static unsigned char bar_y=3, bar_c=1, bar_m=1, bar_i=0, bar_oldi=0;

void bar_clear(void)
{
}

/**
 * Update bar display
 */
void bar_update(void)
{
}

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
  bar_m = m-1;
  bar_i = i;
  bar_oldi = bar_i;
  bar_update();
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
 * Jump to bar slot
 */
void bar_jump(unsigned char i)
{
  bar_oldi=bar_i;
  bar_i=i;
  bar_update();
}

/**
 * Get current bar position
 * @return bar index
 */
unsigned char bar_get()
{
  return bar_i;
}

#endif /* BUILD_C64 */
