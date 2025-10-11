#ifdef _CMOC_VERSION_

#include "mount_and_boot.h"
#include "../io.h"

void mount_and_boot_lobby(void)
{
}

void mount_and_boot(void)
{
  
  io_mount_all();
  exit(1);
}

#endif
