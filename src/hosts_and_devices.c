/**
 * FujiNet for #Adam configuration program
 *
 * Hosts and Devices
 */

#include <string.h>
#include <eos.h>
#include "hosts_and_devices.h"
#include "die.h"

#ifdef BUILD_ADAM
#include "adam/globals.h"
#include "adam/fuji_typedefs.h"
#include "adam/io.h"
#include "adam/screen.h"
#include "adam/input.h"
#include "adam/bar.h"
#endif /* BUILD_ADAM */

DeviceSlot deviceSlots[8];
HostSlot hostSlots[8];
char selected_host_slot;
char selected_host_name[32];

extern bool quick_boot;

static enum 
{
   HOSTS,
   DEVICES,
   DONE
} subState;

void hosts_and_devices_edit_host_slot(unsigned char i)
{
  unsigned char o;

  if (strlen(hostSlots[i])==0)
    {
      screen_hosts_and_devices_clear_host_slot(i);
      o=0;
    }
  else
    o=strlen(hostSlots[i]);

  screen_hosts_and_devices_edit_host_slot(i);
  input_line_hosts_and_devices_host_slot(i,o,hostSlots[i]);

  if (strlen(hostSlots[i])==0)
    screen_hosts_and_devices_host_slot_empty(i);
  
  io_put_host_slots(&hostSlots[0]);
  screen_hosts_and_devices_hosts();
}

void hosts_and_devices_hosts(void)
{
  char k=0;

  screen_hosts_and_devices_hosts();
  while (subState==HOSTS)
    {
      k=input();
      switch(k)
	{
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	  bar_jump(k-0x31);
	  break;
	case 0x09:
	  subState=DEVICES;
	  break;
	case 0x0d:
	  selected_host_slot=bar_get();
	  strcpy(selected_host_name,hostSlots[selected_host_slot]);
	  subState=DONE;
	  state=SELECT_FILE;
	  break;
	case 0x84:
	  subState=DONE;
	  state=SHOW_INFO;
	  break;
	case 0x85:
	  hosts_and_devices_edit_host_slot(bar_get());
	  k=0;
	  subState=HOSTS;
	  break;
	case 0x86:
	  subState=DONE;
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

void hosts_and_devices_eject(unsigned char ds)
{
  io_umount_disk_image(ds);
  memset(deviceSlots[ds].file,0,FILE_MAXLEN);
  deviceSlots[ds].hostSlot=0xFF;
  io_put_device_slots(&deviceSlots[0]);
  screen_hosts_and_devices_eject(ds);
}

void hosts_and_devices_devices(void)
{
  char k=0;

  screen_hosts_and_devices_devices();
  while (subState==DEVICES)
    {
      k=input();
      switch(k)
	{
	case '1':
	case '2':
	case '3':
	case '4':
	  bar_jump(k-0x31);
	  break;
	case 0x09:
	  subState=HOSTS;
	  break;
	case 0x84:
	  hosts_and_devices_eject(bar_get());
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

void hosts_and_devices_done(void)
{
    for (int i=0;i<4;i++)
    {
      if (deviceSlots[i].hostSlot != 0xFF)
	{
	  io_mount_host_slot(deviceSlots[i].hostSlot);
	  io_mount_disk_image(i,deviceSlots[i].mode);
	}
    }
    
    io_set_boot_config(0); // disable config
    eos_init(); // and reboot.
}

void hosts_and_devices(void)
{
  if (quick_boot==true)
    subState=DONE;
  else
    subState=HOSTS;
  
  io_get_host_slots(&hostSlots[0]);
  io_get_device_slots(&deviceSlots[0]);
  screen_hosts_and_devices(&hostSlots[0],&deviceSlots[0]);

  while (state == HOSTS_AND_DEVICES)
    {
      switch(subState)
	{
	case HOSTS:
	  hosts_and_devices_hosts();
	  break;
	case DEVICES:
	  hosts_and_devices_devices();
	  break;
	case DONE:
	  hosts_and_devices_done();
	  break;
	}
    }
}
