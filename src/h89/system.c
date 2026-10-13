#ifdef BUILD_H89

/**
 * @file system.c
 * @brief Leaving CONFIG and creating disk images - Heathkit H89.
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
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
 * Nothing boots from a FujiNet drive on the H89 yet, so this is the way
 * back to HDOS or CP/M, with the drives mounted. The terminal has been put
 * back already (screen_end).
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

void system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                       uint32_t selected_size, const char *path)
{
  static NewDisk nd;

  memset(&nd, 0, sizeof(nd));
  if (selected_size == 999)
  {
    nd.numSectors = custom_numSectors;
    nd.sectorSize = custom_sectorSize;
  }
  else
  {
    /* The size in K, as 512-byte sectors */
    nd.numSectors = (uint16_t)(selected_size * 2);
    nd.sectorSize = 512;
  }
  nd.hostSlot = selected_host_slot;
  nd.deviceSlot = selected_device_slot;
  strncpy(nd.filename, path, sizeof(nd.filename) - 1);

  if (!fuji_create_new(&nd))
  {
    screen_error("The FujiNet could not create the disk image.");
    pause(150);
  }
}

#endif /* BUILD_H89 */
