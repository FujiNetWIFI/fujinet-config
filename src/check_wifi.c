/**
 * Check for Wifi Configuration and/or connection.
 */

#include "check_wifi.h"
#include "globals.h"

extern NetConfig nc;

void check_wifi(void)
{
  unsigned char status;
  // Before checking actual connectivity, there's a command to check if wifi is completely disabled, check that first. If it's
  // disabled, don't go to the set/connect wifi screens.
  //
  if ( !fuji_get_wifi_enabled() )
  {
    state=HOSTS_AND_DEVICES;
  }
  else {
    fuji_get_wifi_status(&status);
    if (status == 3)
      state = HOSTS_AND_DEVICES;
    else {
      fuji_get_ssid(&nc);
      if (nc.ssid[0] == 0x00)
        state = SET_WIFI;
      else
        state = CONNECT_WIFI;
    }
  }
}
