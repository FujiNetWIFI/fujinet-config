#ifdef BUILD_MODEL2

/**
 * @file system.c
 * @brief Leaving CONFIG and creating disk images - TRS-80 Model II.
 */

#include <stdlib.h>
#include <string.h>
#include "../system.h"
#include "../globals.h"
#include "../constants.h"
#include "../screen.h"
#include "../pause.h"
#include "con.h"

char response[256];

static void print(const char *s)
{
  while (*s)
    con_out((uint8_t)*s++);
}

/**
 * The Model II can't boot from a FujiNet drive, so this is the way back to
 * CP/M, with the drives mounted. The screen has been put back already
 * (screen_end).
 */
void system_boot(void)
{
  static uint8_t i, n;

  n = 0;
  for (i = 0; i < NUM_DEVICE_SLOTS; i++)
  {
    if (deviceSlots[i].hostSlot >= NUM_HOST_SLOTS || !deviceSlots[i].file[0])
      continue;
    if (!n++)
      print("FujiNet drives:\r\n");
    print("  ");
    con_out('1' + i);
    print((deviceSlots[i].mode & 0x03) == MODE_WRITE ? " R/W " : " R/O ");
    print((const char *)hostSlots[deviceSlots[i].hostSlot]);
    print(":");
    print((const char *)deviceSlots[i].file);
    print("\r\n");
  }
  if (!n)
    print("FujiNet: no drives mounted.\r\n");

  exit(0);
}

bool system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                       uint32_t selected_size, const char *path)
{
  static NewDisk nd;

  (void)selected_size;
  memset(&nd, 0, sizeof(nd));
  /* IBM-3740, as MekkoGX builds: 77 tracks of 26 128-byte sectors */
  nd.numSectors = 77 * 26;
  nd.sectorSize = 128;
  nd.hostSlot = selected_host_slot;
  nd.deviceSlot = selected_device_slot;
  strncpy(nd.filename, path, sizeof(nd.filename) - 1);

  return fuji_create_new(&nd);
}

#endif /* BUILD_MODEL2 */
