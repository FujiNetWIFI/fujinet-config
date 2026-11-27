#ifdef BUILD_TRS80M1

/**
 * @file mount_and_boot.c
 * @brief Mount & Boot - TRS-80 Model I.
 *
 * Every drive with an image in it is mounted, showing how each one went.
 * Drive 0 is slot 1, so system_boot() then starts the ROM over from it,
 * unless slot 1 failed.
 */

#include "../mount_and_boot.h"
#include "../globals.h"
#include "../constants.h"
#include "../screen.h"
#include "../system.h"
#include "trs80m1.h"
#include "kbd.h"

bool boot_ready;

void mount_and_boot(void)
{
  static uint8_t slot_idx, failed, mode_bits;
  static DeviceSlot *slot;

  screen_mount_and_boot();

  failed = 0;
  boot_ready = false;
  for (slot_idx = 0; slot_idx < FUJI_MAX_DISKS; slot_idx++)
  {
    slot = &deviceSlots[slot_idx];
    if (slot->hostSlot >= FUJI_MAX_HOSTS || !slot->file[0])
      continue;

    screen_mount_and_boot_slot(slot_idx, MOUNT_BUSY);
    mode_bits = slot->mode & (FUJI_DISK_MODE_READ | FUJI_DISK_MODE_WRITE);
    if (fuji_mount_host_slot(slot->hostSlot)
        && fuji_mount_disk_image(slot_idx, mode_bits ? mode_bits : FUJI_DISK_MODE_READ))
    {
      screen_mount_and_boot_slot(slot_idx, MOUNT_OK);
      if (!slot_idx)
        boot_ready = true;
    }
    else
    {
      screen_mount_and_boot_slot(slot_idx, MOUNT_FAIL);
      failed++;
    }
  }

  screen_mount_and_boot_done(failed);
  if (failed)
    kbd_wait();
  if (!boot_ready)
    return;

  fuji_set_boot_config(0);
  system_boot();
}

#endif /* BUILD_TRS80M1 */
