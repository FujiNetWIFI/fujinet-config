#ifdef BUILD_ATARI
/**
 * #FujiNet SIO calls
 */

#include <atari.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <conio.h> // Used for interacting with the standard Atari 'console'
#include <unistd.h> // For sleep
#include "io.h"
#include "globals.h"
#include "screen.h"
#include "bar.h"
#include "fn_io.h"

static NetConfig nc;
static AdapterConfigExtended adapterConfig;
static SSIDInfo ssidInfo;
NewDisk newDisk;
unsigned char wifiEnabled=true;

// variable to hold various responses that we just need to return a char*.
char response[256];

void set_sio_defaults(void)
{
  OS.dcb.ddevic = 0x70;
  OS.dcb.dunit = 1;
  OS.dcb.dtimlo = 0x0F; // 15 second timeout
}

bool io_error(void)
{
  return (OS.dcb.dstats > 128);
}

unsigned char io_status(void)
{
}

void io_init(void)
{
  OS.noclik = 0xFF;
  OS.shflok = 0;
  OS.color0 = 0x9f;
  OS.color1 = 0x0f;
  OS.color2 = 0x90;
  OS.color4 = 0x90;
  OS.coldst = 1;
  OS.sdmctl = 0; // Turn off screen
}

bool io_get_wifi_enabled(void)
{
  wifiEnabled = fn_io_get_wifi_enabled();

  if (wifiEnabled == 1)
  {
    return true;
  }
  else
  {
    bar_set_color(COLOR_SETTING_FAILED);
    return false;
  }
}

unsigned char io_get_wifi_status(void)
{
  unsigned char status = fn_io_get_wifi_status();

  // Shouldn't do this here, but for now its temporary
  // If ^^ is ever fixed, need to change io_set_ssid below too, which uses this colour hack
  if (status != 3)
  {
    bar_set_color(COLOR_SETTING_FAILED);
  }
  else
  {
    bar_set_color(COLOR_SETTING_SUCCESSFUL);
  }

  return status;
}

NetConfig *io_get_ssid(void)
{
  fn_io_get_ssid(&nc);
  return &nc;
}

unsigned char io_scan_for_networks(void)
{
  return fn_io_scan_for_networks();
}

SSIDInfo *io_get_scan_result(unsigned char n)
{
  fn_io_get_scan_result(n, &ssidInfo);
  return &ssidInfo;
}

AdapterConfigExtended *io_get_adapter_config(void)
{
  fn_io_get_adapter_config_extended(&adapterConfig);
  return &adapterConfig;
}

int io_set_ssid(NetConfig *nc)
{
  bool is_err;
 
  fn_io_set_ssid(nc);
  is_err = fn_io_error();
  io_get_wifi_status(); // change bar color based on status.
  return is_err;
}

void io_get_device_slots(DeviceSlot *d)
{
  fn_io_get_device_slots(d);
}

void io_get_host_slots(HostSlot *h)
{
  fn_io_get_host_slots(h);
}

void io_put_host_slots(HostSlot *h)
{
  fn_io_put_host_slots(h);
}

void io_put_device_slots(DeviceSlot *d)
{
  fn_io_put_device_slots(d);
}

void io_mount_host_slot(unsigned char hs)
{
  if (hostSlots[hs][0] == 0) return;
  fn_io_mount_host_slot(hs);
}

void io_open_directory(unsigned char hs, char *p, char *f)
{
  char *_p = p;
  if (f[0] != 0x00)
  {
    // We have a filter, create a directory+filter string
    memset(response, 0, 256);
    strcpy(response, p);
    strcpy(&response[strlen(response) + 1], f);
    _p = &response;
  }
  fn_io_open_directory(hs, _p);
}

char *io_read_directory(unsigned char maxlen, unsigned char a)
{
  memset(response, 0, maxlen);
  return fn_io_read_directory(maxlen, a, &response);
}

void io_close_directory(void)
{
  fn_io_close_directory();
}

void io_set_directory_position(DirectoryPosition pos)
{
  fn_io_set_directory_position(pos);
}

void io_set_device_filename(unsigned char ds, unsigned char hs, unsigned char mode, char* e)
{
  fn_io_set_device_filename(mode, hs, ds, e);
}

char *io_get_device_filename(unsigned char slot)
{
  fn_io_get_device_filename(slot, &response);
  return &response;
}

void io_set_boot_config(unsigned char toggle)
{
  fn_io_set_boot_config(toggle);
}

void io_set_boot_mode(unsigned char mode)
{
  fn_io_set_boot_mode(mode);
}


void io_mount_disk_image(unsigned char ds, unsigned char mode)
{
  fn_io_mount_disk_image(ds, mode);
}

void io_umount_disk_image(unsigned char ds)
{
  fn_io_unmount_disk_image(ds);
}

void io_boot(void)
{
}

void io_create_new(unsigned char selected_host_slot, unsigned char selected_device_slot, unsigned long selected_size, char *path)
{
  switch (selected_size)
  {
  case 90:
    newDisk.numSectors = 720;
    newDisk.sectorSize = 128;
    break;
  case 130:
    newDisk.numSectors = 1040;
    newDisk.sectorSize = 128;
    break;
  case 180:
    newDisk.numSectors = 720;
    newDisk.sectorSize = 256;
    break;
  case 360:
    newDisk.numSectors = 1440;
    newDisk.sectorSize = 256;
    break;
  case 720:
    newDisk.numSectors = 2880;
    newDisk.sectorSize = 256;
    break;
  case 1440:
    newDisk.numSectors = 5760;
    newDisk.sectorSize = 256;
    break;
  case 999:
    newDisk.numSectors = custom_numSectors;
    newDisk.sectorSize = custom_sectorSize;
    break;
  default:
    return;
  }

  newDisk.hostSlot = selected_host_slot;
  newDisk.deviceSlot = selected_device_slot;
  deviceSlots[selected_device_slot].mode = mode;
  strcpy(newDisk.filename, path);

  deviceSlots[selected_device_slot].mode = mode;
  fn_io_create_new(&newDisk);

}

void io_build_directory(unsigned char ds, unsigned long numBlocks, char *v)
{
}

bool io_get_device_enabled_status(unsigned char d)
{
  // adam calls $D1, which doesn't exist in atari.
  return false;
}

void io_update_devices_enabled(bool *e)
{
  int deviceNum;

  // doesn't exist on atari?. Just set to true.
  for (deviceNum = 0; deviceNum < NUM_DEVICE_SLOTS; deviceNum++)
  {
    e[deviceNum] = true;
  }
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
  // incrementing is handled in function, we keep everything 0 based
  fn_io_copy_file(source_slot, destination_slot, &copySpec);
}

unsigned char io_device_slot_to_device(unsigned char ds)
{
}

/**
 * Get filename for device slot
 */
void io_get_filename_for_device_slot(unsigned char slot, const char *filename)
{
  fn_io_get_device_filename(slot, filename);
}

/**
 * Mount all hosts and devices
 */
unsigned char io_mount_all(void)
{
  fn_io_mount_all();
  return OS.dcb.dstats; // 1 = successful, anything else = error.
}

#endif /* BUILD_ATARI */