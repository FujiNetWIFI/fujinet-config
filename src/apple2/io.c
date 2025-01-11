#ifdef BUILD_APPLE2
/**
 * FujiNet CONFIG for #Apple2
 *
 * I/O Routines
 */

#include "io.h"
#include <stdint.h>
#ifdef __ORCAC__
#include <coniogs.h>
#include <apple2gs.h>
#include <texttool.h>
#else
#include <conio.h>
#include <apple2.h>
#include <peekpoke.h> // For the insanity in io_boot()
#endif
#include <stdlib.h>
#include "globals.h"

#include "fujinet-fuji.h"

#define FUJICMD_RESET 0xFF
#define FUJICMD_GET_SSID 0xFE
#define FUJICMD_SCAN_NETWORKS 0xFD
#define FUJICMD_GET_SCAN_RESULT 0xFC
#define FUJICMD_SET_SSID 0xFB
#define FUJICMD_GET_WIFISTATUS 0xFA
#define FUJICMD_MOUNT_HOST 0xF9
#define FUJICMD_MOUNT_IMAGE 0xF8
#define FUJICMD_OPEN_DIRECTORY 0xF7
#define FUJICMD_READ_DIR_ENTRY 0xF6
#define FUJICMD_CLOSE_DIRECTORY 0xF5
#define FUJICMD_READ_HOST_SLOTS 0xF4
#define FUJICMD_WRITE_HOST_SLOTS 0xF3
#define FUJICMD_READ_DEVICE_SLOTS 0xF2
#define FUJICMD_WRITE_DEVICE_SLOTS 0xF1
#define FUJICMD_UNMOUNT_IMAGE 0xE9
#define FUJICMD_GET_ADAPTERCONFIG 0xE8
#define FUJICMD_NEW_DISK 0xE7
#define FUJICMD_UNMOUNT_HOST 0xE6
#define FUJICMD_GET_DIRECTORY_POSITION 0xE5
#define FUJICMD_SET_DIRECTORY_POSITION 0xE4
//#define FUJICMD_SET_HSIO_INDEX 0xE3
#define FUJICMD_SET_DEVICE_FULLPATH 0xE2
#define FUJICMD_SET_HOST_PREFIX 0xE1
#define FUJICMD_GET_HOST_PREFIX 0xE0
//#define FUJICMD_SET_SIO_EXTERNAL_CLOCK 0xDF
#define FUJICMD_WRITE_APPKEY 0xDE
#define FUJICMD_READ_APPKEY 0xDD
#define FUJICMD_OPEN_APPKEY 0xDC
#define FUJICMD_CLOSE_APPKEY 0xDB
#define FUJICMD_GET_DEVICE_FULLPATH 0xDA
#define FUJICMD_CONFIG_BOOT 0xD9
#define FUJICMD_COPY_FILE 0xD8
#define FUJICMD_MOUNT_ALL 0xD7
#define FUJICMD_SET_BOOT_MODE 0xD6
#define FUJICMD_ENABLE_DEVICE 0xD5
#define FUJICMD_DISABLE_DEVICE 0xD4
#define FUJICMD_DEVICE_STATUS 0xD1
#define FUJICMD_GET_ADAPTERCONFIG_EXTENDED 0xC4
#define FUJICMD_STATUS 0x53

#define UNUSED(x) (void)(x);

#include <string.h>
#include "sp.h"

static NetConfig nc;
static AdapterConfigExtended adapterConfig;
static SSIDInfo ssidInfo;
NewDisk newDisk;

// variable to hold various responses that we just need to return a char*.
char response[256];

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
  return fuji_error();
}

uint8_t io_get_wifi_status(void)
{
  unsigned char status = 0;
  unsigned long l = 0;
  for (l=0;l<4096;l++);
  fuji_get_wifi_status(&status);
  return status;
}

NetConfig* io_get_ssid(void)
{
  fuji_get_ssid(&nc);
  return &nc;
}

uint8_t io_scan_for_networks(void)
{
  uint8_t count;
  fuji_scan_for_networks(&count);
  return count;
}

SSIDInfo *io_get_scan_result(uint8_t n)
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
  fuji_set_ssid(nc);
  return 0; // ??? TODO, should this reflect the results?
}

char *io_get_device_filename(uint8_t slot)
{
  fuji_get_device_filename(slot, &response[0]);
  return &response[0];
}

void io_create_new(uint8_t selected_host_slot,uint8_t selected_device_slot,unsigned long selected_size,char *path)
{
  newDisk.createType = io_create_type;
  newDisk.deviceSlot = selected_device_slot;
  newDisk.hostSlot = selected_host_slot;
  newDisk.numBlocks = selected_size;
  strcpy(&newDisk.filename[0], path);
  fuji_create_new(&newDisk);
}

bool io_get_device_slots(DeviceSlot *d)
{
  return fuji_get_device_slots(d, 6);
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
  fuji_put_device_slots(d, 6);
}

void io_mount_host_slot(uint8_t hs)
{
  fuji_mount_host_slot(hs);
}

void io_open_directory(uint8_t hs, char *p, char *f)
{
  char *_p = p;
  if (f[0] != 0x00)
  {
    // We have a filter, create a directory+filter string
    memset(response, 0, 256);
    strcpy(response, p);
    strcpy(&response[strlen(response) + 1], f);
    _p = &response[0];
  }
  fuji_open_directory(hs, _p);
}

char *io_read_directory(uint8_t maxlen, uint8_t a)
{
  memset(&response[0], 0, maxlen);
  fuji_read_directory(maxlen, a, &response[0]);
  
  return &response[0];
}

void io_close_directory(void)
{
  fuji_close_directory();
}

void io_set_directory_position(DirectoryPosition pos)
{
  fuji_set_directory_position(pos);
}

void io_set_device_filename(uint8_t ds, uint8_t hs, uint8_t mode, char* e)
{
  fuji_set_device_filename(mode, hs, ds, e);
}

bool io_mount_disk_image(uint8_t ds, uint8_t mode)
{
  return fuji_mount_disk_image(ds, mode);
}

void io_copy_file(unsigned char source_slot, unsigned char destination_slot)
{
  fuji_copy_file(source_slot, destination_slot, &copySpec[0]);
}

void io_set_boot_config(uint8_t toggle)
{
  #ifndef __ORCAC__
  fuji_set_boot_config(toggle);
  #endif
}

void io_set_boot_mode(uint8_t mode)
{
  fuji_set_boot_mode(mode);
}

void io_umount_disk_image(uint8_t ds)
{
  fuji_unmount_disk_image(ds);
}

void io_update_devices_enabled(bool *e)
{
  char i;

  for (i=0;i<6;i++)
    {
      e[i]=io_get_device_enabled_status(io_device_slot_to_device(i));
    }
}

void io_enable_device(unsigned char d)
{
  fuji_enable_device(d);
}

void io_disable_device(unsigned char d)
{
  fuji_disable_device(d);
}

// The enabled status area needs visiting. this requires a LOT of status codes to send to FN, as there's no payload to specify which device.
// And in FN it always returns true anyway, so this functionality is pretty broken at the moment.
bool io_get_device_enabled_status(unsigned char d)
{
  return true;
}

unsigned char io_device_slot_to_device(unsigned char ds)
{
  return ds;
}

void io_boot(void)
{
  #ifdef __ORCAC__
  sp_done();
	#ifndef BUILD_A2CDA
  WriteChar(0x8c);  // Clear screen
  WriteChar(0x92);  // Set 80 col
  WriteChar(0x86);  // Cursor on
  TextShutDown();
  exit(0);
	#endif

  #else
  char ostype;
  int i;

  ostype = get_ostype() & 0xF0;
  //clrscr();
  cprintf("\r\nRESTARTING...");

  // Wait for fujinet disk ii states to be ready
  for (i = 0; i < 2000; i++)
  {
    if (i % 250 == 0)
    {
      cprintf(".");
    }
  }

  if (ostype == APPLE_IIIEM)
  {
    asm("STA $C082");  // disable language card (Titan3+2)
    asm("LDA #$77");   // enable A3 primary rom
    asm("STA $FFDF");
    asm("JMP $F4EE");  // jmp to A3 reset entry
  }
  else  // Massive brute force hack that takes advantage of MMU quirk. Thank you xot.
  {
    POKE(0xC00E,0); // CLRALTCHAR

    // Make the simulated 6502 RESET result in a cold start.
    // INC $03F4
    POKE(0x100,0xEE);
    POKE(0x101,0xF4);
    POKE(0x102,0x03);

    // Make sure to not get disturbed.
    // SEI
    POKE(0x103,0x78);

    // Disable Language Card (which is enabled for all cc65 programs).
    // LDA $C082
    POKE(0x104,0xAD);
    POKE(0x105,0x82);
    POKE(0x106,0xC0);

    // Simulate a 6502 RESET, additionally do it from the stack page to make the MMU
    // see the 6502 memory access pattern which is characteristic for a 6502 RESET.
    // JMP ($FFFC)
    POKE(0x107,0x6C);
    POKE(0x108,0xFC);
    POKE(0x109,0xFF);

    asm("JMP $0100");
  }
  #endif /* __ORCAC__ */
}

bool io_get_wifi_enabled(void)
{
    return true;
}

void io_list_devs(void)
{
    sp_list_devs();
    state = HOSTS_AND_DEVICES;
}

FNStatus io_get_fuji_status(void)
{
  FNStatus status;
  fuji_status(&status);
  return status;
}

#endif /* BUILD_APPLE2 */
