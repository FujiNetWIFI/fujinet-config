#ifdef BUILD_CONFIG86
/**
 * FujiNet Drivewire calls
 */

#include <stdbool.h>
#include "io.h"
#include "globals.h"
#include "screen.h"
#include "bar.h"

extern NetConfig nc;
static AdapterConfig adapterConfig;
static SSIDInfo ssidInfo;
NewDisk newDisk;
unsigned char wifiEnabled=true;
unsigned char response[256];
int _dirpos=0;
unsigned char orig_casflag;

/**
 * @brief wait for FujiNet device ready
 */
void io_ready(void)
{
}

/**
 * @brief Get response data from last command
 * @param devid The device ID (0-255) 
 * @param buf Target buffer 
 * @param len Length 
 */
unsigned char io_get_response(unsigned char *buf, int len)
{
}

bool io_error(void)
{
  return false;
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
    return 3;
}

NetConfig *io_get_ssid(void)
{
  return &nc;
}

unsigned char io_scan_for_networks(void)
{
  char r=-1;

  return r;
}

SSIDInfo *io_get_scan_result(int n)
{
    return &ssidInfo;
}

AdapterConfig *io_get_adapter_config(void)
{
    return &adapterConfig;
}

int io_set_ssid(NetConfig *nc)
{
    return false;
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

const char *io_read_directory(unsigned char maxlen, unsigned char a)
{    
    return (const char *)response;
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

const char *io_get_device_filename(unsigned char slot)
{
    return (const char *)response;
}

void io_set_boot_config(unsigned char toggle)
{
}

void io_set_boot_mode(unsigned char mode)
{
}


void io_mount_disk_image(unsigned char ds, unsigned char mode)
{
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
unsigned char io_mount_all(void)
{
  return 1;
}

#endif /* BUILD_CONFIG86 */
