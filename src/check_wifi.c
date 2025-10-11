/**
 * Check for Wifi Configuration and/or connection.
 */

#include "check_wifi.h"
#include "io.h"
#include "globals.h"

void check_wifi(void)
{
  // Before checking actual connectivity, there's a command to check if wifi is completely disabled, check that first. If it's
  // disabled, don't go to the set/connect wifi screens.
  //
  if ( !io_get_wifi_enabled() )
  {
    state=HOSTS_AND_DEVICES;
  }
  else if (io_get_wifi_status() == 3)
  {
	  state = HOSTS_AND_DEVICES;
  }
  else if (io_get_ssid()->ssid[0] == 0x00)
  {
	  state = SET_WIFI;
  }
  else
  {
	  state = CONNECT_WIFI;
  }
}
