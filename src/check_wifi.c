/**
 * Check for Wifi Configuration and/or connection.
 */

#include "check_wifi.h"
#include "constants.h"
#include "globals.h"
#include "screen.h"

extern NetConfig nc;

unsigned char wifiEnabled = true;

void check_wifi(void)
{
  unsigned char status;
  // Before checking actual connectivity, there's a command to check if wifi is completely disabled, check that first. If it's
  // disabled, don't go to the set/connect wifi screens.
  //
  if ( !fuji_get_wifi_enabled() )
  {
    wifiEnabled = false;
    state=HOSTS_AND_DEVICES;
#ifdef COLOR_SETTING_FAILED
    bar_set_color(COLOR_SETTING_FAILED);
#endif
  }
  else {
    fuji_get_wifi_status(&status);
    if (status == 3)
    {
      wifiEnabled = true;
#ifdef COLOR_SETTING_SUCCESSFUL
      bar_set_color(COLOR_SETTING_SUCCESSFUL);
#endif
      state = HOSTS_AND_DEVICES;
    }
    else {
      wifiEnabled = false;
#ifdef COLOR_SETTING_FAILED
      bar_set_color(COLOR_SETTING_FAILED);
#endif
      fuji_get_ssid(&nc);
      if (nc.ssid[0] == 0x00)
        state = SET_WIFI;
      else
        state = CONNECT_WIFI;
    }
  }
}
