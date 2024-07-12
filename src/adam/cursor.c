#ifdef BUILD_ADAM
/**
 * Cursor routines
 */

#include <video/tms99x8.h>
#include "cursor.h"

static const unsigned char blank[8] =
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static const unsigned char _cursor[8] =
  {0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF};

void cursor(bool t)
{
  vdp_set_sprite_8(0,t ? _cursor : blank);
}

void cursor_pos(unsigned char x, unsigned char y)
{
  vdp_put_sprite_8(0,x<<3,y<<3,0,VDP_INK_DARK_BLUE);
}

#endif /* BUILD_ADAM */
