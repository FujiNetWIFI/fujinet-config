/**
 * FujiNet CONFIG for #Adam
 *
 * I/O Routines
 */

#include <eos.h>
#include <string.h>
#include "io.h"

#define FUJI_DEV 0x0F

char response[1024];
static DCB *dcb;

static void io_command_and_response(void* buf, unsigned short len)
{
  memset(response,0,sizeof(response));
  eos_write_character_device(FUJI_DEV,buf,len);
  eos_read_character_device(FUJI_DEV,response,sizeof(response));
}

void io_init(void)
{
  dcb = eos_find_dcb(FUJI_DEV);
}

unsigned char io_status(void)
{
  return dcb->status;
}

bool io_error(void)
{
  return dcb->status == 0x8C;
}

unsigned char io_get_wifi_status(void)
{
  unsigned char c=0xFA;
  
  io_command_and_response(&c,1);
  
  return response[0];
}

NetConfig* io_get_ssid(void)
{
  unsigned char c=0xFE;
  
  io_command_and_response(&c,1);

  return (NetConfig *)response;
}

unsigned char io_scan_for_networks(void)
{
  unsigned char c=0xFD;
  
  io_command_and_response(&c,1);

  return response[0];
}

SSIDInfo *io_get_scan_result(unsigned char n)
{
  unsigned char c[2]={0xFC,0x00};

  c[1] = n;

  io_command_and_response(&c,2);

  return (SSIDInfo *)response;
}

AdapterConfig *io_get_adapter_config(void)
{
  unsigned char c=0xE8;

  io_command_and_response(&c,1);

  return (AdapterConfig *)response;
}

void io_set_ssid(NetConfig *nc)
{
  unsigned char c[97]={0xFB};

  memcpy(&c[1],nc,sizeof(NetConfig));

  eos_write_character_device(FUJI_DEV,&c,sizeof(c));

}

void io_get_device_slots(DeviceSlot *d)
{
  unsigned char c=0xF2;

  io_command_and_response(&c,1);

  memcpy(d,response,304);
}

void io_get_host_slots(HostSlot *h)
{
  unsigned char c=0xF4;

  io_command_and_response(&c,1);
  
  memcpy(h,response,256);
}

void io_put_host_slots(HostSlot *h)
{
  unsigned char c[257]={0xF3};

  memcpy(&c[1],h,256);

  eos_write_character_device(FUJI_DEV,&c,sizeof(c));  
}

void io_put_device_slots(DeviceSlot *d)
{
  unsigned char c[305]={0xF1};

  memcpy(&c[1],d,304);

  eos_write_character_device(FUJI_DEV,&c,sizeof(c));
}

void io_mount_host_slot(unsigned char hs)
{
  unsigned char c[2]={0xF9,0x00};

  c[1] = hs;

  eos_write_character_device(FUJI_DEV,&c,sizeof(c));
}

void io_open_directory(unsigned char hs, char *p, char *f)
{
  char c[258]={0xF7};

  strcpy(&c[1],p);
  strcpy(c[strlen(c)],f);

  eos_write_character_device(FUJI_DEV,&c,sizeof(c));
}

char *io_read_directory(unsigned char l, unsigned char a)
{
  unsigned char c[3]={0xF6,0x00,0x00};
  c[1]=l;
  c[2]=a;
  io_command_and_response(&c,3);
  return response;
}

void io_close_directory(void)
{
  unsigned char c=0xF5;

  eos_write_character_device(FUJI_DEV,&c,sizeof(c));
}

void io_set_directory_position(DirectoryPosition pos)
{
  unsigned char c[3]={0xE4,0x00,0x00};

  memcpy(&c[1],&pos,sizeof(DirectoryPosition));

  eos_write_character_device(FUJI_DEV,&c,sizeof(c));
}

void io_set_device_filename(unsigned char ds, char* e)
{
  char c[258]={0xE2,0x00};
  
  c[1] = ds;

  strcpy(&c[2],e);

  eos_write_character_device(FUJI_DEV,&c,sizeof(c));
}

void io_mount_disk_image(unsigned char ds, unsigned char mode)
{
  char c[3]={0xF8,0x00,0x00};
  c[1]=ds;
  c[2]=mode;

  eos_write_character_device(FUJI_DEV,&c,sizeof(c));
}

void io_set_boot_config(unsigned char toggle)
{
  char c[2]={0xD9,0x00};
  c[1]=toggle;

  eos_write_character_device(FUJI_DEV,&c,sizeof(c));
}
