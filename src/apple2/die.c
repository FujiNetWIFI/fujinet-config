/**
 * FujiNet for Coleco #Adam
 *
 * Die function
 */
#ifdef BUILD_APPLE2
#ifdef BUILD_A2CDA
#pragma cda "FujiNet Config" Start ShutDown
#endif /* BUILD_A2CDA */

#include "../die.h"

void die(void)
{
  while(1) {}
}

void quit(void)
{
}
#endif
