#ifdef _CMOC_VERSION_
/**
 * #FujiNet Drivewire calls
 */

#include <cmoc.h>
#include "stdbool.h"
#include "io.h"
#include "globals.h"
#include "screen.h"
#include "bar.h"

extern NetConfig nc;
static AdapterConfigExtended adapterConfig;
static SSIDInfo ssidInfo;
NewDisk newDisk;
unsigned char wifiEnabled=true;

/**
 * @brief Read string to s from DriveWire with expected length l
 * @param s pointer to string buffer
 * @param l expected length of string (0-65535 bytes)
 * @return # of bytes actually read
 */
int dwread(byte *s, int l)
{
  asm
    {
    pshs x,y
    ldx :s
    ldy :l
    jsr [0xD93F]
    puls y,x
    tfr cc,b
    lsrb
    lsrb
    andb #$01
    }
}

/**
 * @brief Write string at s to DriveWire with length l
 * @param s pointer to string buffer
 * @param l length of string (0-65535 bytes)
 * @return # of bytes actually written
 */
int dwwrite(byte *s, int l)
{
  asm
    {
        pshs x,y
        ldx :s
	ldy :l
        jsr [0xD941]
	tfr cc,d
	puls y,x
    }
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
  byte *s="\xE2\xEA";
  int l = 2;
  bool r=0;

  dwwrite(s,l);
  dwread(&r,1);

  return r;
}

unsigned char io_get_wifi_status(void)
{
  byte *s="\xE2\xFA";
  int l = 2;
  byte r;

  sleep(1);
  
  dwwrite(s,l);
  dwread(&r,1);
  
  return r;
}

NetConfig *io_get_ssid(void)
{
  dwwrite((byte *)"\xE2\xFE",2);
  dwread((unsigned char *)&nc,sizeof(nc));
  
  return &nc;
}

unsigned char io_scan_for_networks(void)
{
  unsigned char r=0;
  
  while (true)
    {
      dwwrite((byte *)"\xE2\xFD",2);
      if (dwread(&r,1))
	break;
    }

  if (r > 11)
    r=11;
  
  return r;
}

SSIDInfo *io_get_scan_result(unsigned char n)
{
  byte s[3]={0xE2,0xFC,0x00};

  s[2] = n;

  dwwrite((byte *)s,3);
  dwread((unsigned char *)&ssidInfo,sizeof(SSIDInfo));

  return &ssidInfo;
}

AdapterConfigExtended *io_get_adapter_config(void)
{
  dwwrite((byte *)"\xE2\xE8",2);
  dwread((unsigned char *)&adapterConfig,sizeof(AdapterConfigExtended));
  return &adapterConfig;
}

int io_set_ssid(NetConfig *nc)
{
  struct _setssid
  {
    char fuji;
    char cmd;
    char ssid[33];
    char password[64];
  } s;

  s.fuji=0xE2;
  s.cmd=0xFB;
  memcpy(s.ssid,nc->ssid,33);
  memcpy(s.password,nc->password,64);

  dwwrite((byte *)&s,sizeof(s));
  
  return false;
}

void io_get_device_slots(DeviceSlot *d)
{
  byte *s="\xE2\xF2";

  dwwrite(s,2);
  dwread((unsigned char *)d,304);
}

void io_get_host_slots(HostSlot *h)
{
  byte *s="\xE2\xF4";

  memset(h,0,256);
  
  dwwrite(s,2);
  dwread(h,256);
}

void io_put_host_slots(HostSlot *h)
{
  struct _puthostslots
  {
    unsigned char fuji;
    unsigned char cmd;
    HostSlot hostSlots[8];
  } s;

  memset(h,0,sizeof(s));
  s.fuji=0xE2;
  s.cmd=0xF3;
  memcpy(&s.hostSlots[0],&h[0],256);

  dwwrite((char *)&s,sizeof(s));
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
  return "DIR ENTRY";
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
  return "DEVICE FILENAME";
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

#endif /* _CMOC_VERSION_ */
