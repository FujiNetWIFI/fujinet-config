#ifdef _CMOC_VERSION_

#include "mount_and_boot.h"
#include "../typedefs.h"

void mount_and_boot_lobby(void)
{
}

void mount_and_boot(void)
{
  fuji_mount_all();
  exit(1);
}

#endif
