#ifdef BUILD_RC2014
/**
 * FujiNet CONFIG for RC2014
 *
 * I/O Routines
 */

#include <conio.h> // for sleep() 
#include <stdlib.h>
#include <string.h>
#include "io.h"
#include "globals.h"
#include "fujinet.h"
#include "fujinet_device.h"

#define FUJI_DEV 0x0F
#undef MOCK_WIFI
#undef MOCK_DEVICES
#undef MOCK_HOSTS

uint8_t response[1024];
FUJINET_RC last_rc = FUJINET_RC_OK;

extern unsigned char source_path[224];
extern unsigned char path[224];

#ifdef MOCK_WIFI
AdapterConfig mock_cfg = {
        "my-test-network",
        "localhost",
        { 192, 168, 0, 100 },
        { 192, 168, 0, 1 },
        { 255, 255, 255, 0 },
        { 8, 8, 8, 8 },
        { 1, 2, 3, 4, 5, 6 },
        { 81, 82, 83, 84, 85, 86 },
        "1.0.0-rc2014"
};

SSIDInfo mock_ssid[] = {
        { "my-test-wifi", -30 },
        { "ssid 1", -40 },
        { "Wifi 2", -80 }
};
int mock_ssid_num = 3;

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
        { 7, 0, {} }
};
#endif

#ifdef MOCK_HOSTS
HostSlot mock_hosts[8] = {
        { "rc2014-apps.irata.online" },
        { {} },
        { {} },
        { {} },
        { {} },
        { {} },
        { {} },
        { {} }
};
#endif


void io_init(void)
{
  fujinet_init();
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
  return 0;
#else
  last_rc = fujinet_get_wifi_status(response);
  return response[0];
#endif
}

NetConfig* io_get_ssid(void)
{
#ifdef MOCK_WIFI
  last_rc = FUJINET_RC_OK;
  return mock_netconfig;
#else

  NetConfig* nc = (NetConfig*)response;
  last_rc = fujinet_get_ssid(nc);
  return nc;
#endif
}

unsigned char io_scan_for_networks(void)
{
#ifdef MOCK_WIFI
  last_rc = FUJINET_RC_OK;
  return mock_ssid_num;
#else
  last_rc = fujinet_scan_for_networks(response);
  return response[0];
#endif
}

SSIDInfo *io_get_scan_result(unsigned char n)
{
#ifdef MOCK_WIFI
  last_rc = FUJINET_RC_OK;
  return &mock_ssid[n];
#else
  SSIDInfo* info = (SSIDInfo*)response;
  last_rc = fujinet_get_scan_result(n, info);
  return info;
#endif
}

AdapterConfig *io_get_adapter_config(void)
{
#ifdef MOCK_WIFI
  last_rc = FUJINET_RC_OK;
  return mock_cfg;
#else
  AdapterConfig* ac = (AdapterConfig*)response;
  last_rc = fujinet_get_adapter_config(ac);
  return ac;
#endif
}

int io_set_ssid(NetConfig *nc)
{
#ifdef MOCK_WIFI
  memcpy(&mock_cfg, nc, sizeof(NetConfig));
  last_rc = FUJINET_RC_OK;
#else
  last_rc = fujinet_set_ssid(nc);
#endif
  return 0;
}

void io_get_device_slots(DeviceSlot *d)
{
#ifdef MOCK_DEVICES
  memcpy(d,mock_devices,sizeof(DeviceSlot) * 8);
  last_rc = FUJINET_RC_OK;
#else
  last_rc = fujinet_get_device_slots(d);
#endif
}

void io_get_host_slots(HostSlot *h)
{
#ifdef MOCK_DEVICES
  memcpy(h,mock_hosts,sizeof(HostSlot) * 8);
  last_rc = FUJINET_RC_OK;
#else
  last_rc = fujinet_get_host_slots(h);
#endif
}

void io_put_host_slots(HostSlot *h)
{
#ifdef MOCK_DEVICES
  memcpy(mock_hosts, h, sizeof(HostSlot) * 8);
  last_rc = FUJINET_RC_OK;
#else
  last_rc = fujinet_put_host_slots(h);
#endif
}

void io_put_device_slots(DeviceSlot *d)
{
#ifdef MOCK_DEVICES
  memcpy(mock_hosts, d, sizeof(DeviceSlot) * 8);
  last_rc = FUJINET_RC_OK;
#else
  last_rc = fujinet_put_device_slots(d);
#endif

  csleep(10);
}

void io_mount_host_slot(unsigned char hs)
{
#ifdef MOCK_DEVICES
  /* do nothing */
  last_rc = FUJINET_RC_OK;
#else
  last_rc = fujinet_mount_host_slot(hs);
#endif
}

void io_open_directory(unsigned char hs, char *p, char *f)
{
#ifdef MOCK_DEVICES
  /* do nothing */
  last_rc = FUJINET_RC_OK;
#else
  last_rc = fujinet_open_directory(hs, p, f);
#endif
}

char *io_read_directory(unsigned char l, unsigned char a)
{
#ifdef MOCK_DEVICES
  /* do nothing */
  last_rc = FUJINET_RC_OK;
#else
  last_rc = fujinet_read_directory(response, l, a);
#endif

  return response;
}

void io_close_directory(void)
{
#ifdef MOCK_DEVICES
  /* do nothing */
  last_rc = FUJINET_RC_OK;
#else
  last_rc = fujinet_close_directory();
#endif
}

void io_set_directory_position(DirectoryPosition pos)
{
#ifdef MOCK_DEVICES
  /* do nothing */
  last_rc = FUJINET_RC_OK;
#else
  last_rc = fujinet_set_directory_position(pos);
#endif
}

void io_set_device_filename(unsigned char ds, char* e)
{
#ifdef MOCK_DEVICES
  /* do nothing */
  last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
  last_rc = fujinet_set_device_filename(ds, e);
#endif
}

char *io_get_device_filename(unsigned char ds)
{
#ifdef MOCK_DEVICES
  /* do nothing */
  last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
  last_rc = fujinet_get_device_filename(ds, response);
#endif

  return response;
}

void io_create_new(unsigned char selected_host_slot,unsigned char selected_device_slot,unsigned long selected_size,char *path)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    last_rc = fujinet_create_new(selected_host_slot, selected_device_slot, selected_size, path);
#endif
}

void io_mount_disk_image(unsigned char ds, unsigned char mode)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    last_rc = fujinet_mount_disk_image(ds, mode);
#endif
}

void io_set_boot_config(unsigned char toggle)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    last_rc = fujinet_set_boot_config(toggle);
#endif
}

void io_umount_disk_image(unsigned char ds)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    last_rc = fujinet_umount_disk_image(ds);
#endif
}

void io_boot(void)
{
//  eos_init();
  last_rc = FUJINET_RC_NOT_IMPLEMENTED;
}

void io_build_directory(unsigned char ds, unsigned long numBlocks, char *v)
{
  unsigned int nb = numBlocks;

  // End volume label
  v[strlen(v)]=0x03;

  // Adjust device slot to EOS device #
  ds += 4;

  // Set up block 0 to boot right into SmartWriter
  memset(response,0,1024);
  response[0]=0xC3;
  response[1]=0xE7;
  response[2]=0xFC;
//  eos_write_block(ds,0,&response[0]);
//  eos_write_block(ds,0,&response[0]);
//  eos_write_block(ds,0,&response[0]);
//  eos_write_block(ds,0,&response[0]);
//  eos_write_block(ds,0,&response[0]);
//  eos_write_block(ds,0,&response[0]);
//  eos_write_block(ds,0,&response[0]);
//  eos_write_block(ds,0,&response[0]);

  // Write directory
//  eos_initialize_directory(ds, 1, nb, v);
//  eos_initialize_directory(ds, 1, nb, v);
  last_rc = FUJINET_RC_NOT_IMPLEMENTED;
}

bool io_get_device_enabled_status(unsigned char d)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    last_rc = fujinet_get_device_enabled_status(d + RC2014_DEVICEID_DISK, response);
#endif
    return response[0];
}

void io_enable_device(unsigned char d)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    last_rc = fujinet_enable_device(d + RC2014_DEVICEID_DISK);
#endif
}

void io_disable_device(unsigned char d)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    last_rc = fujinet_disable_device(d + RC2014_DEVICEID_DISK);
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
  return ds+4;
}

bool io_get_wifi_enabled(void)
{
	return true;
}

#endif /* BUILD_RC2014 */
