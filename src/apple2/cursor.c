#ifdef BUILD_APPLE2
/**
 * FujiNet Config for Apple2
 *
 * Cursor routines
 */

#ifdef BUILD_A2CDA
#pragma cda "FujiNet Config" Start ShutDown
#endif /* BUILD_A2CDA */

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
#endif /* BUILD_APPLE2 */
