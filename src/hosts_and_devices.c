/**
 * FujiNet for #Adam configuration program
 *
 * Hosts and Devices
 */

#include <string.h>
#include "globals.h"
#include "fuji_typedefs.h"
#include "hosts_and_devices.h"
#include "die.h"
#include "io.h"
#include "screen.h"
#include "input.h"
#include "bar.h"

DeviceSlot deviceSlots[8];
HostSlot hostSlots[8];
char selected_host_slot;
char selected_host_name[32];

static enum
  {
   HOSTS,
   DEVICES,
   DONE
  } subState = HOSTS;

void hosts_and_devices_edit_host_slot(unsigned char i)
{
  unsigned char o;

  if (strlen(hostSlots[i]==0))
    {
      screen_hosts_and_devices_clear_host_slot(i);
      o=0;
    }
  else
    o=strlen(hostSlots[i])-1;

  screen_hosts_and_devices_edit_host_slot(i);
  input_line_hosts_and_devices_host_slot(i,o,hostSlots[i]);
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
	  break;
	case 0x86:
	  subState=DONE;
	  state=DONE;
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

void hosts_and_devices_devices(void)
{
}

void hosts_and_devices_done(void)
{
}

void hosts_and_devices(void)
{
  subState=HOSTS;
  io_get_host_slots(&hostSlots[0]);
  io_get_device_slots(&deviceSlots[0]);
  screen_hosts_and_devices(&hostSlots[0],&deviceSlots[0]);

  while (subState != DONE)
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
