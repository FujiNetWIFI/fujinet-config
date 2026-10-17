#include "../system.h"
#include "../pause.h"
#include "../globals.h"
#include "strendswith.h"

#define FUJI_IO_REG ((unsigned char *)0xFF41)

/* Big enough to double as system_create_new()'s NewDisk; nothing else is using it then */
char response[sizeof(NewDisk)];

void system_enable_user_rom(void)
{
  *FUJI_IO_REG = 7;
}

bool system_slot0_is_rom(void)
{
  char *name;

  if (!fuji_get_device_filename(0, response))
    return false;
  name = strupr(response);
  return strendswith(name, ".CCC") || strendswith(name, ".ROM");
}

void system_boot(void)
{
  static const char msg[] = "\rBOOTING USER ROM...\r";

  if (system_slot0_is_rom()) {
    // putstr, not printf: keeps the printf library out of apps that never print.
    putstr(msg, sizeof(msg) - 1);
    pause(120);                 
    system_enable_user_rom();   // write 7 (ROM_MODE | USERROM_ENABLE | AUTOSTART_ENABLE) to IO_CONTROL. Firmware arms the CART trigger and triggers RESET.
  }
  else
  {
    pause(120);
  }

  // On a Rev0000 FujiNet the ROM boot never happens, so cold start to boot normally.
#ifndef DRAGON
  coldStart();
#endif
  exit(0);
}

void system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                       uint32_t selected_size, const char *path)
{
  NewDisk *nd = (NewDisk *)response;

  memset(nd, 0, sizeof(NewDisk));
  nd->numDisks = (byte)selected_size;
  nd->hostSlot = selected_host_slot;
  nd->deviceSlot = selected_device_slot;
  strcpy(nd->filename, path);

  fuji_create_new(nd);
  return;
}

