#ifdef _CMOC_VERSION_
/**
 * FujiNet Drivewire calls
 */

#include <cmoc.h>
#include <coco.h>
#include <fujinet-fuji.h>
#include "io.h"
#include "globals.h"
#include "screen.h"
#include "bar.h"
#include "../pause.h"

#define OP_FUJI 0xE2
#define CMD_READY 0x00
#define CMD_RESPONSE 0x01

extern NetConfig nc;
static AdapterConfig adapterConfig;
static SSIDInfo ssidInfo;
NewDisk newDisk;
unsigned char wifiEnabled=true;
byte response[256];
int _dirpos=0;
byte orig_casflag;

bool io_error(void)
{
    return fuji_error();
}

unsigned char io_status(void)
{
  return (unsigned char) io_error();
}

void io_init(void)
{
	// There's no partnering exit function for screen_init, so we'll set up
	//   how casing is being handled here.  We default to lowercase.
    // Set casing to lowercase.

	asm {
		lda $011A
		sta orig_casflag
		clr $011A
	}
}

bool io_get_wifi_enabled(void)
{
    return fuji_get_wifi_enabled();
}

unsigned char io_get_wifi_status(void)
{
    unsigned char status;
    fuji_get_wifi_status(&status);
    return status;
}

NetConfig *io_get_ssid(void)
{
    memset(&nc, 0, sizeof(NetConfig));
    fuji_get_ssid(&nc);
    return &nc;
}

unsigned char io_scan_for_networks(void)
{
    unsigned char r=-1;

    fuji_scan_for_networks(&r);
  
    if (r > 11)
    {
        r=11;
    }
  
  return r;
}

SSIDInfo *io_get_scan_result(int n)
{
    memset(&ssidInfo, 0, sizeof(SSIDInfo));
    fuji_get_scan_result((unsigned char) n, &ssidInfo);
    return &ssidInfo;
}

AdapterConfig *io_get_adapter_config(void)
{
    memset(&adapterConfig, 0, sizeof(adapterConfig));

    fuji_get_adapter_config(&adapterConfig);

    return &adapterConfig;
}

int io_set_ssid(NetConfig *nc)
{
    fuji_set_ssid(nc);
    return false;
}

void io_get_device_slots(DeviceSlot *d)
{
    fuji_get_device_slots(d, NUM_DEVICE_SLOTS);  
}

void io_get_host_slots(HostSlot *h)
{
    fuji_get_host_slots(h, NUM_HOST_SLOTS);
}

void io_put_host_slots(HostSlot *h)
{
    fuji_put_host_slots(h, NUM_HOST_SLOTS);
}

void io_put_device_slots(DeviceSlot *d)
{
    fuji_put_device_slots(d, NUM_DEVICE_SLOTS);
}

void io_mount_host_slot(unsigned char hs)
{
    fuji_mount_host_slot(hs);
}

void io_open_directory(unsigned char hs, char *p, char *f)
{
	char *_p = p;
	if (f[0] != 0x00)
	{
		// We have a filter, create a directory+filter string
		memset(response, 0, 256);
		strcpy((char *) response, p);
		strcpy((char *) &response[strlen((char *) response) + 1], f);
		_p = (char *) &response;
	}
	fuji_open_directory(hs, _p);
}

const char *io_read_directory(unsigned char maxlen, unsigned char a)
{
    memset(response,0,sizeof(response));
    fuji_read_directory(maxlen, a, (char *) &response[0]);
    
    return (const char *)response;
}

void io_close_directory(void)
{
    fuji_close_directory();
}

void io_set_directory_position(DirectoryPosition pos)
{
    fuji_set_directory_position(pos);
}

void io_set_device_filename(unsigned char ds, char* e)
{
    char fn[256];
    strcpy(fn,e);
    
    fuji_set_device_filename(mode, selected_host_slot, ds, fn);
}

const char *io_get_device_filename(unsigned char slot)
{
    memset(response,0,sizeof(response));
    fuji_get_device_filename(slot, (char *) &response[0]);
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
    fuji_mount_disk_image(ds, mode);
}

void io_umount_disk_image(unsigned char ds)
{
    fuji_unmount_disk_image(ds);
}

void io_boot(void)
{
	// Restore the original casing flag.
	asm {
		lda orig_casflag
		sta $011A
	}
    pause(180); // Pause for 3 seconds to let the user see the mounted disks.
    coldStart();
	exit(0);
}

void io_create_new(unsigned char selected_host_slot, unsigned char selected_device_slot, unsigned long selected_size, char *path)
{
    memset(&newDisk, 0, sizeof(newDisk));
    newDisk.numDisks = (byte)selected_size;
    newDisk.hostSlot = selected_host_slot;
    newDisk.deviceSlot = selected_device_slot;
    strcpy(newDisk.filename, path);

    fuji_create_new(&newDisk);
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
    fuji_copy_file(source_slot+1, destination_slot+1, &copySpec[0]);
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

#endif /* _CMOC_VERSION_ */
