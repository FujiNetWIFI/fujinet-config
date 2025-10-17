#include "../system.h"
#include "../typedefs.h"
#include "../globals.h"
#include <string.h>

static NewDisk newDisk;
unsigned char wifiEnabled=true;
char response[256];

void system_boot(void)
{
  return;
}

void system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                       uint32_t selected_size, const char *path)
{
  switch (selected_size)
  {
  case 90:
    newDisk.numSectors = 720;
    newDisk.sectorSize = 128;
    break;
  case 130:
    newDisk.numSectors = 1040;
    newDisk.sectorSize = 128;
    break;
  case 180:
    newDisk.numSectors = 720;
    newDisk.sectorSize = 256;
    break;
  case 360:
    newDisk.numSectors = 1440;
    newDisk.sectorSize = 256;
    break;
  case 720:
    newDisk.numSectors = 2880;
    newDisk.sectorSize = 256;
    break;
  case 1440:
    newDisk.numSectors = 5760;
    newDisk.sectorSize = 256;
    break;
  case 999:
    newDisk.numSectors = custom_numSectors;
    newDisk.sectorSize = custom_sectorSize;
    break;
  default:
    return;
  }

  newDisk.hostSlot = selected_host_slot;
  newDisk.deviceSlot = selected_device_slot;
  deviceSlots[selected_device_slot].mode = mode;
  strcpy(newDisk.filename, path);

  deviceSlots[selected_device_slot].mode = mode;
  fuji_create_new(&newDisk);

}
