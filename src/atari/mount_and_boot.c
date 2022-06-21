#ifdef BUILD_ATARI

#include "mount_and_boot.h"
#include "io.h"
#include "globals.h"

void mount_and_boot_all_hosts(void)
{
    unsigned char i;
    unsigned char retry = 5;

    io_get_device_slots(&deviceSlots[0]);

    for (i = 0; i < NUM_HOST_SLOTS; i++)
    {
        if (deviceSlots[i].hostSlot != 0xFF)
        {
            // text_mounting_host_slot_X[19] = i + 0x31; // update status msg.
            // screen_puts(0, 21, text_mounting_host_slot_X);
            while (retry > 0)
            {
                io_mount_host_slot(i);
                if (io_error())
                    retry--;
                else
                    break;
            }

            /*
            if (fuji_sio_error())
            {
              error(ERROR_MOUNTING_HOST_SLOT);
              wait_a_moment();
              context->state = DISKULATOR_HOSTS;
              return;
            }
            */
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

    for (i = 0; i < NUM_DEVICE_SLOTS; i++)
    {
        if (deviceSlots[i].hostSlot != 0xFF)
        {
            // text_mounting_device_slot_X[18] = i + 0x31; // update status msg
            // screen_puts(0, 21, text_mounting_device_slot_X);

            while (retry > 0)
            {
                io_mount_disk_image(i, deviceSlots[i].mode);

                if (io_error())
                    retry--;
                else
                    break;
            }

            /*
            if (fuji_sio_error())
            {
              error(ERROR_MOUNTING_DEVICE_SLOT);
              wait_a_moment();
              context->state = DISKULATOR_HOSTS;
              return;
            }
            */
        }
    }

    // screen_puts(0, 21, text_booting);
}

void mount_and_boot(void)
{
    io_get_device_slots(&deviceSlots[0]);
    io_get_host_slots(&hostSlots[0]);
    mount_and_boot_all_hosts();
    mount_and_boot_all_devices();
    io_set_boot_config(0);
    cold_start();
}

#endif