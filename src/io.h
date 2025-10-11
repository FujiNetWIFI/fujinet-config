/**
 * I/O Routines
 */

#ifndef IO_H
#define IO_H

#include "typedefs.h"

bool io_error(void);
uint8_t io_status(void);
void io_init(void);
uint8_t io_get_wifi_status(void);
NetConfig* io_get_ssid(void);
uint8_t io_scan_for_networks(void);
SSIDInfo *io_get_scan_result(uint_fast8_t n);
//AdapterConfig *io_get_adapter_config(void);
AdapterConfigExtended *io_get_adapter_config_extended(void);
int io_set_ssid(NetConfig *nc);
const char *io_get_device_filename(uint8_t ds);
void io_create_new(uint8_t selected_host_slot,uint8_t selected_device_slot,uint32_t selected_size,char *path);
bool io_get_device_slots(DeviceSlot *d);
bool io_get_host_slots(HostSlot *h);
void io_put_host_slots(HostSlot *h);
void io_put_device_slots(DeviceSlot *d);
uint8_t io_mount_host_slot(uint8_t hs);
void io_open_directory(uint8_t hs, char *p, char *f);
const char *io_read_directory(uint8_t l, uint8_t a);
void io_close_directory(void);
void io_set_directory_position(DirectoryPosition pos);
bool io_mount_disk_image(uint8_t ds, uint8_t mode);
void io_copy_file(uint8_t source_slot, uint8_t destination_slot);
void io_set_boot_config(uint8_t toggle);
void io_set_boot_mode(uint8_t mode);
void io_umount_disk_image(uint8_t ds);
bool io_get_device_enabled_status(uint8_t d);
void io_update_devices_enabled(bool *e);
void io_enable_device(uint8_t d);
void io_disable_device(uint8_t d);
uint8_t io_device_slot_to_device(uint8_t ds);
void io_boot(void);
bool io_get_wifi_enabled(void);
void io_list_devs(void);
void io_build_directory(uint8_t ds, uint32_t numBlocks, char *v);
bool io_mount_all(void);

#if defined(BUILD_ATARI) || defined(BUILD_APPLE2) || defined(__CBM__)
void io_set_device_filename(uint8_t ds, uint8_t hs, uint8_t mode, char* e);
#else
//#warning "Why is io_set_device_filename() different on this platform?"
void io_set_device_filename(uint8_t ds, char* e);
#endif

#ifdef BUILD_ATARI
void rtclr();
void cold_start();
void io_get_filename_for_device_slot(unsigned char slot, const char* filename);
#endif /* BUILD_ATARI */

#endif /* IO_H */
