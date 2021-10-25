/**
 * #FUJINET CONFIG
 * Diskulator Hosts/Devices
 */

#include <msx.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <eos.h>
#include "diskulator_hosts.h"
#include "fuji_adam.h"
#include "fuji_typedefs.h"
#include "input.h"
#include "smartkeys.h"
#include "bar.h"

char empty[]="Empty";
char fn[256];

typedef enum _substate
  {
   HOSTS,
   DEVICES,
   DONE
  } SubState;

#define HOST_SLOTS_Y 1

/**
 * Display Hosts Slots
 */
void diskulator_hosts_display_host_slots(unsigned char y, Context *context)
{
  msx_color(1,15,7);
  for (int i=0; i<8; i++)
    {
      gotoxy(1,y+i);
      if (context->hostSlots.host[i][0] != 0x00)
	{
	  cputs(context->hostSlots.host[i]);
	}
      else
	{
	  cputs(empty);
	}
    }
}

/**
 * Display device slots
 */
void diskulator_hosts_display_device_slots(unsigned char y, Context *context)
{
  unsigned char d[6];
  msx_color(1,15,7);

  for (int i=0;i<4;i++)
    {
      gotoxy(1,y+i);
      if (context->deviceSlots.slot[i].file[0] != 0x00)
	cprintf("%s",context->deviceSlots.slot[i].file);
      else
	cprintf("%s",empty);
    }  
}

/**
 * Keys for Hosts mode
 */
void diskulator_hosts_keys_hosts(void)
{
}

/**
 * Keys for Devices mode
 */
void diskulator_hosts_keys_devices(void)
{
}

AdapterConfig ac;

/**
 * Diskulator hosts setup
 */
void diskulator_hosts_setup(Context *context)
{
  memset(&ac,0,sizeof(ac));
  
  clrscr();

  msx_vfill(MODE2_ATTR+4096,0xF9,256);
  msx_vfill(MODE2_ATTR+4352,0x1F,256);
  msx_vfill(MODE2_ATTR+4608,0xF9,256);
  msx_vfill(MODE2_ATTR+4864,0x1F,256);
  fuji_adamnet_read_adapter_config(&ac);
  
  // Write adapter config
  msx_color(15,9,7);
  gotoxy(0,16); cprintf("HOSTNAME\n");
  gotoxy(0,18); cprintf("ADDRESS\n",ac.localIP[0],ac.localIP[1],ac.localIP[2],ac.localIP[3]);
  msx_color(1,15,7);
  gotoxy(0,17); cprintf("%s\n",ac.hostname);
  gotoxy(0,19); cprintf("%u.%u.%u.%u\n",ac.localIP[0],ac.localIP[1],ac.localIP[2],ac.localIP[3]);
  
  // Paint headers
  msx_color(15,5,7);
  gotoxy(22,0); cputs("HOST SLOTS");
  gotoxy(22,10); cputs("DISK SLOTS");

  for (int i=0;i<8;i++)
    {
      gotoxy(0,1+i); putchar(0x31+i);
    }

  for (int i=0;i<4;i++)
    {
      gotoxy(0,11+i); putchar(0x31+i);
    }
  
  msx_vfill(MODE2_ATTR,0xF4,256);
  msx_vfill(MODE2_ATTR+2560,0xF5,256);

  // Paint content boxes
  msx_vfill(MODE2_ATTR+256,0x1F,2048);
  msx_vfill(MODE2_ATTR+2816,0x1F,1024);

  // Paint columns
  msx_vfill_v(MODE2_ATTR+256,0xF4,64);
  msx_vfill_v(MODE2_ATTR+2816,0xF5,32);

  memset(context->hostSlots,0,sizeof(HostSlots));
  memset(context->deviceSlots,0,sizeof(DeviceSlots));
  
  // Load host slots
  fuji_adamnet_read_host_slots(&context->hostSlots);
  diskulator_hosts_display_host_slots(1,context);

  // Load device slots
  fuji_adamnet_read_device_slots(&context->deviceSlots);
  diskulator_hosts_display_device_slots(11,context);
  
}

/**
 * Edit a host slot
 */
void diskulator_hosts_edit_host_slot(unsigned char i, Context *context)
{
  gotoxy(1,i+1); cgets(context->hostSlots.host[i]);
  fuji_adamnet_write_host_slots(context->hostSlots);
}

/**
 * Eject image from device slot
 */
void diskulator_hosts_eject_device_slot(unsigned char i, unsigned char pos, Context *context)
{
  char tmp[2]={0,0};

}

/**
 * Set device slot to read or write
 */
void diskulator_hosts_set_device_slot_mode(unsigned char i, unsigned char mode, Context* context)
{
  unsigned char tmp_hostSlot;
  unsigned char tmp_file[FILE_MAXLEN];

}

/**
 * Diskulator interactive - hosts
 */
void diskulator_hosts_hosts(Context *context, SubState *new_substate)
{
  unsigned char k;
  unsigned char i=0;

  bar_set(0,1,8,0);

  smartkeys_display(NULL,NULL,NULL," CONFIG","  EDIT\n  SLOT","  BOOT");
  smartkeys_status(" ARROWS OR [1-8]\n [RETURN] TO SELECT FILES\n [TAB] FOR DISK SLOTS\n");
  
  while (*new_substate==HOSTS)
    {
      k=input_ucase();

      switch(k)
        {
	case 0x09: // TAB
	  *new_substate=DEVICES;
	  bar_clear();
	  break;
        case 0x0d: // RETURN
          if (context->hostSlots.host[i][0]==0x00 || (context->net_connected == false && strcmp(context->hostSlots.host[i],"SD") != 0)) // empty host slot?
            break; // do nothing
	  
          context->state=DISKULATOR_SELECT;
          context->host_slot=i;
          fuji_adamnet_mount_host(context->host_slot,&context->hostSlots);
	  *new_substate=DONE;
          break;
	case 0x84:
	  *new_substate=DONE;
	  context->state=DISKULATOR_INFO;
	  break;
	case 0x85:
	  diskulator_hosts_edit_host_slot(bar_get(),context);
	  break;
	case 0xA0:
	  bar_up();
	  break;
	case 0xA2:
	  bar_down();
	  break;
        }
    }
  
}

/**
 * Diskulator interactive - device slots
 */
void diskulator_hosts_devices(Context *context, SubState *new_substate)
{
  unsigned char k;
  unsigned char i=0;

  bar_set(10,1,4,0);
  
  smartkeys_display(NULL,NULL,NULL," CONFIG","  EDIT\n  SLOT","  BOOT");
  smartkeys_status(" ARROWS OR [1-4]\n [CLEAR] TO EJECT\n [TAB] FOR HOST SLOTS\n");

  while (*new_substate==DEVICES)
    {
      k=input_ucase();

      switch(k)
        {
	case 0x09: // TAB
	  *new_substate=HOSTS;
	  bar_clear();
	  break;
        case 0x0d: // RETURN
          break;	  
	case 0x85:
	  diskulator_hosts_edit_host_slot(bar_get(),context);
	  break;
	case 0xA0:
	  bar_up();
	  break;
	case 0xA2:
	  bar_down();
	  break;
        }
    }
}

/**
 * Clear file context
 */
void diskulator_hosts_clear_file_context(Context *context)
{
  memset(context->directory,0,sizeof(context->directory));
  memset(context->filter,0,sizeof(context->filter));
  context->dir_page=0;
}

/**
 * Connect wifi State
 */
State diskulator_hosts(Context *context)
{
  SubState ss=HOSTS;

  diskulator_hosts_setup(context);
  diskulator_hosts_clear_file_context(context);

  eos_end_read_keyboard();
  eos_start_read_keyboard();
  
  while (ss != DONE)
    {
      switch(ss)
              {
              case HOSTS:
                diskulator_hosts_hosts(context,&ss);
                break;
              case DEVICES:
                diskulator_hosts_devices(context,&ss);
                break;
              }
    }

  return context->state;
}

