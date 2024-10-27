#ifdef BUILD_APPLE2

#include "mount_and_boot.h"
#include "io.h"
#include "globals.h"
#include "screen.h"
#include "../die.h"
#ifdef __ORCAC__
#include <coniogs.h>
#else
#include <conio.h>
#endif

void mount_and_boot_lobby(void)
{
    if (screen_mount_and_boot_lobby())
    {
        io_set_boot_mode(2);
        io_boot();
    }
    else
    {
        screen_hosts_and_devices_hosts();
    }
}

#endif