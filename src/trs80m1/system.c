#ifdef BUILD_TRS80M1

/**
 * @file system.c
 * @brief Booting the mounted image and creating disk images - TRS-80 Model I.
 */

#include <string.h>
#include "../system.h"
#include "../globals.h"
#include "../constants.h"
#include "con.h"
#include "ui.h"

char response[256];

/** Slot 1 is drive 0, which the ROM boots from: start it over. */
void system_boot(void)
{
  ui_message("Booting...");
  con_reboot();
}

bool system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                       uint32_t selected_size, const char *path)
{
  static NewDisk new_disk;

  (void)selected_size;
  memset(&new_disk, 0, sizeof(new_disk));
  /* JV1: 35 tracks of 10 256-byte sectors */
  new_disk.numSectors = 35 * 10;
  new_disk.sectorSize = 256;
  new_disk.hostSlot = selected_host_slot;
  new_disk.deviceSlot = selected_device_slot;
  strncpy(new_disk.filename, path, sizeof(new_disk.filename) - 1);

  return fuji_create_new(&new_disk);
}

#endif /* BUILD_TRS80M1 */
