/**
 * FujiNet for Coleco #Adam
 * 
 * Die function
 */

#include <eos.h>
#include "die.h"

void die(void)
{
  while(1) {}
}

void quit(void)
{
  eos_exit_to_smartwriter();
}
