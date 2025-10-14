/**
 * CONFIG custom types
 */

#ifndef FUJI_TYPEDEFS_H
#define FUJI_TYPEDEFS_H

#ifdef USING_FUJINET_LIB
#include "fujinet-fuji.h"
#else
// Source the types in fujinet-fuji.h for platforms that are not yet using fujinet-lib
#include "fuji_typedefs_io.h"

extern bool fuji_get_scan_result(uint8_t n, SSIDInfo *ssid_info);
extern bool fuji_get_adapter_config_extended(AdapterConfigExtended *ac);
extern bool fuji_scan_for_networks(uint8_t *count);
extern bool fuji_error(void);
extern bool fuji_set_ssid(NetConfig *nc);
extern bool fuji_close_directory(void);
extern bool fuji_mount_host_slot(uint8_t hs);
extern bool fuji_open_directory2(uint8_t hs, char *path, char *filter);
extern bool fuji_set_directory_position(uint16_t pos);
extern bool fuji_read_directory(uint8_t maxlen, uint8_t aux2, char *buffer);
extern bool fuji_put_host_slots(HostSlot *h, size_t size);
extern bool fuji_set_boot_config(uint8_t toggle);
extern bool fuji_get_wifi_enabled(void);
extern bool fuji_get_wifi_status(uint8_t *status);
extern bool fuji_get_ssid(NetConfig *net_config);
extern bool fuji_get_device_filename(uint8_t ds, char *buffer);
extern bool fuji_unmount_disk_image(uint8_t ds);
extern bool fuji_put_device_slots(DeviceSlot *d, size_t size);
extern bool fuji_get_device_slots(DeviceSlot *d, size_t size);
extern bool fuji_set_device_filename(uint8_t mode, uint8_t hs, uint8_t ds, char *buffer);
extern bool fuji_disable_device(uint8_t d);
extern bool fuji_enable_device(uint8_t d);
extern bool fuji_mount_disk_image(uint8_t ds, uint8_t mode);
extern bool fuji_get_host_slots(HostSlot *h, size_t size);
extern bool fuji_copy_file(uint8_t src_slot, uint8_t dst_slot, char *copy_spec);
extern bool fuji_create_new(NewDisk *new_disk);

#ifdef BUILD_ADAM
#define FUJI_DEV 0x0F
#endif /* BUILD_ADAM */

#endif

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

// Other than Adam, all implementations are hard coded to return true or false.
#define fuji_get_device_enabled_status(ds) true
// Just calls the above for count devices
#define fuji_update_devices_enabled(de, count)

#define fuji_device_slot_to_device(ds) (ds + FUJI_SLOT_TO_DEV_OFFSET)

#ifndef fuji_open_directory2
#define fuji_open_directory_filter(hs, path, filter) fuji_open_directory2(hs, path, filter)
//#define fuji_mount_all() (void)
#endif /* fuji_open_directory2 */

#if defined(BUILD_ADAM) || defined(BUILD_RC2014)
#define FUJI_SLOT_TO_DEV_OFFSET 4
#else /* !BUILD_ADAM && !BUILD_RC2014 */
#define FUJI_SLOT_TO_DEV_OFFSET 4
#endif /* BUILD_ADAM || BUILD_RC2014 */

#endif /* FUJI_TYPEDEFS_H */
