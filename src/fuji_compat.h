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

//#warning "FIXME - needs to be implemented in fujinet-lib and firmware."

// No platform queries the enabled state yet.
#define fuji_get_device_enabled_status(ds) true
#define fuji_update_devices_enabled(de, count)

#define fuji_device_slot_to_device(ds) (ds + FUJI_SLOT_TO_DEV_OFFSET)
#define fuji_open_directory_filter(hs, path, filter) fuji_open_directory2(hs, path, filter)

#if defined(BUILD_ADAM) || defined(BUILD_RC2014)
#define FUJI_SLOT_TO_DEV_OFFSET 4
#else /* !BUILD_ADAM && !BUILD_RC2014 */
#define FUJI_SLOT_TO_DEV_OFFSET 4
#endif /* BUILD_ADAM || BUILD_RC2014 */

#endif /* FUJI_TYPEDEFS_H */
