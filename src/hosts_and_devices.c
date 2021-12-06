/**
 * FujiNet for #Adam configuration program
 *
 * Hosts and Devices
 */

#include <string.h>
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

#ifdef BUILD_APPLE2
#include "apple2/globals.h"
#include "apple2/fuji_typedefs.h"
#include "apple2/io.h"
#include "apple2/screen.h"
#include "apple2/input.h"
#include "apple2/bar.h"
#endif /* BUILD_APPLE2 */

#ifdef BUILD_C64
#include "c64/globals.h"
#include "c64/fuji_typedefs.h"
#include "c64/io.h"
#include "c64/screen.h"
#include "c64/input.h"
#include "c64/bar.h"
#endif /* BUILD_C64 */

DeviceSlot deviceSlots[8];
HostSlot hostSlots[8];
char selected_host_slot;
char selected_device_slot;
char selected_host_name[32];

extern bool quick_boot;

static enum 
{
   HD_HOSTS,
   HD_DEVICES,
   HD_DONE
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
  while (subState==HD_HOSTS)
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
	  bar_clear(false);
	  subState=HD_DEVICES;
	  break;
	case 0x0d:
	  selected_host_slot=bar_get();
	  strcpy(selected_host_name,hostSlots[selected_host_slot]);
	  subState=HD_DONE;
	  state=SELECT_FILE;
	  break;
	case 0x1b:
	  quit();
	  break;
	case 0x84:
	  subState=HD_DONE;
	  state=SHOW_INFO;
	  break;
	case 0x85:
	  hosts_and_devices_edit_host_slot(bar_get());
	  bar_clear(false);
	  bar_jump(selected_host_slot);
	  k=0;
	  subState=HD_HOSTS;
	  break;
	case 0x86:
	  subState=HD_DONE;
	  break;
	case 0xA0:
	  bar_up();
	  selected_host_slot=bar_get();
	  break;
	case 0xA2:
	  bar_down();
	  selected_host_slot=bar_get();
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
  io_get_device_slots(&deviceSlots[0]);
  screen_hosts_and_devices_eject(ds);
}

void hosts_and_devices_devices(void)
{
  char k=0;

  screen_hosts_and_devices_devices();
  while (subState==HD_DEVICES)
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
	  bar_clear(false);
	  subState=HD_HOSTS;
	  break;
	case 0x84:
	  hosts_and_devices_eject(bar_get());
	  break;
	case 0xA0:
	  bar_up();
	  selected_device_slot=bar_get();
	  break;
	case 0xA2:
	  bar_down();
	  selected_device_slot=bar_get();
	  break;
	}
    }
}

void hosts_and_devices_done(void)
{
  char i;
  
    for (i=0;i<4;i++)
    {
      if (deviceSlots[i].hostSlot != 0xFF)
	{
	  io_mount_host_slot(deviceSlots[i].hostSlot);
	  io_mount_disk_image(i,deviceSlots[i].mode);
	}
    }
    
    io_set_boot_config(0); // disable config
    io_boot(); // and reboot.
}

void hosts_and_devices(void)
{
  if (quick_boot==true)
    subState=HD_DONE;
  else
    subState=HD_HOSTS;
  
  io_get_host_slots(&hostSlots[0]);
  io_get_device_slots(&deviceSlots[0]);
  screen_hosts_and_devices(&hostSlots[0],&deviceSlots[0]);

  while (state == HOSTS_AND_DEVICES)
    {
      switch(subState)
	{
	case HD_HOSTS:
	  hosts_and_devices_hosts();
	  break;
	case HD_DEVICES:
	  hosts_and_devices_devices();
	  break;
	case HD_DONE:
	  hosts_and_devices_done();
	  break;
	}
    }
}
