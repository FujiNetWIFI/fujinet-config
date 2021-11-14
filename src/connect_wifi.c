/**
 * FujiNet for #Adam configuration program
 *
 * Connect to existing WiFi connection
 */

#include <string.h>
#include "connect_wifi.h"

#ifdef BUILD_ADAM
#include "adam/io.h"
#include "adam/screen.h"
#include "adam/globals.h"
#endif /* BUILD_ADAM */
 
void connect_wifi(void)
{
  unsigned char retries=20;
  NetConfig nc;
  unsigned char s;
    
  memcpy(&nc,io_get_ssid(),sizeof(NetConfig));

  state = SET_WIFI;
  
  screen_connect_wifi(&nc);

  while (retries>0)
    {
      sleep(1);
      s=io_get_wifi_status();
      switch (s)
	{
	case 1:
	  screen_error("  NO SSID AVAILABLE.");
	  sleep(2);
	  return;
	case 3:
	  screen_error("  CONNECTION SUCCESSFUL.");
	  state=HOSTS_AND_DEVICES;
	  sleep(2);
	  return;
	case 4:
	  screen_error("  CONNECT FAILED.");
	  sleep(2);
	  return;
	case 5:
	  screen_error("  CONNECTION LOST.");
	  sleep(2);
	  return;
	default:
	  retries--;
	  break;
	}
    }

  screen_error("  UNABLE TO CONNECT.");
  sleep(2);
  state=SET_WIFI;
}
