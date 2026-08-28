/**
 * MSX device slot layout
 */

#include <string.h>
#include "device_slots.h"
#include "../constants.h"
#include "../globals.h"

static bool mount_is_rom = false;

bool msx_device_slot_is_rom(const char *filename)
{
  return strstr(filename, ".rom") != NULL || strstr(filename, ".ROM") != NULL
      || strstr(filename, ".bin") != NULL || strstr(filename, ".BIN") != NULL;
}

uint8_t msx_visible_device_slots(void)
{
  uint8_t roms = 0;

  for (uint8_t i = 0; i < MSX_MAX_SLOTS && roms < MSX_MAX_ROM_SLOTS; i++)
    if (msx_device_slot_is_rom((const char *)deviceSlots[i].file))
      roms++;

  return MSX_NUM_DISK_SLOTS + roms;
}

void msx_compact_device_slots(uint8_t ds)
{
  char filename[256];
  uint8_t i, m;

  // Work from a fresh copy: ds has already been unmounted by the caller.
  fuji_get_device_slots(deviceSlots, NUM_DEVICE_SLOTS);

  for (i = ds; i < MSX_MAX_SLOTS - 1; i++)
  {
    if (deviceSlots[i + 1].file[0] == 0x00)
    {
      // Pulling an empty slot up just empties this one.
      fuji_unmount_disk_image(i);
      continue;
    }

    // The slot table only holds a truncated name, so ask for the real one.
    memset(filename, 0, sizeof(filename));
    if (!fuji_get_device_filename(i + 1, filename))
      return; // leave the rest in place rather than dropping the tail slot

    m = deviceSlots[i + 1].mode & (MODE_READ | MODE_WRITE);
    fuji_set_device_filename(m ? m : MODE_READ, deviceSlots[i + 1].hostSlot, i, filename);
  }

  // Nothing left to pull into the last slot.
  fuji_unmount_disk_image(MSX_MAX_SLOTS - 1);
}

void msx_set_mount_is_rom(bool is_rom)
{
  mount_is_rom = is_rom;
}

uint8_t msx_mount_target_slots(void)
{
  uint8_t n = msx_visible_device_slots();

  // Only a ROM grows the list, and only while there is room for another.
  if (!mount_is_rom || n >= MSX_MAX_SLOTS)
    return n;

  for (uint8_t i = 0; i < n; i++)
    if (deviceSlots[i].file[0] == 0x00)
      return n; // an empty row is already there to drop it into

  return n + 1;
}

bool msx_mount_replaces_rom(void)
{
  // A full list means both ROM slots are taken, so a third ROM has to
  // take the place of one that is already there.
  return mount_is_rom && msx_visible_device_slots() >= MSX_MAX_SLOTS;
}

bool msx_mount_slot_allowed(uint8_t ds)
{
  if (ds >= msx_mount_target_slots())
    return false;

  if (!msx_mount_replaces_rom())
    return true;

  return msx_device_slot_is_rom((const char *)deviceSlots[ds].file);
}

uint8_t msx_first_mount_slot(void)
{
  uint8_t n = msx_mount_target_slots();

  for (uint8_t i = 0; i < n; i++)
    if (msx_mount_slot_allowed(i))
      return i;

  return 0;
}

uint8_t msx_next_mount_slot(uint8_t ds, bool down)
{
  uint8_t n = msx_mount_target_slots();

  // Step over any slot the image cannot go into, and stay put if there is
  // nothing selectable left in that direction.
  while (down ? (ds + 1 < n) : (ds > 0))
  {
    ds = down ? ds + 1 : ds - 1;
    if (msx_mount_slot_allowed(ds))
      return ds;
  }

  return MSX_NO_SLOT;
}
