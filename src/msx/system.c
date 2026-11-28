#include <conio.h>
#include <string.h>
#include "../system.h"
#include "../globals.h"

uint8_t system_create_type;
/* Big enough to double as system_create_new()'s NewDisk; nothing else is using it then */
char response[sizeof(NewDisk)];

void system_boot(void)
{
  fuji_enable_user_rom();
}

bool system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                       uint32_t selected_size, const char *path)
{
  NewDisk *nd = (NewDisk *)response;

  memset(nd, 0, sizeof(NewDisk));
  nd->createType = system_create_type;
  nd->deviceSlot = selected_device_slot;
  nd->hostSlot = selected_host_slot;
  // selected_size is in K; FujiNet counts 512-byte sectors
  nd->numBlocks = (uint16_t)selected_size * 2;
  strcpy(nd->filename, path);
  return fuji_create_new(nd);
}
