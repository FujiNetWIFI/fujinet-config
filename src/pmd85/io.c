#ifdef BUILD_PMD85
/**
 * FujiNet CONFIG for #Adam
 *
 * I/O Routines
 */

#include <conio.h> // for sleep() 
#include <stdlib.h>
#include <string.h>
#include "io.h"

#define MOCK_WIFI
#define MOCK_DEVICES
#define MOCK_HOSTS

#define FUJINET_TIMEOUT 15000

typedef enum {
    FUJINET_RC_OK,
    FUJINET_RC_NOT_IMPLEMENTED,
    FUJINET_RC_NOT_SUPPORTED,
    FUJINET_RC_INVALID,
    FUJINET_RC_TIMEOUT,
    FUJINET_RC_NO_ACK,
    FUJINET_RC_NO_COMPLETE,
} FUJINET_RC;

uint8_t response[256];
FUJINET_RC last_rc = FUJINET_RC_OK;


#ifdef MOCK_WIFI
// AdapterConfig mock_cfg = {
//         "my-test-network",
//         "localhost",
//         { 192, 168, 0, 100 },
//         { 192, 168, 0, 1 },
//         { 255, 255, 255, 0 },
//         { 8, 8, 8, 8 },
//         { 1, 2, 3, 4, 5, 6 },
//         { 81, 82, 83, 84, 85, 86 },
//         "1.0.0-rc2014"
// };
AdapterConfigExtended mock_cfg = {
        "mock wifi test",
        "localhost",
        { 192, 168, 0, 100 },
        { 192, 168, 0, 1 },
        { 255, 255, 255, 0 },
        { 8, 8, 8, 8 },
        { 1, 2, 3, 4, 5, 6 },
        { 81, 82, 83, 84, 85, 86 },
        "v0.0-pmd85mock",
        "192.168.0.100",
        "192.168.0.1",
        "255.255.255.0",
        "8.8.8.8",
        "01:02:03:04:05:06",
        "81:82:83:84:85:86"
};

SSIDInfo mock_ssid[] = {
        { "mock wifi test", -30 },
        { "wifi 2", -50 },
        { "ssid 1", -60 },
        { "Wifi 3", -80 }
};
int mock_ssid_num = 4;

NetConfig mock_netconfig = {
        "my-test-wifi", "password"
};
#endif

#ifdef MOCK_DEVICES
DeviceSlot mock_devices[8] = {
        { 0, 1, "TEST.DSK" },
        { 1, 0, {} },
        { 2, 0, {} },
        { 3, 0, {} },
        { 4, 0, {} },
        { 5, 0, {} },
        { 6, 0, {} },
        { 7, 0, {} },
};
#endif

#ifdef MOCK_HOSTS
HostSlot mock_hosts[8] = {
        { "SD" },
        { "fujinet.test.offline" },
        { "pmd85-apps.irata.online" },
        { {} },
        { {} },
        { {} },
        { {} },
        { {} }
};
#endif

#ifdef MOCK_HOSTS
#define MOCK_FILES_NUM 40
static unsigned short mock_files = 0;
#endif

static void io_command_and_response(void* buf, unsigned short len)
{
}

void io_init(void)
{
  // fujinet_init();
}

unsigned char io_status(void)
{
  return last_rc;
}

bool io_error(void)
{
  return last_rc != FUJINET_RC_OK;
}

unsigned char io_get_wifi_status(void)
{
  sleep(1);

#ifdef MOCK_WIFI
  last_rc = FUJINET_RC_OK;
  return 3;
#else
  // last_rc = fujinet_get_wifi_status(response);
  // return response[0];
  return 0;
#endif
}

NetConfig* io_get_ssid(void)
{
#ifdef MOCK_WIFI
  last_rc = FUJINET_RC_OK;
  return mock_netconfig;
#else

  // NetConfig* nc = (NetConfig*)response;
  // last_rc = fujinet_get_ssid(nc);
  // return nc;
  return NULL;
#endif
}

unsigned char io_scan_for_networks(void)
{
  sleep(1);
#ifdef MOCK_WIFI
  last_rc = FUJINET_RC_OK;
  return mock_ssid_num;
#else
  // last_rc = fujinet_scan_for_networks(response);
  // return response[0];
  return 0;
#endif
}

SSIDInfo *io_get_scan_result(unsigned char n)
{
#ifdef MOCK_WIFI
  last_rc = FUJINET_RC_OK;
  return &mock_ssid[n];
#else
  // SSIDInfo* info = (SSIDInfo*)response;
  // last_rc = fujinet_get_scan_result(n, info);
  // return info;
  return NULL;
#endif
}

AdapterConfigExtended *io_get_adapter_config(void)
{
#ifdef MOCK_WIFI
  last_rc = FUJINET_RC_OK;
  return mock_cfg;
#else
  // AdapterConfig* ac = (AdapterConfig*)response;
  // last_rc = fujinet_get_adapter_config(ac);
  // return ac;
  return NULL;
#endif
}

int io_set_ssid(NetConfig *nc)
{
#ifdef MOCK_WIFI
  memcpy(&mock_netconfig, nc, sizeof(NetConfig));
  memcpy(&mock_cfg.ssid, nc->ssid, sizeof(mock_cfg.ssid));
  last_rc = FUJINET_RC_OK;
#else
  // last_rc = fujinet_set_ssid(nc);
#endif
  return 0;
}

void io_get_device_slots(DeviceSlot *d)
{
#ifdef MOCK_DEVICES
  memcpy(d,mock_devices,sizeof(DeviceSlot) * 8);
  last_rc = FUJINET_RC_OK;
#else
  // last_rc = fujinet_get_device_slots(d);
#endif
  for (int i = 0; i < 8; i++)
    d[i].file[FILE_MAXLEN-1] = '\0';
}

void io_get_host_slots(HostSlot *h)
{
#ifdef MOCK_HOSTS
  memcpy(h,mock_hosts,sizeof(HostSlot) * 8);
  last_rc = FUJINET_RC_OK;
#else
  // last_rc = fujinet_get_host_slots(h);
#endif
}

void io_put_host_slots(HostSlot *h)
{
#ifdef MOCK_HOSTS
  memcpy(mock_hosts, h, sizeof(HostSlot) * 8);
  last_rc = FUJINET_RC_OK;
#else
  // last_rc = fujinet_put_host_slots(h);
#endif
}

void io_put_device_slots(DeviceSlot *d)
{
#ifdef MOCK_DEVICES
  memcpy(mock_devices, d, sizeof(DeviceSlot) * 8);
  last_rc = FUJINET_RC_OK;
#else
  // last_rc = fujinet_put_device_slots(d);
#endif

  csleep(10);
}

void io_mount_host_slot(unsigned char hs)
{
#ifdef MOCK_HOSTS
  /* do nothing */
  last_rc = FUJINET_RC_OK;
#else
  // last_rc = fujinet_mount_host_slot(hs);
#endif
}

void io_open_directory(unsigned char hs, char *p, char *f)
{
#ifdef MOCK_HOSTS
  /* do nothing */
  last_rc = FUJINET_RC_OK;
  mock_files = 0;
#else
  // last_rc = fujinet_open_directory(hs, p, f);
#endif
}

char *io_read_directory(unsigned char l, unsigned char a)
{
  struct _additl_info
	{
		unsigned char year;
		unsigned char month;
		unsigned char day;
		unsigned char hour;
		unsigned char min;
		unsigned char sec;
		unsigned long size;
		unsigned char flags;
		unsigned char type;
		char filename[0];
	} *info = (struct _additl_info *)response;

#ifdef MOCK_HOSTS
  char *fn = response;
  if (a & 0x80)
    fn = &info->filename[0];

  if (mock_files < MOCK_FILES_NUM)
  {
    mock_files++;
    if (a & 0x80) 
    {
      info->year = 84 + mock_files;
      info->month = 8;
      info->day = 11;
      info->hour = 17;
      info->min = 15;
      info->sec = 4;
      info->size = ((unsigned long)(mock_files) << 12) | mock_files;
      info->flags = 0;
      info->type = 0;
    }
    // test some long file names
    if (mock_files >= 20 && mock_files <= 25)
    {
      unsigned char ll = 2*mock_files;
      memset(fn, mock_files + ('A' - 20), ll);
      fn[ll] = '\0';
      if (ll > l)
      {
        fn[l] = '\0';
        if (a &0x80)
          info->flags |= 0x02;
      }
    }
    else
    {
      sprintf(fn, "mock-%d.mrm", mock_files);
    }
  }
  else
    sprintf(response, "\x7F\x7F"); // end of dir

  last_rc = FUJINET_RC_OK;
#else
  // last_rc = fujinet_read_directory(response, l, a);
#endif

  return response;
}

void io_close_directory(void)
{
#ifdef MOCK_HOSTS
  /* do nothing */
  last_rc = FUJINET_RC_OK;
  mock_files = 0;
#else
  // last_rc = fujinet_close_directory();
#endif
}

void io_set_directory_position(DirectoryPosition pos)
{
#ifdef MOCK_HOSTS
  mock_files = pos;
  last_rc = FUJINET_RC_OK;
#else
  // last_rc = fujinet_set_directory_position(pos);
#endif
}

void io_set_device_filename(unsigned char ds, char* e)
{
#ifdef MOCK_DEVICES
  /* do nothing */
  last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
  // last_rc = fujinet_set_device_filename(ds, e);
#endif
}

char *io_get_device_filename(unsigned char ds)
{
#ifdef MOCK_DEVICES
  /* do nothing */
  last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
  // last_rc = fujinet_get_device_filename(ds, response);
#endif

  return response;
}

void io_create_new(unsigned char selected_host_slot,unsigned char selected_device_slot,unsigned long selected_size,char *path)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    // last_rc = fujinet_create_new(selected_host_slot, selected_device_slot, selected_size, path);
#endif
}

void io_mount_disk_image(unsigned char ds, unsigned char mode)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    // last_rc = fujinet_mount_disk_image(ds, mode);
#endif
}

void io_set_boot_config(unsigned char toggle)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    // last_rc = fujinet_set_boot_config(toggle);
#endif
}

void io_umount_disk_image(unsigned char ds)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    // last_rc = fujinet_umount_disk_image(ds);
#endif
}

void io_boot(void)
{
  // last_rc = FUJINET_RC_NOT_IMPLEMENTED;
}

// The enabled status area needs visiting. this requires a LOT of status codes to send to FN, as there's no payload to specify which device.
// And in FN it always returns true anyway, so this functionality is pretty broken at the moment.
bool io_get_device_enabled_status(unsigned char d)
{
#ifdef MOCK_DEVICES
    last_rc = FUJINET_RC_OK;
    return true;
#else
    last_rc = fujinet_get_device_enabled_status(d + RC2014_DEVICEID_DISK, response);
    return response[0];
#endif
}

void io_enable_device(unsigned char d)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    // last_rc = fujinet_enable_device(d + RC2014_DEVICEID_DISK);
#endif
}

void io_disable_device(unsigned char d)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    // last_rc = fujinet_disable_device(d + RC2014_DEVICEID_DISK);
#endif
}

void io_update_devices_enabled(bool *e)
{
  char i;

  for (i=0;i<4;i++)
  {
    e[i]=io_get_device_enabled_status(io_device_slot_to_device(i));
  }
}

void io_copy_file(unsigned char source_slot, unsigned char destination_slot)
{
  char cf[259]={0xD8,0x00,0x00};
  
  cf[1]=source_slot;
  cf[2]=destination_slot;
  strcpy(&cf[3],copySpec);
  
//  eos_write_character_device(FUJI_DEV,cf,sizeof(cf));
  last_rc = FUJINET_RC_NOT_IMPLEMENTED;
}

unsigned char io_device_slot_to_device(unsigned char ds)
{
	return ds;
}

bool io_get_wifi_enabled(void)
{
	return true;
}

#endif /* BUILD_PMD85 */
