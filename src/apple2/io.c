#ifdef BUILD_APPLE2
/**
 * FujiNet CONFIG for #Apple2
 *
 * I/O Routines
 */
#include "io.h"
#include <stdint.h>
#include <conio.h>

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
#define FUJICMD_STATUS 0x53

#include <string.h>
#include "sp.h"

static NetConfig nc;
static SSIDInfo ssid_response;
static AdapterConfig ac;

/* Test Fixtures, remove when actual I/O present */
// static DeviceSlot _ds[8];
// static HostSlot _hs[8];
// static char de[36][8]={
//   {"Entry 1"},
//   {"Entry 2"},
//   {"Entry 3"},
//   {"Entry 4"},
//   {"Entry 5"},
//   {"Entry 6"},
//   {"Entry 7"},
//   {"\x7F"}
// };
// static char dc=0;

void io_init(void)
{
}

uint8_t io_status(void)
{
  return 0;
}

bool io_error(void)
{
  return sp_error; 
}

uint8_t io_get_wifi_status(void)
{
  // call the SP status command and get the returned byte
  uint8_t s;

  sp_error = sp_status(sp_dest, FUJICMD_GET_WIFISTATUS);
  if (sp_error)
      return 0;
 
  return sp_payload[0];
}

NetConfig* io_get_ssid(void)
{  
  memset(&nc, 0, sizeof(nc));
  sp_error = sp_status(sp_dest, FUJICMD_GET_SSID); 
  if (!sp_error)
  {
    memcpy(&nc.ssid, sp_payload, sizeof(nc.ssid));
    memcpy(&nc.password, &sp_payload[sizeof(nc.ssid)], sizeof(nc.password));
  }
  sp_error = sp_error;
  return &nc;
}

uint8_t io_scan_for_networks(void)
{
  sp_error = sp_status(sp_dest, FUJICMD_SCAN_NETWORKS);
  if (!sp_error)
    return sp_payload[0];
  return 0;
}

SSIDInfo *io_get_scan_result(uint8_t n)
{
  sp_payload[0] = 1;
  sp_payload[1] = 0;
  sp_payload[2] = n;
  memset(ssid_response.ssid, 0, 32);
  sp_error = sp_control(sp_dest, FUJICMD_GET_SCAN_RESULT);
  if (!sp_error)
  {
    sp_error = sp_status(sp_dest, FUJICMD_GET_SCAN_RESULT);
    if (!sp_error)
    {
      memcpy(ssid_response.ssid,sp_payload,32);
      ssid_response.rssi = sp_payload[32];
    }
  }
  return &ssid_response;
}

AdapterConfig *io_get_adapter_config(void)
{
  uint16_t idx = 0;
  sp_error = sp_status(sp_dest, FUJICMD_GET_ADAPTERCONFIG);
  if (!sp_error)
  {
    memcpy(ac.ssid, sp_payload, 32);
    idx += 32;
    memcpy(ac.hostname, &sp_payload[idx], 64);
    idx += 64;
    memcpy(ac.localIP, &sp_payload[idx], 4);
    idx += 4;
    memcpy(ac.gateway, &sp_payload[idx],4);
    idx += 4;
    memcpy(ac.netmask, &sp_payload[idx], 4);
    idx += 4;
    memcpy(ac.dnsIP, &sp_payload[idx], 4);
    idx += 4;
    memcpy(ac.macAddress, &sp_payload[idx], 6);
    idx += 6;
    memcpy(ac.bssid, &sp_payload[idx], 6);
    idx += 6;
    memcpy(ac.fn_version, &sp_payload[idx], 15);
  }
  return &ac;
}

void io_set_ssid(NetConfig *nc)
{
  char idx = 0;
  sp_payload[idx++] = sizeof(*nc);
  sp_payload[idx++] = 0;
  memcpy(&sp_payload[idx], nc->ssid, sizeof(nc->ssid));
  idx += sizeof(nc->ssid);
  memcpy(&sp_payload[idx], nc->password, sizeof(nc->password));
  sp_error = sp_control(sp_dest, FUJICMD_SET_SSID);
}

char *io_get_device_filename(uint8_t ds)
{
  // TODO: implement
}

void io_create_new(uint8_t selected_host_slot,uint8_t selected_device_slot,unsigned long selected_size,char *path)
{
  // TODO: implement
}

void io_get_device_slots(DeviceSlot *d)
{
  sp_status(sp_dest, FUJICMD_READ_DEVICE_SLOTS);
  memcpy(d,sp_payload,sp_count); // 304 bytes?
}

void io_get_host_slots(HostSlot *h)
{
  sp_status(sp_dest, FUJICMD_READ_HOST_SLOTS);
  memcpy(h, sp_payload, sp_count); // 256 bytes?
}

void io_put_host_slots(HostSlot *h)
{
  sp_payload[0] = 0;
  sp_payload[1] = 1; // 256 bytes
  memcpy(&sp_payload[2], h, 256); 
  sp_error = sp_control(sp_dest, FUJICMD_WRITE_HOST_SLOTS);
}

void io_put_device_slots(DeviceSlot *d)
{
}

void io_mount_host_slot(uint8_t hs)
{
  sp_payload[0] = 1;
  sp_payload[1] = 0;
  sp_payload[2] = hs;
  sp_error = sp_control(sp_dest, FUJICMD_MOUNT_HOST);
}

void io_open_directory(uint8_t hs, char *p, char *f)
{
  char *e;
  char idx = 0;
  uint16_t s;

  // to do - copy strings into payload and figure out length
  s = 1 + strlen(p) + 1 + strlen(f) +1;
  sp_payload[idx++] = (uint8_t)(s & 0xFF);
  sp_payload[idx++] = (uint8_t)(s >> 8);
  sp_payload[idx++] = hs;

  strcpy(&sp_payload[idx++], p);
  idx += strlen(p);
  strcpy(&sp_payload[idx], f);

  sp_error = sp_control(sp_dest, FUJICMD_OPEN_DIRECTORY);
}

char *io_read_directory(uint8_t l, uint8_t a)
{
  sp_payload[0] = 2;
  sp_payload[1] = 0;
  sp_payload[2] = l;
  sp_payload[3] = a;
  sp_error = sp_control(sp_dest, FUJICMD_READ_DIR_ENTRY);
  sp_payload[0] = 0; // null string
  if (!sp_error)
    sp_error = sp_status(sp_dest, FUJICMD_READ_DIR_ENTRY);
  return sp_payload;
}

void io_close_directory(void)
{
  sp_payload[0] = 0;
  sp_payload[1] = 0;
  sp_error = sp_control(sp_dest, FUJICMD_CLOSE_DIRECTORY);
}

void io_set_directory_position(DirectoryPosition pos)
{
  sp_payload[0] = 1;
  sp_payload[1] = 0;
  sp_payload[2] = (uint8_t)pos;
  sp_error = sp_control(sp_dest, FUJICMD_SET_DIRECTORY_POSITION);
}

void io_set_device_filename(uint8_t ds, char* e)
{
}

void io_mount_disk_image(uint8_t ds, uint8_t mode)
{
}

void io_set_boot_config(uint8_t toggle)
{
}

void io_umount_disk_image(uint8_t ds)
{
}

void io_boot(void)
{
}

#endif /* BUILD_APPLE2 */
