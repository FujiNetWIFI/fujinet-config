#if defined(BUILD_APPLE2) || defined(BUILD_ATARI)
#include "typedefs.h"
#include "globals.h"
#include <string.h>

#warning "FIXME - this should be part of fujinet-lib"

bool fuji_open_directory2(uint8_t hs, char *p, char *f)
{
  char *_p = p;
  if (f[0] != 0x00)
  {
    // We have a filter, create a directory+filter string
    memset(response, 0, 256);
    strcpy(response, p);
    strcpy(&response[strlen(response) + 1], f);
    _p = &response[0];
  }
  return fuji_open_directory(hs, _p);
}
#endif /* BUILD_APPLE2 || BUILD_ATARI */

#ifdef BUILD_ADAM
#include "typedefs.h"
#include <string.h>

#define FIRMWARE_DEVICE_SLOTS 8

bool adam_put_device_slots(DeviceSlot *d, size_t count)
{
  static DeviceSlot slots[FIRMWARE_DEVICE_SLOTS];

  memset(slots, 0, sizeof(slots));
  memcpy(slots, d, count * sizeof(DeviceSlot));
  return FUJICALL_D(FUJICMD_WRITE_DEVICE_SLOTS, slots, sizeof(slots));
}
#endif /* BUILD_ADAM */
