/**
 * FujiNet for Coleco #Adam
 * 
 * Die function
 */
#ifdef BUILD_ADAM
#include <eos.h>
#endif
#include "die.h"

void die(void)
{
  while(1) {}
}

void quit(void)
{
#ifdef BUILD_ADAM
  eos_exit_to_smartwriter();
#endif
}
