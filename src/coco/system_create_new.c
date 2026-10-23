#include "../system.h"
#include "../globals.h"

/* Big enough to double as system_slot0_is_rom()'s (coco/system.c) filename
   buffer; nothing else is using it then */
char response[sizeof(NewDisk)];

bool system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                       uint32_t selected_size, const char *path)
{
  NewDisk *nd = (NewDisk *)response;

  memset(nd, 0, sizeof(NewDisk));
  nd->numDisks = (byte)selected_size;
  nd->hostSlot = selected_host_slot;
  nd->deviceSlot = selected_device_slot;
  strcpy(nd->filename, path);

  return fuji_create_new(nd);
}
