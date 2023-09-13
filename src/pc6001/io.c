#ifdef BUILD_PC6001
/**
 * FujiNet CONFIG for #Adam
 *
 * I/O Routines
 */

#include <conio.h> // for sleep() 
#include <stdlib.h>
#include <string.h>
#include "io.h"

static void io_command_and_response(void* buf, unsigned short len)
{
}

void io_init(void)
{
}

unsigned char io_status(void)
{
  return 0;
}

bool io_error(void)
{
  return false;
}

unsigned char io_get_wifi_status(void)
{
  return 0;
}

NetConfig* io_get_ssid(void)
{
  return NULL;
}

unsigned char io_scan_for_networks(void)
{
  return 0;
}

SSIDInfo *io_get_scan_result(unsigned char n)
{
  return NULL;
}

AdapterConfig *io_get_adapter_config(void)
{
  return NULL;
}

int io_set_ssid(NetConfig *nc)
{
  return 0;
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

void io_mount_host_slot(unsigned char hs)
{
}

void io_open_directory(unsigned char hs, char *p, char *f)
{
}

char *io_read_directory(unsigned char l, unsigned char a)
{
}

void io_close_directory(void)
{
}

void io_set_directory_position(DirectoryPosition pos)
{
}

void io_set_device_filename(unsigned char ds, char* e)
{
}

char *io_get_device_filename(unsigned char ds)
{
}

void io_create_new(unsigned char selected_host_slot,unsigned char selected_device_slot,unsigned long selected_size,char *path)
{
}

void io_mount_disk_image(unsigned char ds, unsigned char mode)
{
}

void io_set_boot_config(unsigned char toggle)
{
}

void io_umount_disk_image(unsigned char ds)
{
}

void io_boot(void)
{
}

void io_build_directory(unsigned char ds, unsigned long numBlocks, char *v)
{
}

bool io_get_wifi_enabled(void)
{
	return true;
}

#endif /* BUILD_PC6001 */
