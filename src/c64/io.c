#ifdef BUILD_C64
/**
 * FujiNet CONFIG for #Apple2
 *
 * I/O Routines
 */

#include <string.h>
#include "io.h"

/* Test Fixtures, remove when actual I/O present */
static NetConfig nc;
static SSIDInfo ssid_response;
static AdapterConfig ac;
static DeviceSlot _ds[8];
static HostSlot _hs[8];
static char de[36][8]={
  {"Entry 1"},
  {"Entry 2"},
  {"Entry 3"},
  {"Entry 4"},
  {"Entry 5"},
  {"Entry 6"},
  {"Entry 7"},
  {"\x7F"}
};
static char dc=0;

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
  return 3;
}

NetConfig* io_get_ssid(void)
{
  return &nc;
}

unsigned char io_scan_for_networks(void)
{
  return 1;
}

SSIDInfo *io_get_scan_result(unsigned char n)
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

void io_get_device_slots(DeviceSlot *d)
{
  d=_ds;
}

void io_get_host_slots(HostSlot *h)
{
  h=_hs;
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
  dc=0;
}

char *io_read_directory(unsigned char l, unsigned char a)
{
  return de[dc++];
}

void io_close_directory(void)
{
  dc=0;
}

void io_set_directory_position(DirectoryPosition pos)
{
  dc=(char)pos;
}

void io_set_device_filename(unsigned char ds, char* e)
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

bool io_get_wifi_enabled(void)
{
	return true;
}
#endif /* BUILD_C64 */
