/**
 * Bar routines
 */

#include <video/tms99x8.h>
#include <stdbool.h>
#include <conio.h>

#include "gfxutil.h"

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
// static unsigned char bar_y=3, bar_c=1, bar_m=1, bar_i=0, bar_oldi=0, bar_co=0x13;
static unsigned char bar_y=3, bar_c=3, bar_m=1, bar_i=0, bar_oldi=0, bar_co=0x17;

/**
* Reset text to default
*/
void bar_clear(bool oldRow)
{
  if (!oldRow) {
    vdp_set_sprite_8(1, blank);
    vdp_set_sprite_8(2, blank);
  }
  uint16_t coff = bar_c << 3;
  uint16_t addr = ADDR_FILE_LIST + ROW(bar_y+(oldRow==true ? bar_oldi : bar_i)) + coff;
  vdp_vwrite(row_pattern + coff, addr, 256 - coff - 16);
}

/**
 * Update bar display
 */
void bar_update(void)
{
  int coff = bar_c << 3;
  bar_clear(true);

  vdp_vfill(ADDR_FILE_LIST + ROW(bar_y+bar_i) + coff, 0xF6, 256 - coff - 16);

  uint8_t y = (bar_y+bar_i+1) << 3;
  vdp_put_sprite_8(1, coff-8, y, 1, VDP_INK_DARK_RED);
  vdp_put_sprite_8(2, 256-2*8, y, 2, VDP_INK_DARK_RED);
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
  bar_m = (m == 0 ? 0 : m-1);
  bar_i = i;
  bar_oldi = bar_i;

  bar_update();

  vdp_set_sprite_8(1, m ? pill_left : blank);
  vdp_set_sprite_8(2, m ? pill_right : blank);
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
