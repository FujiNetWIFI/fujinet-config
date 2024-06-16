#ifdef BUILD_C64
/**
 * FujiNet CONFIG for #C64
 *
 * I/O Routines
 */

#ifndef IO_H
#define IO_H

#include <stdbool.h>
#include "../fuji_typedefs.h"

bool io_error(void);
unsigned char io_status(void);
void io_init(void);
unsigned char io_get_wifi_status(void);
NetConfig *io_get_ssid(void);
unsigned char io_scan_for_networks(void);
SSIDInfo *io_get_scan_result(unsigned char n);
AdapterConfigExtended *io_get_adapter_config(void);
int io_set_ssid(NetConfig *nc);
char *io_get_device_filename(unsigned char ds);
void io_create_new(unsigned char selected_host_slot, unsigned char selected_device_slot, unsigned long selected_size, char *path);
bool io_get_device_slots(DeviceSlot *d);
bool io_get_host_slots(HostSlot *h);
void io_put_host_slots(HostSlot *h);
void io_put_device_slots(DeviceSlot *d);
uint8_t io_mount_host_slot(unsigned char hs);
void io_open_directory(unsigned char hs, char *p, char *f);
char *io_read_directory(unsigned char l, unsigned char a);
void io_close_directory(void);
void io_set_directory_position(DirectoryPosition pos);
void io_set_device_filename(unsigned char ds, unsigned char hs, unsigned char mode, char *e);
uint8_t io_mount_disk_image(unsigned char ds, unsigned char mode);
void io_copy_file(unsigned char source_slot, unsigned char destination_slot);
void io_set_boot_config(unsigned char toggle);
void io_set_boot_mode(unsigned char mode);
void io_umount_disk_image(unsigned char ds);
void io_boot(void);
bool io_get_device_enabled_status(unsigned char d);
void io_update_devices_enabled(bool *e);
void io_enable_device(unsigned char d);
void io_disable_device(unsigned char d);
unsigned char io_device_slot_to_device(unsigned char ds);
void io_get_filename_for_device_slot(unsigned char slot, const char *filename);
bool io_mount_all(void);

bool io_get_wifi_enabled(void);

#endif /* IO_H */
#endif /* BUILD_C64 */
