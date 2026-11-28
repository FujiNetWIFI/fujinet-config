#include <conio.h>
#include "../system.h"
#include "../globals.h"
#include "device_slots.h"

// Cart mode bits for the expanded slot, not in a fujinet-lib release yet:
// subslot 0 is CONFIG or user ROM A, 1 UNAPI, 2 Nextor, 3 user ROM B.
#ifndef FUJI_CART_MODE_NEXTOR
#define FUJI_CART_MODE_POWER_ON   0x3C
#define FUJI_CART_MODE_USER_ROM_A 0x01
#define FUJI_CART_MODE_NEXTOR     0x02
#define FUJI_CART_MODE_USER_ROM_B 0x40
#endif

char response[256];

// The FujiNet sends the first ROM to user ROM A and the second to B. Nextor
// comes up for disks, or for nothing at all; ROMs alone don't need it.
void system_boot(void)
{
  uint8_t i, roms = 0, disks = 0;
  uint8_t mode = FUJI_CART_MODE_POWER_ON;

  for (i = 0; i < MSX_MAX_SLOTS; i++)
  {
    const char *file = (const char *)deviceSlots[i].file;

    if (!file[0])
      continue;
    if (msx_device_slot_is_rom(file))
      roms++;
    else
      disks++;
  }

  if (roms)
    mode |= FUJI_CART_MODE_USER_ROM_A;
  if (roms > 1)
    mode |= FUJI_CART_MODE_USER_ROM_B;
  if (disks || !roms)
    mode |= FUJI_CART_MODE_NEXTOR;

  fuji_set_cart_mode(mode);
}

bool system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                       uint32_t selected_size, const char *path)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#endif
    return false;
}
