/**
 * FujiNet for #Adam configuration program
 *
 * Check for Wifi Configuration and/or connection.
 */

#ifdef BUILD_ADAM
#include "adam/io.h"
#include "adam/globals.h"
#endif /* BUILD_ADAM */

#include "check_wifi.h"

void check_wifi(void)
{
  if (io_get_wifi_status() == 3)
    {
      state=HOSTS_AND_DEVICES;
    }
  else if (io_get_ssid()->ssid[0] == 0x00)
    {
      state=SET_WIFI;
    }
  else
    {
      state=CONNECT_WIFI;
    }
}
