#ifdef BUILD_APPLE2
/**
 * Bar routines
 */

#include <peekpoke.h>
#include <conio.h>
#include "bar.h"

/**
 * static local variables for bar y, max, and index.
 */
static unsigned char bar_y=3, bar_c=1, bar_m=1, bar_i=0, bar_oldi=0;

void bar_clear(bool oldRow)
{
  char i;
  char by;

  if (oldRow)
    by = bar_y + bar_oldi;
  else
    by = bar_y + bar_i;

  gotoy(by);
  for (i = 0; i < 40; i++)
    CURRENT_LINE[i] |= 0x80; // white char on black background is in upper half of char set
}

/**
 * Update bar display
 */
void bar_update(void)
{
  char i;

  bar_clear(true);

  // Clear bar color
    gotoy(bar_y + bar_i);
    for (i = 0; i < 40; i++)
      if (CURRENT_LINE[i] < 0xe0)
        CURRENT_LINE[i] &= 0x3f; // black char on white background is in lower half of char set
      else
        CURRENT_LINE[i] &= 0x7f; // keep bit 6 for lowercase
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
void bar_up(void)
{
  bar_oldi = bar_i;

  if (bar_i > 0)
    {
      bar_i--;
      bar_update();
    }
}

/**
 * Move bar downward until index m
 */
void bar_down(void)
{
  bar_oldi = bar_i;

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
  bar_oldi = bar_i;
  bar_i = i;
  bar_update();
}

/**
 * Get current bar position
 * @return bar index
 */
unsigned char bar_get(void)
{
  return bar_i;
}

#endif /* BUILD_APPLE2 */
