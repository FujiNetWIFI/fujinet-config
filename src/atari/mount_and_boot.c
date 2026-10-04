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

void mount_and_boot_hisio(void)
{
    screen_mount_and_boot();
    set_active_screen(SCREEN_MOUNT_AND_BOOT);
    screen_clear();
    screen_puts(3, 0, "MOUNTING HISIO BOOT");

    if (!fuji_set_boot_mode(3))
    {
        screen_error("ERROR MOUNTING HISIO BOOT");
        wait_a_moment();
        state = HOSTS_AND_DEVICES;
        return;
    }

    screen_puts(9, 22, "SUCCESSFUL! BOOTING");
    cold_start();
}

void mount_and_boot_selected(void)
{
    if (hisio_boot_enabled)
        mount_and_boot_hisio();
    else
        mount_and_boot();
}

/* Use individual mounts for compatibility with older firmware and identify
 * a bad slot instead of relying on the newer aggregate mount command. */
static bool mount_slots_individually(void)
{
    unsigned char i, hs, disk_mode;
    char message[] = "ERROR MOUNTING D1: - CHECK HOST/FILE";
    for (i = 0; i < NUM_DEVICE_SLOTS; ++i)
    {
        if (deviceSlots[i].hostSlot == 0xff || !deviceSlots[i].file[0])
            continue;
        hs = deviceSlots[i].hostSlot;
        disk_mode = deviceSlots[i].mode & 3;
        if (!disk_mode) disk_mode = 1;
        if (hs >= NUM_HOST_SLOTS ||
            !fuji_mount_host_slot(hs) ||
            !fuji_mount_disk_image(i, disk_mode))
        {
            message[16] = '1' + i;
            screen_error(message);
            return false;
        }
    }
    return true;
}

static void return_to_slots(void)
{
    wait_a_moment();
    state = HOSTS_AND_DEVICES;
    slots_dirty = true;
    screen_hosts_and_devices(hostSlots, deviceSlots, deviceEnabled);
    if (hd_subState == HD_DEVICES)
        screen_hosts_and_devices_devices();
    else
        screen_hosts_and_devices_hosts();
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

    if (!mount_slots_individually())
    {
        return_to_slots();
    }
    else
    {
        screen_puts(9, 22, "SUCCESSFUL! BOOTING");
        if (!fuji_set_boot_config(0))
        {
            screen_error("ERROR SELECTING DISK BOOT");
            return_to_slots();
            return;
        }
        cold_start();
    }

}

#endif
