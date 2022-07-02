#ifdef BUILD_ATARI

#include "mount_and_boot.h"
#include "io.h"
#include "globals.h"
#include "screen.h"
#include "../die.h"
#include "atari_die.h"
#include <stdio.h>
#include <conio.h>


void mount_and_boot(void)
{
    unsigned char i;
    char temp[40];

    screen_mount_and_boot();
    set_active_screen(SCREEN_MOUNT_AND_BOOT);

    io_get_device_slots(&deviceSlots[0]);
    if (io_error())
    {
        screen_error("ERROR READING DEVICE SLOTS");
        die();
    }

    io_get_host_slots(&hostSlots[0]);
    if (io_error())
    {
        screen_error("ERROR READING HOST SLOTS");
        die();
    }

    screen_clear();
    screen_puts(3, 0, "MOUNT AND BOOT");

    for (i = 0; i < NUM_DEVICE_SLOTS; i++)
    {
        if (deviceSlots[i].hostSlot != 0xFF)
        {
            sprintf(temp, "Device %d: uses Host %d, mounting Host", i + 1, deviceSlots[i].hostSlot + 1);
            screen_puts(0, 2 + i, temp);
        }
        else
        {
            sprintf(temp, "Device %d: empty", i + 1);
            screen_puts(0, 2 + i, temp);
        }
    }

    for (i = 0; i < NUM_DEVICE_SLOTS; i++)
    {
        if (deviceSlots[i].hostSlot != 0xFF)
        {
            sprintf(temp, "Device %d: Mounting image from Host %d", i + 1, deviceSlots[i].hostSlot + 1);
            screen_puts(0, 4 + NUM_HOST_SLOTS + i, temp);
        }
        else
        {
            sprintf(temp, "Device %d: empty", i + 1);
            screen_puts(0, 4 + NUM_HOST_SLOTS + i, temp);
        }
    }

    if ( !io_mount_all() )
    {
        screen_error("ERROR MOUNTING ALL");
        wait_a_moment();
        //die();
        state = HOSTS_AND_DEVICES;
    }
    else
    {
        screen_puts(9, 22, "SUCCESSFUL! BOOTING");
        io_set_boot_config(0);
        cold_start();
    }

}

#endif