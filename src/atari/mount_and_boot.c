#ifdef BUILD_ATARI

#include "mount_and_boot.h"
#include "atari_die.h"
#include "../globals.h"
#include "../screen.h"
#include "../system.h"
#include "../constants.h"
#include <conio.h>
#include <string.h>

/* Southern AMIS Projects: server-hosted Atari BBS Gateway launcher.
 * The gateway supplies the BBS directory; CONFIG only boots the client. */
#ifndef BBS_HOST
#define BBS_HOST "tnfs.fujinet.online"
#endif
#ifndef BBS_PATH
#define BBS_PATH "/ATARI/ataribbs.xex"
#endif

void mount_and_boot_lobby(void)
{
    screen_mount_and_boot();
    set_active_screen(SCREEN_MOUNT_AND_BOOT);
    screen_clear();
    screen_puts(3, 0, "Booting Lobby");

    fuji_set_boot_mode(2);
    cold_start();
}

void mount_and_boot_bbs(void)
{
    unsigned char hs;
    bool changed = false;
    static char old_filename[256];
    static DeviceSlot old_slot;

    screen_mount_and_boot();
    set_active_screen(SCREEN_MOUNT_AND_BOOT);
    screen_clear();
    screen_puts(3, 0, "ATARI BBS GATEWAY");
    if (!fuji_get_host_slots(hostSlots, NUM_HOST_SLOTS) ||
        !fuji_get_device_slots(deviceSlots, NUM_DEVICE_SLOTS) ||
        !fuji_get_device_filename(0, old_filename))
        goto failed;
    hs = selected_host_slot;
    if (BBS_HOST[0])
    {
        for (hs = 0; hs < NUM_HOST_SLOTS; ++hs)
            if (!strcmp((char *)hostSlots[hs], BBS_HOST))
                break;
        if (hs == NUM_HOST_SLOTS)
        {
            for (hs = 0; hs < NUM_HOST_SLOTS; ++hs)
                if (!hostSlots[hs][0])
                    break;
            if (hs == NUM_HOST_SLOTS)
            {
                screen_error("ADD BBS SERVER TO A HOST SLOT");
                goto done;
            }
            strcpy((char *)hostSlots[hs], BBS_HOST);
            if (!fuji_put_host_slots(hostSlots, NUM_HOST_SLOTS))
                goto failed;
        }
    }
    if (hs >= NUM_HOST_SLOTS || !hostSlots[hs][0])
    {
        screen_error("SELECT A BBS HOST FIRST");
        goto done;
    }
    if (!fuji_mount_host_slot(hs))
        goto failed;
    old_slot = deviceSlots[0];
    if (!fuji_unmount_disk_image(0))
        goto failed;
    changed = true;
    slots_dirty = true;
    if (!fuji_set_device_filename(1, hs, 0, BBS_PATH) ||
        !fuji_mount_disk_image(0, 1) ||
        !fuji_set_boot_config(0))
        goto failed;
    cold_start();
    return;

failed:
    if (changed)
    {
        fuji_unmount_disk_image(0);
        deviceSlots[0] = old_slot;
        if (!fuji_put_device_slots(deviceSlots, NUM_DEVICE_SLOTS) ||
            (old_slot.hostSlot < NUM_HOST_SLOTS &&
             !fuji_set_device_filename(old_slot.mode, old_slot.hostSlot,
                                      0, old_filename)))
        {
            screen_error("ERROR RESTORING D1: - CHECK SLOTS");
            goto done;
        }
        if (old_slot.hostSlot < NUM_HOST_SLOTS && old_filename[0] &&
            (!fuji_mount_host_slot(old_slot.hostSlot) ||
             !fuji_mount_disk_image(0, old_slot.mode)))
        {
            screen_error("ERROR RESTORING D1: - CHECK SLOTS");
            goto done;
        }
    }
    screen_error("BBS BOOT FAILED - CHECK HOST/PATH");
done:
    wait_a_moment();
    state = HOSTS_AND_DEVICES;
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
