#ifdef BUILD_ATARI

#include "mount_and_boot.h"
#include "atari_die.h"
#include "../globals.h"
#include "../screen.h"
#include "../system.h"
#include "../constants.h"
#include <conio.h>

void mount_and_boot_lobby(void)
{
    screen_mount_and_boot();
    set_active_screen(SCREEN_MOUNT_AND_BOOT);
    screen_clear();
    screen_puts(3, 0, "Booting Lobby");

    fuji_set_boot_mode(2);
    cold_start();
}

void mount_and_boot(void)
{
    screen_mount_and_boot();
    set_active_screen(SCREEN_MOUNT_AND_BOOT);

    if (!fuji_get_device_slots(deviceSlots, NUM_DEVICE_SLOTS))
    {
        screen_error("ERROR READING DEVICE SLOTS");
        die();
    }

    if (!fuji_get_host_slots(hostSlots, NUM_HOST_SLOTS))
    {
        screen_error("ERROR READING HOST SLOTS");
        die();
    }

    screen_clear();
    screen_puts(3, 0, "MOUNT AND BOOT");

    screen_puts(0, 3, "Mounting all Host and Device Slots");

    if (!fuji_mount_all())
    {
        screen_error("ERROR MOUNTING ALL");
        wait_a_moment();
        state = HOSTS_AND_DEVICES;
    }
    else
    {
        screen_puts(9, 22, "SUCCESSFUL! BOOTING");
        fuji_set_boot_config(0);
        cold_start();
    }

}

#endif
