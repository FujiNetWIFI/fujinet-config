#ifdef BUILD_ADAM
/**
 * FujiNet CONFIG for #Adam
 *
 * I/O Routines
 */

#include "../typedefs.h"
#include <conio.h> // for sleep()
#include <stdlib.h>
#include <eos.h>
#include <string.h>

char response[1024];
static DCB *dcb = NULL;

extern unsigned char source_path;
extern unsigned char path;

static void io_command_and_response(void* buf, unsigned short len)
{
  if (!dcb)
    dcb = eos_find_dcb(FUJI_DEV);
    
  memset(response,0,sizeof(response));
  eos_write_character_device(FUJI_DEV,buf,len);
  eos_read_character_device(FUJI_DEV,response,sizeof(response));
}

#ifdef UNUSED
void io_init(void)
{
  dcb = eos_find_dcb(FUJI_DEV);
}
#endif /* UNUSED */

#ifdef UNUSED
unsigned char io_status(void)
{
  return dcb->status;
}
#endif /* UNUSED */

bool fuji_error(void)
{
  return dcb->status == 0x8C;
}

bool fuji_get_wifi_status(uint8_t *status)
{
  unsigned char c=0xFA;

  sleep(1);
  io_command_and_response(&c,1);
  *status = response[0];
  return;
}

bool fuji_get_ssid(NetConfig *net_config)
{
  unsigned char c=0xFE;

  io_command_and_response(&c,1);
  memcpy(net_config, response, sizeof(*net_config));
  return true;
}

bool fuji_scan_for_networks(uint8_t *count)
{
  unsigned char c=0xFD;

  io_command_and_response(&c,1);

  *count = response[0];
  return true;
}

bool fuji_get_scan_result(uint8_t n, SSIDInfo *ssid_info)
{
  unsigned char c[2]={0xFC,0x00};

  c[1] = n;

  io_command_and_response(&c,2);

  memcpy(ssid_info, response, sizeof(*ssid_info));
}

#ifdef UNUSED
AdapterConfig *io_get_adapter_config(void)
{
  unsigned char c=0xE8;

  io_command_and_response(&c,1);

  return (AdapterConfig *)response;
}
#endif /* UNUSED */

bool fuji_get_adapter_config_extended(AdapterConfigExtended *acx)
{
  unsigned char c=0xC4;

  io_command_and_response(&c,1);

  memcpy(acx, response, sizeof(*acx));
  return true;
}

bool fuji_set_ssid(NetConfig *nc)
{
  unsigned char c[98]={0xFB};

  memcpy(&c[1],nc,sizeof(NetConfig));

  eos_write_character_device(FUJI_DEV,&c,sizeof(c));
  return true;
}

bool fuji_get_device_slots(DeviceSlot *d, size_t size)
{
  unsigned char c=0xF2;

  io_command_and_response(&c,1);

  memcpy(d,response,304);
  return true;
}

bool fuji_get_host_slots(HostSlot *h, size_t size)
{
  unsigned char c=0xF4;

  io_command_and_response(&c,1);

  memcpy(h,response,256);
  return true;
}

bool fuji_put_host_slots(HostSlot *h, size_t size)
{
  unsigned char c[257]={0xF3};

  memcpy(&c[1],h,256);

  eos_write_character_device(FUJI_DEV,&c,sizeof(c));
  return true;
}

bool fuji_put_device_slots(DeviceSlot *d, size_t size)
{
  unsigned char c[305]={0xF1};

  memcpy(&c[1],d,304);

  eos_write_character_device(FUJI_DEV,&c,sizeof(c));
  csleep(10);
}

bool fuji_mount_host_slot(uint8_t hs)
{
  unsigned char c[2]={0xF9,0x00};

  c[1] = hs;

  eos_write_character_device(FUJI_DEV,&c,sizeof(c));
  return true;
}

bool fuji_open_directory2(unsigned char hs, char *p, char *f)
{
  char c[258];
  char *e;

  memset(&c,0,258);
  c[0]=0xF7;
  c[1]=hs;
  strcpy(&c[2],p);
  e=&c[2];

  if (f[0]!=0x00)
    {
      while (*e != 0x00)
	e++;

      e++;
      strcpy(e,f);
    }

  eos_write_character_device(FUJI_DEV,&c,sizeof(c));
  return true;
}

bool fuji_read_directory(unsigned char l, unsigned char a, char *buffer)
{
  unsigned char c[3]={0xF6,0x00,0x00};
  c[1]=l;
  c[2]=a;
  io_command_and_response(&c,3);
  memcpy(buffer, response, l);
  return true;
}

bool fuji_close_directory(void)
{
  unsigned char c=0xF5;

  eos_write_character_device(FUJI_DEV,&c,sizeof(c));
  return true;
}

bool fuji_set_directory_position(DirectoryPosition pos)
{
  unsigned char c[3]={0xE4,0x00,0x00};

  memcpy(&c[1],&pos,sizeof(DirectoryPosition));

  eos_write_character_device(FUJI_DEV,&c,sizeof(c));
  return true;
}

bool fuji_set_device_filename(uint8_t mode, uint8_t hs, uint8_t ds, char *e)
{
  char c[258]={0xE2,0x00};

  c[1] = ds;

  strcpy(&c[2],e);

  eos_write_character_device(FUJI_DEV,&c,sizeof(c));
  return true;
}

bool fuji_get_device_filename(uint8_t ds, char *buffer)
{
  char c[2]={0xDA,0x00};

  c[1] = ds;

  io_command_and_response(&c,2);
  strcpy(buffer, response);
  return true;
}

bool fuji_mount_disk_image(uint8_t ds, uint8_t mode)
{
  char c[3]={0xF8,0x00,0x00};
  c[1]=ds;
  c[2]=mode;

  eos_write_character_device(FUJI_DEV,&c,sizeof(c));
  return true;
}

bool fuji_set_boot_config(unsigned char toggle)
{
  char c[2]={0xD9,0x00};
  c[1]=toggle;

  eos_write_character_device(FUJI_DEV,&c,sizeof(c));
  return true;
}

bool fuji_unmount_disk_image(uint8_t ds)
{
  char c[2]={0xE9,0x00};
  c[1]=ds;

  eos_write_character_device(FUJI_DEV,&c,sizeof(c));
  return true;
}

#ifdef UNUSED
bool io_get_device_enabled_status(unsigned char d)
{
  struct
  {
    unsigned char cmd;
    unsigned char dev;
  } ds;

  ds.cmd = 0xD1; // Get Device status
  ds.dev = d;

  eos_write_character_device(FUJI_DEV,ds,sizeof(ds));
  eos_read_character_device(FUJI_DEV,response,sizeof(response));

  return response[0];
}
#endif /* UNUSED */

bool fuji_enable_device(uint8_t d)
{
  struct
  {
    unsigned char cmd;
    unsigned char dev;
  } ed;

  ed.cmd = 0xD5;
  ed.dev = d;

  eos_write_character_device(FUJI_DEV,ed,sizeof(ed));
  return true;
}

bool fuji_disable_device(uint8_t d)
{
  struct
  {
    unsigned char cmd;
    unsigned char dev;
  } dd;

  dd.cmd = 0xD4;
  dd.dev = d;

  eos_write_character_device(FUJI_DEV,dd,sizeof(dd));
  return true;
}

#ifdef UNUSED
void io_update_devices_enabled(bool *e)
{
  char i;

  for (i=0;i<4;i++)
    {
      e[i]=io_get_device_enabled_status(fuji_device_slot_to_device(i));
    }
}
#endif /* UNUSED */

bool fuji_copy_file(uint8_t source_slot, uint8_t destination_slot, char *copy_spec)
{
  char cf[259]={0xD8,0x00,0x00};
  DCB *dcb = NULL;
  
  cf[1]=source_slot;
  cf[2]=destination_slot;
  strcpy(&cf[3],copy_spec);

  eos_write_character_device(FUJI_DEV,cf,sizeof(cf));

  while (eos_request_device_status(FUJI_DEV,dcb) != 0x80);
  return true;
}

#ifdef UNUSED
unsigned char io_device_slot_to_device(unsigned char ds)
{
  return ds+4;
}
#endif /* UNUSED */

bool fuji_get_wifi_enabled(void)
{
	return true;
}

#endif /* BUILD_ADAM */
