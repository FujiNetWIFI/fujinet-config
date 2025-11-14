#ifdef _CMOC_VERSION_

#include "mount_and_boot.h"
#include "../screen.h"
#include "../typedefs.h"
#include "../globals.h"
#include "../system.h"

void mount_and_boot_lobby(void)
{
	if (screen_mount_and_boot_lobby())
	{
		fuji_set_boot_mode(2);
		system_boot();
	}
	else
	{
		if (hd_subState == HD_HOSTS)
		{
			screen_hosts_and_devices_hosts();
		}
		else if (hd_subState == HD_DEVICES)
		{
			screen_hosts_and_devices_devices();
		}
	}
}

void mount_and_boot(void)
{
  fuji_mount_all();
  exit(1);
}

#endif
