#ifdef BUILD_C64
/**
 * Bar routines
 */

#include <c64.h>
#include "bar.h"

#define TEXT_RAM ((unsigned char *)0x0400)

#define COLOR_DESELECT 14;
#define COLOR_SELECT   13;

/**
 * static local variables for bar y, max, and index.
 */
static unsigned char bar_y=3, bar_c=1, bar_m=1, bar_i=0, bar_oldi=0;

unsigned short bar_coord(unsigned char x, unsigned char y)
{
  return (y*40)+x;
}

void bar_clear(bool oldRow)
{
  char i;
  char by;
  unsigned short o;

  if (oldRow)
    by = bar_y + bar_oldi;
  else
    by = bar_y + bar_i;

  o = bar_coord(0,by);

  for (i=0;i<40;i++)
    {
      COLOR_RAM[o+i] = COLOR_DESELECT;
      TEXT_RAM[o+i] &= 0x7F;
    }
}

/**
 * Update bar display
 */
void bar_update(void)
{
  unsigned short o;
  unsigned char i;

  o = bar_coord(0,bar_y+i);

  for (i=0;i<40;i++)
    {
      COLOR_RAM[o+i] = COLOR_SELECT;
      TEXT_RAM[o+i] |= 0x80;
    }
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
