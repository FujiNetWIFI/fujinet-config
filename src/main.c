/**
 * #FujiNet for ADAM Configuration Tool
 *
 */

#include <stdlib.h>
#include "typedefs.h"
#include "check_wifi.h"
#include "connect_wifi.h"
#include "set_wifi.h"
#include "hosts_and_devices.h"
#include "select_file.h"
#include "select_slot.h"
#include "perform_copy.h"
#include "show_info.h"
#include "io.h"
#include "screen.h"

State state=CHECK_WIFI;
extern DeviceSlot deviceSlots[8];

void setup(void)
{
  io_init();
  screen_init();
}

void done(void)
{
}

void run(void)
{
  while (state != DONE)
    {
      switch(state)
	{
	case CHECK_WIFI:
	  check_wifi();
	  break;
	case CONNECT_WIFI:
	  connect_wifi();
	  break;
	case SET_WIFI:
	  set_wifi();
	  break;
	case HOSTS_AND_DEVICES:
	  hosts_and_devices();
	  break;
	case SELECT_FILE:
	  select_file();
	  break;
	case SELECT_SLOT:
	  select_slot();
	  break;
	case PERFORM_COPY:
	  perform_copy();
	  break;
	case SHOW_INFO:
	  show_info();
	  break;
	case DONE:
	  done();
	  break;
	}
    }
}

void main(void)
{
  setup();
  run();
}
