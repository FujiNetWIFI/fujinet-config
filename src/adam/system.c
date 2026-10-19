#include "../system.h"
#include "../globals.h"
#include <eos.h>
#include <string.h>

#define FUJI_DEV 0x0F

// eos.h: strip the high bit of an EOS result; 0 is success
#define EOS_OK(r) (((r) & 0x7F) == EOS_ERR_NONE)

char response[1024]; // one EOS block; system_build_directory writes whole blocks from it

void system_boot(void)
{
  eos_init();
}

void system_create_new(unsigned char selected_host_slot,unsigned char selected_device_slot,unsigned long selected_size,char *path)
{
  char nd[263]={0xE7,0x00,0x00,0x00,0x00,0x00,0x00};
  char *c = &nd[3];
  unsigned long *l = (unsigned long *)c;

  nd[1]=selected_host_slot;
  nd[2]=selected_device_slot;
  *l=selected_size;
  strcpy(&nd[7],path);

  eos_write_character_device(FUJI_DEV,&nd,sizeof(nd));
}

bool system_build_directory(unsigned char ds, unsigned long numBlocks, char *v)
{
  unsigned int db = 1; // 1 directory block by default
  unsigned int nb = (unsigned short)numBlocks;
  DCB *dcb = NULL;
  unsigned char s=0;
  
  // EOS only finds devices at boot; rescan so it sees the disk just mounted
  eos_scan_for_devices();

  // End volume label
  v[strlen(v)]=0x03;

  // Adjust device slot to EOS device #
  ds += 4;

  memset(response,0,sizeof(response));
  
  for (unsigned long i=0;i<db;i++)
    {
      if (!EOS_OK(eos_write_block(ds,i+1,&response[0])))
        return false;
    }
    
  memset(response,0,sizeof(response));
  response[0]=0xC3;
  response[1]=0xE7;
  response[2]=0xFC;

  if (!EOS_OK(eos_write_block(ds,0UL,&response[0])))
    return false;

  if (numBlocks>719)
    db=6;
  else if (numBlocks>319)
    db=3;
  else if (numBlocks>160)
    db=2;
  else
    db=1;

  return EOS_OK(eos_initialize_directory(ds,db,nb,v));
  
}

