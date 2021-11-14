#ifdef BUILD_APPLE2
/**
 * FujiNet CONFIG for #Apple2
 *
 * I/O Routines
 */

#ifndef IO_H
#define IO_H

#include <stdbool.h>
#include "fuji_typedefs.h"

bool io_error(void);
unsigned char io_status(void);
void io_init(void);
unsigned char io_get_wifi_status(void);
NetConfig* io_get_ssid(void);
unsigned char io_scan_for_networks(void);
SSIDInfo *io_get_scan_result(unsigned char n);
AdapterConfig *io_get_adapter_config(void);
void io_set_ssid(NetConfig *nc);
void io_get_device_slots(DeviceSlot *d);
void io_get_host_slots(HostSlot *h);
void io_put_host_slots(HostSlot *h);
void io_put_device_slots(DeviceSlot *d);
void io_mount_host_slot(unsigned char hs);
void io_open_directory(unsigned char hs, char *p, char *f);
char *io_read_directory(unsigned char l, unsigned char a);
void io_close_directory(void);
void io_set_directory_position(DirectoryPosition pos);
void io_set_device_filename(unsigned char ds, char* e);
void io_mount_disk_image(unsigned char ds, unsigned char mode);
void io_set_boot_config(unsigned char toggle);
void io_umount_disk_image(unsigned char ds);
void io_boot(void);

#endif /* IO_H */
#endif /* BUILD_APPLE2 */
