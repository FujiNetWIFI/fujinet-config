/**
 * CONFIG custom types
 */

#ifndef FUJI_TYPEDEFS_H
#define FUJI_TYPEDEFS_H

#include "fujinet-fuji.h"

#define MODE_READ 1
#define MODE_WRITE 2
#define MODE_MOUNTED 0x40

#define MAX_HOST_LEN 32
#define NUM_HOST_SLOTS 8

typedef enum _entry_types
{
  ENTRY_TYPE_TEXT,
  ENTRY_TYPE_FOLDER,
  ENTRY_TYPE_FILE,
  ENTRY_TYPE_LINK,
  ENTRY_TYPE_MENU
} EntryType;

typedef unsigned short DirectoryPosition;

// Not read from FujiNet yet; every device reports enabled.
#define fuji_get_device_enabled_status(ds) true
#define fuji_update_devices_enabled(de, count)

#define fuji_device_slot_to_device(ds) (ds + FUJI_SLOT_TO_DEV_OFFSET)
#define fuji_open_directory_filter(hs, path, filter) fuji_open_directory2(hs, path, filter)

#ifdef BUILD_ADAM
/* fujinet-lib 5.0.0 sends an aux byte ahead of the NetConfig, but the Adam
   firmware reads the SSID right after the command. Fixed in lib e125476 (#216). */
#undef fuji_set_ssid
#define fuji_set_ssid(nc) FUJICALL_D(FUJICMD_SET_SSID, nc, sizeof(NetConfig))

/* The firmware's WRITE DEVICE SLOTS takes all 8 slots and saves them, garbage
   past what was sent included. The Adam uses only 4, so send those plus 4 empty.
   Moves into the lib with fujinet-lib #242. */
#undef fuji_put_device_slots
#define fuji_put_device_slots(d, count) adam_put_device_slots(d, count)
bool adam_put_device_slots(DeviceSlot *d, size_t count);
#endif /* BUILD_ADAM */

#if defined(BUILD_ADAM) || defined(BUILD_RC2014)
#define FUJI_SLOT_TO_DEV_OFFSET 4
#else /* !BUILD_ADAM && !BUILD_RC2014 */
#define FUJI_SLOT_TO_DEV_OFFSET 4
#endif /* BUILD_ADAM || BUILD_RC2014 */

#endif /* FUJI_TYPEDEFS_H */
