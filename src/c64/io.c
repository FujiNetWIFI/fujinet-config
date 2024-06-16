#ifdef BUILD_C64
/**
 * FujiNet CONFIG for #C64
 *
 * I/O Routines
 */
#include "io.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <peekpoke.h> // For the insanity in io_boot()
#include <c64.h>
#include <cbm.h>
#include "globals.h"

#define UNUSED(x) (void)(x);

// #define FUJICMD_RESET "\xFF"
// #define FUJICMD_GET_SSID "\xFE"
// #define FUJICMD_SCAN_NETWORKS "\xFD"
// #define FUJICMD_GET_SCAN_RESULT "\xFC"
// #define FUJICMD_SET_SSID "\xFB"
// #define FUJICMD_GET_WIFISTATUS "\xFA"
// #define FUJICMD_MOUNT_HOST "\xF9"
// #define FUJICMD_MOUNT_IMAGE "\xF8"
// #define FUJICMD_OPEN_DIRECTORY "\xF7"
// #define FUJICMD_READ_DIR_ENTRY "\xF6"
// #define FUJICMD_CLOSE_DIRECTORY "\xF5"
// #define FUJICMD_READ_HOST_SLOTS "\xF4"
// #define FUJICMD_WRITE_HOST_SLOTS "\xF3"
// #define FUJICMD_READ_DEVICE_SLOTS "\xF2"
// #define FUJICMD_WRITE_DEVICE_SLOTS "\xF1"
// #define FUJICMD_UNMOUNT_IMAGE "\xE9"
// #define FUJICMD_GET_ADAPTERCONFIG "\xE8"
// #define FUJICMD_NEW_DISK "\xE7"
// #define FUJICMD_UNMOUNT_HOST "\xE6"
// #define FUJICMD_GET_DIRECTORY_POSITION "\xE5"
// #define FUJICMD_SET_DIRECTORY_POSITION "\xE4"
// #define FUJICMD_SET_DEVICE_FULLPATH "\xE2"
// #define FUJICMD_SET_HOST_PREFIX "\xE1"
// #define FUJICMD_GET_HOST_PREFIX "\xE0"
// #define FUJICMD_WRITE_APPKEY "\xDE"
// #define FUJICMD_READ_APPKEY "\xDD"
// #define FUJICMD_OPEN_APPKEY "\xDC"
// #define FUJICMD_CLOSE_APPKEY "\xDB"
// #define FUJICMD_GET_DEVICE_FULLPATH "\xDA"
// #define FUJICMD_CONFIG_BOOT "\xD9"
// #define FUJICMD_COPY_FILE "\xD8"
// #define FUJICMD_MOUNT_ALL "\xD7"
// #define FUJICMD_SET_BOOT_MODE "\xD6"
// #define FUJICMD_ENABLE_DEVICE "\xD5"
// #define FUJICMD_DISABLE_DEVICE "\xD4"
// #define FUJICMD_DEVICE_STATUS "\xD1"
// #define FUJICMD_STATUS "\x53"

#define LFN 15
#define DEV 15
#define SAN 15

#include <string.h>

static NetConfig nc;
static AdapterConfigExtended adapterConfig;
static SSIDInfo ssidInfo;
unsigned char wifiEnabled = true;

char response[256];

unsigned char io_create_type;

void io_init(void) {}

uint8_t io_status(void)
{
	return 0;
}

bool io_error(void)
{
	return fuji_error();
}

uint8_t io_get_wifi_status(void)
{
	uint8_t status;
	fuji_get_wifi_status(&status);
	return status;
}

NetConfig *io_get_ssid(void)
{
	fuji_get_ssid(&nc);
	return &nc;
}

uint8_t io_scan_for_networks(void)
{
	uint8_t count;
	fuji_scan_for_networks(&count);
	return count;
}

SSIDInfo *io_get_scan_result(uint8_t n)
{
	fuji_get_scan_result(n, &ssidInfo);
	return &ssidInfo;
}

AdapterConfigExtended *io_get_adapter_config(void)
{
	fuji_get_adapter_config_extended(&adapterConfig);
	return &adapterConfig;
}

int io_set_ssid(NetConfig *nc)
{
	bool is_err;

	fuji_set_ssid(nc);
	is_err = fuji_error();
	io_get_wifi_status();
	return is_err;
}

char *io_get_device_filename(uint8_t slot)
{
	fuji_get_device_filename(slot, &response);
	return &response;
}

void io_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot, unsigned long selected_size, char *path)
{
	// TODO implement
}

bool io_get_device_slots(DeviceSlot *d)
{
	return fuji_get_device_slots(d, 8);
}

bool io_get_host_slots(HostSlot *h)
{
	return fuji_get_host_slots(h, 8);
}

void io_put_host_slots(HostSlot *h)
{
	fuji_put_host_slots(h, 8);
}

void io_put_device_slots(DeviceSlot *d)
{
	fuji_put_device_slots(d, 8);
}

uint8_t io_mount_host_slot(uint8_t hs)
{
	if (hostSlots[hs][0] == 0)
		return 2;
	fuji_mount_host_slot(hs);
	return 0;
}

void io_open_directory(uint8_t hs, char *p, char *f)
{
	fuji_open_directory2(hs, p, f);
}

char *io_read_directory(uint8_t maxlen, uint8_t a)
{
	memset(response, 0, maxlen);
	fuji_read_directory(maxlen, a, &response);
	return &response;
}

void io_close_directory(void)
{
	fuji_close_directory();
}

void io_set_directory_position(DirectoryPosition pos)
{
	fuji_set_directory_position(pos);
}

void io_set_device_filename(unsigned char ds, unsigned char hs, unsigned char mode, char *e)
{
	fuji_set_device_filename(mode, hs, ds, e);
}

void io_copy_file(unsigned char source_slot, unsigned char destination_slot)
{
	// incrementing is handled in function, we keep everything 0 based
	fuji_copy_file(source_slot, destination_slot, &copySpec);
}

void io_set_boot_config(uint8_t toggle)
{
	fuji_set_boot_config(toggle);
}

void io_set_boot_mode(unsigned char mode)
{
	fuji_set_boot_mode(mode);
}

uint8_t io_mount_disk_image(uint8_t ds, uint8_t mode)
{
	return fuji_mount_disk_image(ds, mode);
}

void io_umount_disk_image(uint8_t ds)
{
	fuji_unmount_disk_image(ds);
}

void io_update_devices_enabled(bool *e) {}

void io_enable_device(unsigned char d) {}

void io_disable_device(unsigned char d) {}

bool io_get_device_enabled_status(unsigned char d)
{
	return false;
}

unsigned char io_device_slot_to_device(unsigned char ds)
{
	return ds;
}

void io_boot(void) {}

bool io_get_wifi_enabled(void)
{
	wifiEnabled = fuji_get_wifi_enabled();
	return wifiEnabled == 1;
}

void io_get_filename_for_device_slot(unsigned char slot, const char *filename)
{
	fuji_get_device_filename(slot, filename);
}

bool io_mount_all(void)
{
	return fuji_mount_all();
	// return OS.dcb.dstats; // 1 = successful, anything else = error.
}

#endif /* BUILD_C64 */
