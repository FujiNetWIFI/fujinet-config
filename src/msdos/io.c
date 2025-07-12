#ifdef __WATCOMC__
/**
 * FujiNet SIO calls
 */

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <dos.h> // for int86/x()
#include "io.h"
#include "globals.h"
#include "screen.h"
#include "bar.h"

static NetConfig nc;
static AdapterConfigExtended adapterConfig;
static SSIDInfo ssidInfo;
NewDisk newDisk;
unsigned char wifiEnabled=true;

// Register pack for INT F5 calls
union REGS r;

// variable to hold various responses that we just need to return a char*.
char response[256];

bool io_error(void)
{
  return 0;
}

unsigned char io_status(void)
{
  return 0;
}

void io_init(void)
{
}

bool io_get_wifi_enabled(void)
{
  return false;
}

unsigned char io_get_wifi_status(void)
{
  return 0;
}

NetConfig *io_get_ssid(void)
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

AdapterConfigExtended *io_get_adapter_config(void)
{
  return NULL;
}

int io_set_ssid(NetConfig *nc)
{
  return 0;
}

bool io_get_device_slots(DeviceSlot *d)
{
  return false;
}

bool io_get_host_slots(HostSlot *h)
{
  return false;
}

void io_put_host_slots(HostSlot *h)
{
}

void io_put_device_slots(DeviceSlot *d)
{
}

uint8_t io_mount_host_slot(unsigned char hs)
{
  return 0;
}

void io_open_directory(unsigned char hs, char *p, char *f)
{
}

char *io_read_directory(unsigned char maxlen, unsigned char a)
{
  return "";
}

void io_close_directory(void)
{
}

void io_set_directory_position(DirectoryPosition pos)
{
}

void io_set_device_filename(unsigned char ds, unsigned char hs, unsigned char mode, char* e)
{
}

char *io_get_device_filename(unsigned char slot)
{
  return "";
}

void io_set_boot_config(unsigned char toggle)
{
}

void io_set_boot_mode(unsigned char mode)
{
}


uint8_t io_mount_disk_image(unsigned char ds, unsigned char mode)
{
  return 0;
}

void io_umount_disk_image(unsigned char ds)
{
}

void io_boot(void)
{
}

void io_create_new(unsigned char selected_host_slot, unsigned char selected_device_slot, unsigned long selected_size, char *path)
{
}

void io_build_directory(unsigned char ds, unsigned long numBlocks, char *v)
{
}

bool io_get_device_enabled_status(unsigned char d)
{
  return false;
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

/**
 * NOTE: On the Fuji, command 0xD8 (copy file) expects the slots to be 1-8, not 0-7 like most things.
 * That's why we add one, since config is tracking the slots as 0-7
 */
void io_copy_file(unsigned char source_slot, unsigned char destination_slot)
{
}

unsigned char io_device_slot_to_device(unsigned char ds)
{
  return 0;
}

/**
 * Get filename for device slot
 */
void io_get_filename_for_device_slot(unsigned char slot, const char *filename)
{
}

/**
 * Mount all hosts and devices
 */
bool io_mount_all(void)
{
  return false;
}

#endif /* __WATCOMC__ */
