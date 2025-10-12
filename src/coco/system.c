#include "../system.h"

static NewDisk newDisk;

void system_boot(void)
{
  pause(180); // Pause for 3 seconds to let the user see the mounted disks.
  coldStart();
  exit(0);
}

void system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                       uint32_t selected_size, const char *path)
{
  memset(&newDisk, 0, sizeof(newDisk));
  newDisk.numDisks = (byte)selected_size;
  newDisk.hostSlot = selected_host_slot;
  newDisk.deviceSlot = selected_device_slot;
  strcpy(newDisk.filename, path);

  fuji_create_new(&newDisk);
  return;
}

