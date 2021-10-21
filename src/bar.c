/**
 * Bar routines
 */

#include <msx.h>
#include "bar.h"

#define DISPLAY_ROWS 17
#define DISPLAY_LENGTH 60
#define ADDR_PATH_LINE MODE2_ATTR
#define ATTR_PATH_LINE 0xF5
#define ADDR_FILE_LIST ADDR_PATH_LINE + 256
#define ATTR_FILE_LIST 0x1F
#define ADDR_FILE_TYPE ADDR_FILE_LIST
#define ATTR_FILE_TYPE 0xF5
#define ATTR_BAR 0x13

#define ROW(x) (x<<8)
#define COL(x) (x<<3)

/**
 * static local variables for bar y, max, and index.
 */
static unsigned char bar_y, bar_c, bar_m, bar_i, bar_oldi;

/**
 * Update bar display
 */
void bar_update(void)
{
  // Fill column color in old row
  msx_vfill(ADDR_FILE_LIST + ROW(bar_y+bar_oldi), ATTR_PATH_LINE, COL(bar_c));

  // Fill background color in old row
  msx_vfill(ADDR_FILE_LIST + ROW(bar_y+bar_oldi)+COL(bar_c),ATTR_FILE_LIST,256-COL(bar_c));
  
  // Fill bar color in new row
  msx_vfill(ADDR_FILE_LIST + ROW(bar_y+bar_i),ATTR_BAR,256);
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
  bar_m = m;
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
 * Get current bar position
 * @return bar index
 */
unsigned char bar_get()
{
  return bar_i;
}
