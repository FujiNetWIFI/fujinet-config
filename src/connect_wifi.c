/**
 * #FUJINET CONFIG
 * Connect Wifi Network
 */

#include <msx.h>
#include <eos.h>
#include <smartkeys.h>
#include <conio.h>
#include "connect_wifi.h"
#include "fuji_typedefs.h"
#include "fuji_adam.h"

extern char tmp[256];

/**
 * Setup
 */
void connect_wifi_setup(char* ssid)
{
  clrscr();
  smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
  sprintf(tmp,"  CONNECTING TO NETWORK:\n  %s",ssid);
  smartkeys_status(tmp);
}

/**
 * Wait for network connection
 */
State connect_wifi_wait_for_network(NetConfig* n, Context *context)
{
  State new_state = SET_WIFI;
  unsigned char wifiStatus=0;
  unsigned char retries=0;

  while (retries < 10)
    {
      sleep(1);
      fuji_adamnet_get_wifi_status(&wifiStatus);
      switch (wifiStatus)
	{
	case 1:
	  smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
	  smartkeys_status("  NO SSID AVAILABLE.");
	  sleep(5);
	  break;
	case 3:
	  smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
	  smartkeys_status("  CONNECTION SUCCESSFUL.");
	  sleep(2);
	  return DISKULATOR_HOSTS;
	  break;
	case 4:
	  smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
	  smartkeys_status("  CONNECT FAILED.");
	  sleep(5);
	  break;
	case 5:
	  smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
	  smartkeys_status("  CONNECTION LOST.");
	  sleep(5);
	  break;
	case 6:
	default:
	  retries++;
	  new_state=CONNECT_WIFI;
	  break;
	}
    }
  return new_state;
}

/**
 * Connect wifi State
 */
State connect_wifi(Context *context)
{
  NetConfig n;
  unsigned char s;

  sleep(1); // So we don't fire command too fast.
  
  fuji_adamnet_get_wifi_status(&s);

  if (s==3)
    return DISKULATOR_HOSTS;
  
  memset(n,0,sizeof(n));
  
  fuji_adamnet_get_ssid(&n);
  
  if (n.ssid[0] == 0)
  {
    connect_wifi_setup("NO SSID");
    return context->state=SET_WIFI;
  }
  else
  {
    connect_wifi_setup(n.ssid);
    fuji_adamnet_set_ssid(false,&n);
  }

  return connect_wifi_wait_for_network(&n,context);
}
