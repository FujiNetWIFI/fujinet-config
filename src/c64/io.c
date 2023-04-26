#ifdef BUILD_C64
/**
 * FujiNet CONFIG for #C64
 *
 * I/O Routines
 */
#include "io.h"
#include <stdint.h>
#include <conio.h>
#include <stdlib.h>
#include <peekpoke.h> // For the insanity in io_boot()
#include "globals.h"

#define FUJICMD_RESET 0xFF
#define FUJICMD_GET_SSID 0xFE
#define FUJICMD_SCAN_NETWORKS 0xFD
#define FUJICMD_GET_SCAN_RESULT 0xFC
#define FUJICMD_SET_SSID 0xFB
#define FUJICMD_GET_WIFISTATUS 0xFA
#define FUJICMD_MOUNT_HOST 0xF9
#define FUJICMD_MOUNT_IMAGE 0xF8
#define FUJICMD_OPEN_DIRECTORY 0xF7
#define FUJICMD_READ_DIR_ENTRY 0xF6
#define FUJICMD_CLOSE_DIRECTORY 0xF5
#define FUJICMD_READ_HOST_SLOTS 0xF4
#define FUJICMD_WRITE_HOST_SLOTS 0xF3
#define FUJICMD_READ_DEVICE_SLOTS 0xF2
#define FUJICMD_WRITE_DEVICE_SLOTS 0xF1
#define FUJICMD_UNMOUNT_IMAGE 0xE9
#define FUJICMD_GET_ADAPTERCONFIG 0xE8
#define FUJICMD_NEW_DISK 0xE7
#define FUJICMD_UNMOUNT_HOST 0xE6
#define FUJICMD_GET_DIRECTORY_POSITION 0xE5
#define FUJICMD_SET_DIRECTORY_POSITION 0xE4
//#define FUJICMD_SET_HSIO_INDEX 0xE3
#define FUJICMD_SET_DEVICE_FULLPATH 0xE2
#define FUJICMD_SET_HOST_PREFIX 0xE1
#define FUJICMD_GET_HOST_PREFIX 0xE0
//#define FUJICMD_SET_SIO_EXTERNAL_CLOCK 0xDF
#define FUJICMD_WRITE_APPKEY 0xDE
#define FUJICMD_READ_APPKEY 0xDD
#define FUJICMD_OPEN_APPKEY 0xDC
#define FUJICMD_CLOSE_APPKEY 0xDB
#define FUJICMD_GET_DEVICE_FULLPATH 0xDA
#define FUJICMD_CONFIG_BOOT 0xD9
#define FUJICMD_COPY_FILE 0xD8
#define FUJICMD_MOUNT_ALL 0xD7
#define FUJICMD_SET_BOOT_MODE 0xD6
#define FUJICMD_ENABLE_DEVICE 0xD5
#define FUJICMD_DISABLE_DEVICE 0xD4
#define FUJICMD_DEVICE_STATUS 0xD1
#define FUJICMD_STATUS 0x53

#define UNUSED(x) (void)(x);

#include <string.h>

static NetConfig nc;
static SSIDInfo ssid_response;
static AdapterConfig ac;

unsigned char io_create_type;

void io_init(void)
{
}

uint8_t io_status(void)
{
  return io_error();
}

bool io_error(void)
{
  return 0; 
}

uint8_t io_get_wifi_status(void)
{
  return 0;
}

NetConfig* io_get_ssid(void)
{  
  memset(&nc, 0, sizeof(nc));
  return &nc;
}

uint8_t io_scan_for_networks(void)
{
  return 0;
}

SSIDInfo *io_get_scan_result(uint8_t n)
{
  return &ssid_response;
}

AdapterConfig *io_get_adapter_config(void)
{
  return &ac;
}

void io_set_ssid(NetConfig *nc)
{
}

char *io_get_device_filename(uint8_t ds)
{
  return (char *)0;
}

void io_create_new(uint8_t selected_host_slot,uint8_t selected_device_slot,unsigned long selected_size,char *path)
{
}

void io_get_device_slots(DeviceSlot *d)
{
}

void io_get_host_slots(HostSlot *h)
{
}

void io_put_host_slots(HostSlot *h)
{
}

void io_put_device_slots(DeviceSlot *d)
{
}

void io_mount_host_slot(uint8_t hs)
{
}

void io_open_directory(uint8_t hs, char *p, char *f)
{
}

char *io_read_directory(uint8_t l, uint8_t a)
{
  return (char *)0;
}

void io_close_directory(void)
{
}

void io_set_directory_position(DirectoryPosition pos)
{
}

void io_set_device_filename(uint8_t ds, char* e)
{
}

void io_mount_disk_image(uint8_t ds, uint8_t mode)
{
}

void io_copy_file(unsigned char source_slot, unsigned char destination_slot)
{
}

void io_set_boot_config(uint8_t toggle)
{
}

void io_umount_disk_image(uint8_t ds)
{
}

void io_update_devices_enabled(bool *e)
{
}

void io_enable_device(unsigned char d)
{
}

void io_disable_device(unsigned char d)
{
}

bool io_get_device_enabled_status(unsigned char d)
{
  return false;
}

unsigned char io_device_slot_to_device(unsigned char ds)
{
  return ds;
}

void io_boot(void)
{
}

bool io_get_wifi_enabled(void)
{
  return true;
}

#endif /* BUILD_C64 */
