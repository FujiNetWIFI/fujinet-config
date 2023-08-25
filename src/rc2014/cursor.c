#ifdef BUILD_RC2014
/**
 * FujiNet Config for Adam
 * 
 * Cursor routines
 */

#include <conio.h>
#include "cursor.h"

void cursor(bool t)
{
  if (t) {
    cprintf("%c[?25h", 27);
  } else {
    cprintf("%c[?25l", 27);
  }
}

void cursor_pos(unsigned char x, unsigned char y)
{
  cprintf("%c[%d;%dH", 27, y+1, x+1);
}
#endif /* BUILD_RC2014 */
