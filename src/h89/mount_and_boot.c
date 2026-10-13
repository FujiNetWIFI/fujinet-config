#ifdef BUILD_H89

/**
 * @file mount_and_boot.c
 * @brief Mount & Exit - Heathkit H89.
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 *
 * No FujiNet drive can boot the H89 yet, so CONFIG mounts every drive with
 * an image in it, showing how each one went, and then returns to HDOS or
 * CP/M (done() and system_boot()).
 */

#include "../mount_and_boot.h"
#include "../globals.h"
#include "../constants.h"
#include "../screen.h"
#include "h89.h"
#include "kbd.h"

void mount_and_boot(void)
{
  static uint8_t i, failed;
  static DeviceSlot *d;

  screen_mount_and_boot();

  failed = 0;
  for (i = 0; i < NUM_DEVICE_SLOTS; i++)
  {
    d = &deviceSlots[i];
    if (d->hostSlot >= NUM_HOST_SLOTS || !d->file[0])
      continue;

    screen_mount_and_boot_slot(i, MOUNT_BUSY);
    if (fuji_mount_host_slot(d->hostSlot) && fuji_mount_disk_image(i, (d->mode & 0x03) ? (d->mode & 0x03) : MODE_READ))
      screen_mount_and_boot_slot(i, MOUNT_OK);
    else
    {
      screen_mount_and_boot_slot(i, MOUNT_FAIL);
      failed++;
    }
  }

  screen_mount_and_boot_done(failed);
  if (failed)
    kbd_wait();
}

#endif /* BUILD_H89 */
