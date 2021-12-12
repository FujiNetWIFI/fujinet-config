/**
 * FujiNet for #Adam configuration program
 *
 * Hosts and Devices
 */

#include <string.h>
#include "hosts_and_devices.h"
#include "die.h"
#include "typedefs.h"

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

HDSubState hd_subState;
DeviceSlot deviceSlots[8];
HostSlot hostSlots[8];
char selected_host_slot;
char selected_device_slot;
char selected_host_name[32];

extern bool quick_boot;

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
  screen_hosts_and_devices_hosts();

  while (hd_subState==HD_HOSTS)
    hd_subState=input_hosts_and_devices_hosts();
}

void hosts_and_devices_long_filename(void)
{
  char *f = io_get_device_filename(selected_device_slot);
  
  screen_hosts_and_devices_long_filename(f);
}

void hosts_and_devices_eject(unsigned char ds)
{
  io_umount_disk_image(ds);
  memset(deviceSlots[ds].file,0,FILE_MAXLEN);
  deviceSlots[ds].hostSlot=0xFF;
  io_put_device_slots(&deviceSlots[0]);
  io_get_device_slots(&deviceSlots[0]);
  screen_hosts_and_devices_eject(ds);
  hosts_and_devices_long_filename();
}

void hosts_and_devices_devices_clear_all(void)
{
  char i;
  
  screen_hosts_and_devices_devices_clear_all();
  
  for (i=0;i<4;i++)
    hosts_and_devices_eject(i);

  hd_subState=HD_DEVICES;
}

void hosts_and_devices_devices(void)
{
  char k=0;

  screen_hosts_and_devices_devices();
  hosts_and_devices_long_filename();
  
  while (hd_subState==HD_DEVICES)
    hd_subState=input_hosts_and_devices_devices();
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
    hd_subState=HD_DONE;
  else
    hd_subState=HD_HOSTS;
  
  io_get_host_slots(&hostSlots[0]);
  io_get_device_slots(&deviceSlots[0]);
  screen_hosts_and_devices(&hostSlots[0],&deviceSlots[0]);

  while (state == HOSTS_AND_DEVICES)
    {
      switch(hd_subState)
	{
	case HD_HOSTS:
	  hosts_and_devices_hosts();
	  break;
	case HD_DEVICES:
	  hosts_and_devices_devices();
	  break;
	case HD_CLEAR_ALL_DEVICES:
	  hosts_and_devices_devices_clear_all();
	  break;
	case HD_DONE:
	  hosts_and_devices_done();
	  break;
	}
    }
}
