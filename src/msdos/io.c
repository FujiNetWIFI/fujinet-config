#ifdef BUILD_ATARI
/**
 * FujiNet SIO calls
 */

#include <atari.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h> // For sleep
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
}

bool io_get_wifi_enabled(void)
{
  wifiEnabled = fuji_get_wifi_enabled();

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
  unsigned char status;
  fuji_get_wifi_status(&status);

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
  fuji_get_ssid(&nc);
  return &nc;
}

unsigned char io_scan_for_networks(void)
{
  uint8_t count;
  fuji_scan_for_networks(&count);
  return count;
}

SSIDInfo *io_get_scan_result(unsigned char n)
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
  io_get_wifi_status(); // change bar color based on status.
  return is_err;
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

uint8_t io_mount_host_slot(unsigned char hs)
{
  if (hostSlots[hs][0] == 0) return 2;
  fuji_mount_host_slot(hs);
  return 0;
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
  fuji_open_directory(hs, _p);
}

char *io_read_directory(unsigned char maxlen, unsigned char a)
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

void io_set_device_filename(unsigned char ds, unsigned char hs, unsigned char mode, char* e)
{
  fuji_set_device_filename(mode, hs, ds, e);
}

char *io_get_device_filename(unsigned char slot)
{
  fuji_get_device_filename(slot, &response);
  return &response;
}

void io_set_boot_config(unsigned char toggle)
{
  fuji_set_boot_config(toggle);
}

void io_set_boot_mode(unsigned char mode)
{
  fuji_set_boot_mode(mode);
}


uint8_t io_mount_disk_image(unsigned char ds, unsigned char mode)
{
  return fuji_mount_disk_image(ds, mode);
}

void io_umount_disk_image(unsigned char ds)
{
  fuji_unmount_disk_image(ds);
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
  fuji_create_new(&newDisk);

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
  fuji_copy_file(source_slot, destination_slot, &copySpec);
}

unsigned char io_device_slot_to_device(unsigned char ds)
{
}

/**
 * Get filename for device slot
 */
void io_get_filename_for_device_slot(unsigned char slot, const char *filename)
{
  fuji_get_device_filename(slot, filename);
}

/**
 * Mount all hosts and devices
 */
bool io_mount_all(void)
{
  return fuji_mount_all();
  // return OS.dcb.dstats; // 1 = successful, anything else = error.
}

#endif /* BUILD_ATARI */
