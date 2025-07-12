#ifdef __WATCOMC__
/**
 * FujiNet SIO calls
 */

#include <stdlib.h>
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
unsigned char err=0;

// Register pack for INT F5 calls
union REGS r;
struct SREGS sr;

// variable to hold various responses that we just need to return a char*.
char response[1024];

bool io_error(void)
{
  return err != 'C';
}

unsigned char io_status(void)
{
  return err;
}

void io_init(void)
{
  err=0;
}

bool io_get_wifi_enabled(void)
{
  r.h.dl = 0x40;
  r.h.al = 0x70;
  r.h.ah = 0xea;
  r.h.cl = 0x00;
  r.h.ch = 0x00;
  r.x.si = 0x0000;
  sr.es  = FP_SEG(response);
  r.x.bx = FP_OFF(response);
  r.x.di = 1;

  int86x(0xF5,&r,&r,&sr);
  err=r.h.al;
  return (bool)response[0];
}

unsigned char io_get_wifi_status(void)
{
  r.h.dl = 0x40;
  r.h.al = 0x70;
  r.h.ah = 0xFA;
  r.h.cl = 0x00;
  r.h.ch = 0x00;
  r.x.si = 0x0000;
  sr.es  = FP_SEG(response);
  r.x.bx = FP_OFF(response);
  r.x.di = 1;

  int86x(0xF5,&r,&r,&sr);
  err=r.h.al;
  return (unsigned char)response[0];
}

NetConfig *io_get_ssid(void)
{
  r.h.dl = 0x40;
  r.h.al = 0x70;
  r.h.ah = 0xFE;
  r.h.cl = 0x00;
  r.h.ch = 0x00;
  r.x.si = 0x0000;
  sr.es  = FP_SEG(response);
  r.x.bx = FP_OFF(response);
  r.x.di = sizeof(NetConfig);

  int86x(0xF5,&r,&r,&sr);
  err=r.h.al;
  return (NetConfig *)response;
}

unsigned char io_scan_for_networks(void)
{
  r.h.dl = 0x40;
  r.h.al = 0x70;
  r.h.ah = 0xFD;
  r.h.cl = 0x00;
  r.h.ch = 0x00;
  r.x.si = 0x0000;
  sr.es  = FP_SEG(response);
  r.x.bx = FP_OFF(response);
  r.x.di = 1;

  int86x(0xF5,&r,&r,&sr);
  err=r.h.al;
  return (unsigned char)response[0];
}

SSIDInfo *io_get_scan_result(unsigned char n)
{
  r.h.dl = 0x40;
  r.h.al = 0x70;
  r.h.ah = 0xFC;
  r.h.cl = 0x00;
  r.h.ch = 0x00;
  r.x.si = 0x0000;
  sr.es  = FP_SEG(response);
  r.x.bx = FP_OFF(response);
  r.x.di = sizeof(SSIDInfo);

  int86x(0xF5,&r,&r,&sr);

  err=r.h.al;
  return (SSIDInfo *)response;
}

AdapterConfig *io_get_adapter_config(void)
{
  r.h.dl = 0x40;
  r.h.al = 0x70;
  r.h.ah = 0xe8;
  r.h.cl = 0x00;
  r.h.ch = 0x00;
  r.x.si = 0x0000;
  sr.es  = FP_SEG(response);
  r.x.bx = FP_OFF(response);
  r.x.di = sizeof(AdapterConfig);

  int86x(0xF5,&r,&r,&sr);

  err=r.h.al;
  return (AdapterConfig *)response;
}

int io_set_ssid(NetConfig *nc)
{
  r.h.dl = 0x80;
  r.h.al = 0x70;
  r.h.ah = 0xFB;
  r.h.cl = 0x00;
  r.h.ch = 0x00;
  r.x.si = 0x0000;
  sr.es  = FP_SEG(response);
  r.x.bx = FP_OFF(response);
  r.x.di = sizeof(NetConfig);

  int86x(0xF5,&r,&r,&sr);

  err=r.h.al;
  return r.h.al;
}

bool io_get_device_slots(DeviceSlot *d)
{
  r.h.dl = 0x40;
  r.h.al = 0x70;
  r.h.ah = 0xF2;
  r.h.cl = 0x00;
  r.h.ch = 0x00;
  r.x.si = 0x0000;
  sr.es  = FP_SEG(d);
  r.x.bx = FP_OFF(d);
  r.x.di = sizeof(DeviceSlot) * NUM_DEVICE_SLOTS;

  int86x(0xF5,&r,&r,&sr);

  err=r.h.al;
  return r.h.al == 'C';
}

bool io_get_host_slots(HostSlot *h)
{
  r.h.dl = 0x40;
  r.h.al = 0x70;
  r.h.ah = 0xF4;
  r.h.cl = 0x00;
  r.h.ch = 0x00;
  r.x.si = 0x0000;
  sr.es  = FP_SEG(h);
  r.x.bx = FP_OFF(h);
  r.x.di = sizeof(HostSlot) * NUM_HOST_SLOTS;

  int86x(0xF5,&r,&r,&sr);

  err=r.h.al;
  return r.h.al == 'C';
}

bool io_put_host_slots(HostSlot *h)
{
  r.h.dl = 0x80;
  r.h.al = 0x70;
  r.h.ah = 0xF3;
  r.h.cl = 0x00;
  r.h.ch = 0x00;
  r.x.si = 0x0000;
  sr.es  = FP_SEG(h);
  r.x.bx = FP_OFF(h);
  r.x.di = sizeof(HostSlot) * NUM_HOST_SLOTS;

  int86x(0xF5,&r,&r,&sr);

  err=r.h.al;
  return r.h.al == 'C';
}

bool io_put_device_slots(DeviceSlot *d)
{
  r.h.dl = 0x80;
  r.h.al = 0x70;
  r.h.ah = 0xF1;
  r.h.cl = 0x00;
  r.h.ch = 0x00;
  r.x.si = 0x0000;
  sr.es  = FP_SEG(d);
  r.x.bx = FP_OFF(d);
  r.x.di = sizeof(DeviceSlot) * NUM_DEVICE_SLOTS;

  int86x(0xF5,&r,&r,&sr);

  err=r.h.al;
  return r.h.al == 'C';
}

unsigned char io_mount_host_slot(unsigned char hs)
{
  r.h.dl = 0x00;
  r.h.al = 0x70;
  r.h.ah = 0xF9;
  r.h.cl = 0x00;
  r.h.ch = 0x00;
  r.x.si = 0x0000;
  r.x.di = 0x00;

  int86(0xF5,&r,&r);

  err=r.h.al;
  return r.h.al;
}

void io_open_directory(unsigned char hs, char *p, char *f)
{
  char *e;

  memset(&response,0,sizeof(response));
  r.h.dl = 0x80;
  r.h.al = 0x70;
  r.h.ah = 0xF7;
  r.h.cl = hs;
  r.h.ch = 0x00;
  r.x.si = 0x0000;
  r.x.di = 256;

  strcpy(&response,p);
  e=&response[0];

  if (f[0]!=0x00)
    {
      while (*e != 0x00)
	e++;

      e++;
      strcpy(e,f);
    }

  int86x(0xF5,&r,&r,&sr);
  err=r.h.al;
}

const char *io_read_directory(unsigned char maxlen, unsigned char a)
{
  r.h.dl = 0x40;
  r.h.al = 0x70;
  r.h.ah = 0xF6;
  r.h.cl = maxlen;
  r.h.ch = a;
  r.x.si = 0x0000;
  r.x.di = (a ? maxlen + 10 : maxlen);
  sr.es  = FP_SEG(response);
  r.x.bx = FP_OFF(response);

  int86x(0xF5,&r,&r,&sr);

  err=r.h.al;
  return (const char *)response;
}

void io_close_directory(void)
{
  r.h.dl = 0x00;
  r.h.al = 0x70;
  r.h.ah = 0xF5;
  r.h.cl = 0x00;
  r.h.ch = 0x00;
  r.x.si = 0x0000;

  int86(0xF5,&r,&r);
  err=r.h.al;
}

void io_set_directory_position(DirectoryPosition pos)
{
  r.h.dl = 0x00;
  r.h.al = 0x70;
  r.h.ah = 0xE4;
  r.x.cx = pos;
  r.x.si = 0x0000;
  
  int86(0xF5,&r,&r);
  err=r.h.al;
}

void io_set_device_filename(unsigned char ds, unsigned char hs, unsigned char mode, char* e)
{
  r.h.dl = 0x80;
  r.h.al = 0x70;
  r.h.ah = 0xE2;
  r.h.cl = hs;
  r.h.ch = mode;
  r.x.si = 0x0000;
  sr.es  = FP_SEG(e);
  r.x.bx = FP_OFF(e);
  r.x.di = 256;

  int86x(0xF5,&r,&r,&sr);
  err=r.h.al;
}

const char *io_get_device_filename(unsigned char slot)
{
  r.h.dl = 0x40;
  r.h.al = 0x70;
  r.h.ah = 0xDA;
  r.h.cl = slot;
  r.h.ch = 0x00;
  r.x.si = 0x0000;
  sr.es  = FP_SEG(response);
  r.x.bx = FP_OFF(response);
  r.x.di = 256;

  int86x(0xF5,&r,&r,&sr);
  err=r.h.al;
  return (char *)response;
}

void io_set_boot_config(unsigned char toggle)
{
  r.h.dl = 0x00;
  r.h.al = 0x70;
  r.h.ah = 0xD9;
  r.h.cl = toggle;
  r.h.ch = 0x00;
  r.x.si = 0x0000;
  r.x.di = 0x0000;

  int86(0xF5,&r,&r);
  err=r.h.al;
}

void io_set_boot_mode(unsigned char mode)
{
  r.h.dl = 0x00;
  r.h.al = 0x70;
  r.h.ah = 0xD6;
  r.h.cl = mode;
  r.h.ch = 0x00;
  r.x.si = 0x0000;
  r.x.di = 0x0000;

  int86(0xF5,&r,&r);
  err=r.h.al;
}


unsigned char io_mount_disk_image(unsigned char ds, unsigned char mode)
{
  r.h.dl = 0x00;
  r.h.al = 0x70;
  r.h.ah = 0xF8;
  r.h.cl = ds;
  r.h.ch = mode;
  r.x.si = 0x0000;
  r.x.di = 0x0000;

  int86(0xF5,&r,&r);
  err=r.h.al;
  return r.h.al == 'C';
}

void io_umount_disk_image(unsigned char ds)
{
  r.h.dl = 0x00;
  r.h.al = 0x70;
  r.h.ah = 0xE9;
  r.h.cl = ds;
  r.h.ch = 0x00;
  r.x.si = 0x0000;
  r.x.di = 0x0000;

  int86(0xF5,&r,&r);
  err=r.h.al;
}

void io_boot(void)
{
  exit(0);
}

void io_create_new(unsigned char selected_host_slot, unsigned char selected_device_slot, unsigned long selected_size, char *path)
{
}

void io_build_directory(unsigned char ds, unsigned long numBlocks, char *v)
{
  // Not used
}

bool io_get_device_enabled_status(unsigned char d)
{
  // not used
  return false;
}

void io_update_devices_enabled(bool *e)
{
  // not used
}

void io_enable_device(unsigned char d)
{
  // not used
}

void io_disable_device(unsigned char d)
{
  // not used
}

/**
 * NOTE: On the Fuji, command 0xD8 (copy file) expects the slots to be 1-8, not 0-7 like most things.
 * That's why we add one, since config is tracking the slots as 0-7
 */
void io_copy_file(unsigned char source_slot, unsigned char destination_slot)
{
  r.h.dl = 0x80;
  r.h.al = 0x70;
  r.h.ah = 0xD8;
  r.h.cl = source_slot;
  r.h.ch = destination_slot;
  r.x.si = 0x0000;
  sr.es  = FP_SEG(copySpec);
  r.x.bx = FP_OFF(copySpec);
  r.x.di = 256;

  int86x(0xF5,&r,&r,&sr);
  err=r.h.al;
}

unsigned char io_device_slot_to_device(unsigned char ds)
{
  return ds;
}

/**
 * Get filename for device slot
 */
void io_get_filename_for_device_slot(unsigned char slot, const char *filename)
{
  // Why the fuck is this here?
}

/**
 * Mount all hosts and devices
 */
bool io_mount_all(void)
{
  r.h.dl = 0x00;
  r.h.al = 0x70;
  r.h.ah = 0xD7;
  r.h.cl = 0x00;
  r.h.ch = 0x00;
  r.x.si = 0x0000;
  r.x.di = 0x0000;

  int86(0xF5,&r,&r);
  
  err=r.h.al;
  return r.h.al == 'C';
}

#endif /* __WATCOMC__ */
