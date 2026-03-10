/**
 * Mount and boot
 */

#include <stdlib.h>
#include "io.h"

void mount_and_boot(void)
{
  io_mount_all();
  exit(0);
}
