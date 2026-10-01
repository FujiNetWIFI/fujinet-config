#include "../system.h"
#include "../pause.h"
#include "../globals.h"
#include "strendswith.h"

extern char *screen_upper(char *s);

#define FUJI_IO_REG ((unsigned char *)0xFF41)

static NewDisk newDisk;
char response[256];

void system_enable_user_rom(void)
{
  *FUJI_IO_REG = 7;
}

bool system_slot0_is_rom(void)
{
  static char fn[256];
  char *name;

  if (!fuji_get_device_filename(0, fn))
    return false;
  name = screen_upper(fn);
  return strendswith(name, ".CCC") || strendswith(name, ".ROM");
}

void system_boot(void)
{
  if (system_slot0_is_rom()) {
    printf("\nBOOTING USER ROM...\n");
    pause(120);                 
    system_enable_user_rom();   // write 7 (ROM_MODE | USERROM_ENABLE | AUTOSTART_ENABLE) to IO_CONTROL. Firmware arms the CART trigger and triggers RESET.
  }
  else
  {
    pause(120);
  }

  // If the ROM detection code runs on a Rev0000 FujiNet, 
  // The ROM boot won't happen, so just do a cold start to boot the system normally.
#ifndef DRAGON
  coldStart();
#endif
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

