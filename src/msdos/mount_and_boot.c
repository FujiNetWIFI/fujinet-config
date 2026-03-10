/**
 * Mount and boot
 */

#include <stdlib.h>
#include <fujinet-fuji.h>

void mount_and_boot(void)
{
  fuji_mount_all();
  exit(0);
}
