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

#define LFN 15
#define DEV 15
#define SAN 15

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
  unsigned char ws=0xFF;
  
  cbm_open(LFN,DEV,SAN,"\xFA"); // FUJICMD_GET_WIFI_STATUS
  cbm_read(LFN,&ws,sizeof(ws));
  cbm_close(LFN);
  return ws;
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
