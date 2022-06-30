#ifdef BUILD_ATARI

#include "mount_and_boot.h"
#include "io.h"
#include "globals.h"
#include "screen.h"
#include "../die.h"
#include "atari_die.h"
#include <stdio.h>
#include <conio.h>

void mount_and_boot_all_hosts(void)
{
    unsigned char i;
    unsigned char retry = 5;
    char temp[40];

    io_get_device_slots(&deviceSlots[0]);

    // Loop through all the device slots. For each one that has an image mounted from a host, mount that host.
    //
    for (i = 0; i < NUM_DEVICE_SLOTS; i++)
    {
        if (deviceSlots[i].hostSlot != 0xFF)
        {   
            sprintf(temp, "Device %d: uses Host %d, mounting Host", i+1, deviceSlots[i].hostSlot+1);
            screen_puts(0, 2 + i, temp);
            while (retry > 0)
            {
                io_mount_host_slot(deviceSlots[i].hostSlot);
                if (io_error())
                    retry--;
                else
                    break;
            }

            if (io_error())
            {
                screen_error("ERROR MOUNTING HOST SLOT");
                wait_a_moment();
                state = HOSTS_AND_DEVICES;
                return;
            }
        }
        else
        {
            sprintf(temp, "Device %d: empty", i+1);
            screen_puts(0, 2 + i, temp);
        }
    }
}

/**
 * Mount all devices
 */
void mount_and_boot_all_devices(void)
{
    unsigned char i;
    unsigned char retry = 5;
    char temp[40];

    for (i = 0; i < NUM_DEVICE_SLOTS; i++)
    {
        if (deviceSlots[i].hostSlot != 0xFF)
        {
            sprintf(temp, "Device %d: Mounting image from Host %d", i+1, deviceSlots[i].hostSlot+1 );
            screen_puts(0, 4 + NUM_HOST_SLOTS + i, temp);

            while (retry > 0)
            {
                io_mount_disk_image(i, deviceSlots[i].mode);

                if (io_error())
                    retry--;
                else
                    break;
            }

            if (io_error())
            {
                screen_error("ERROR MOUNTING DEVICE SLOT");

                wait_a_moment();
                state = HOSTS_AND_DEVICES;
                return;
            }
        }
        else
        {
            sprintf(temp, "Device %d: empty", i+1);
            screen_puts(0, 4 + NUM_HOST_SLOTS + i, temp);
        }
    }

    screen_puts(9, 22, "SUCCESSFUL! BOOTING");
}

void mount_and_boot(void)
{
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

    mount_and_boot_all_hosts();
    mount_and_boot_all_devices();

    io_set_boot_config(0);
    cold_start();
}

#endif