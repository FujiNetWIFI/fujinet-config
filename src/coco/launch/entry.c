#include "entry.h"

void entry_set(unsigned char entry, unsigned char host_slot, unsigned char device_slot)
{
  ENTRY_DATA->magic[0] = 'F';
  ENTRY_DATA->magic[1] = 'J';
  ENTRY_DATA->entry = entry;
  ENTRY_DATA->host_slot = host_slot;
  ENTRY_DATA->device_slot = device_slot;
}

unsigned char entry_take(void)
{
  unsigned char e = ENTRY_NONE;

  if (ENTRY_DATA->magic[0] == 'F' && ENTRY_DATA->magic[1] == 'J')
    e = ENTRY_DATA->entry;
  ENTRY_DATA->magic[0] = 0;
  return e;
}
