#ifdef BUILD_C64
/**
 * FujiNet Config for C64
 * 
 * Cursor routines
 */

#include "cursor.h"

#define UNUSED(x) (void)(x)

void cursor(bool t)
{
  UNUSED(t);
}

void cursor_pos(unsigned char x, unsigned char y)
{
  UNUSED(x);
  UNUSED(y);
}
#endif /* BUILD_C64 */
