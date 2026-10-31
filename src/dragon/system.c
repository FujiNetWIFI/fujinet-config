#include "../system.h"
#include "../pause.h"
#include "../globals.h"
#include "strendswith.h"

static NewDisk newDisk;
char response[256];

bool system_slot0_is_rom(void)
{
  static char fn[256];
  char *name;

  if (!fuji_get_device_filename(0, fn))
    return false;
  name = strupr(fn);
  return strendswith(name, ".CCC") || strendswith(name, ".ROM");
}

void system_boot(void)
{
  if (system_slot0_is_rom()) {
    printf("\nBOOTING USER ROM...\n");
    pause(120);                 
    fuji_enable_user_rom();
  }
  else
  {
    pause(120);
  }

#ifndef DRAGON
  coldStart();
#endif
  exit(0);
}

bool system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                       uint32_t selected_size, const char *path)
{
  memset(&newDisk, 0, sizeof(newDisk));
  newDisk.numDisks = (byte)selected_size;
  newDisk.hostSlot = selected_host_slot;
  newDisk.deviceSlot = selected_device_slot;
  strcpy(newDisk.filename, path);

  return fuji_create_new(&newDisk);
}

