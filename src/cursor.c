/**
 * FujiNet Config for Adam
 * 
 * Cursor routines
 */

#include <msx.h>
#include "cursor.h"

void cursor(bool t)
{
  msx_vfill(0x3800,t == true ? 0xFF : 0x00, 8);
  msx_vfill(0x1b00,0x00,4);
  msx_vpoke(0x1b03,4);
}

void cursor_pos(unsigned char x, unsigned char y)
{
  msx_vpoke(0x1b00,(y<<3)-1);
  msx_vpoke(0x1b01,(x<<3)+1);
}
