/**
 * FujiNet for #Adam configuration program
 *
 * Set new WiFi connection
 */

#include <string.h>
#include "set_wifi.h"
#include "die.h"

#ifdef BUILD_ADAM
#include "adam/io.h"
#include "adam/fuji_typedefs.h"
#include "adam/screen.h"
#include "adam/bar.h"
#include "adam/input.h"
#include "adam/globals.h"
#endif /* BUILD_ADAM */

#ifdef BUILD_APPLE2
#include "apple2/io.h"
#include "apple2/fuji_typedefs.h"
#include "apple2/screen.h"
#include "apple2/bar.h"
#include "apple2/input.h"
#include "apple2/globals.h"
#endif /* BUILD_APPLE2 */

#ifdef BUILD_C64
#include "c64/io.h"
#include "c64/fuji_typedefs.h"
#include "c64/screen.h"
#include "c64/bar.h"
#include "c64/input.h"
#include "c64/globals.h"
#endif /* BUILD_APPLE2 */

static enum
  {
   SF_SCAN,
   SF_SELECT,
   SF_CUSTOM,
   SF_PASSWORD,
   SF_DONE
  } subState=SF_SCAN;

NetConfig nc;

static unsigned char numNetworks;

void set_wifi_set_ssid(unsigned char i)
{
  SSIDInfo *s = io_get_scan_result(i);
  memcpy(nc.ssid,s->ssid,32);
}

void set_wifi_select(void)
{
  unsigned char k=0;
  
  screen_set_wifi_select_network(numNetworks);

  bar_set(0,3,numNetworks,0);

  while(subState==SF_SELECT)
    {
      k=input();
      
      switch(k)
	{
	case 0x0D:
	  set_wifi_set_ssid(bar_get());
	  subState=SF_PASSWORD;
	  break;
	case 0x84:
	  subState=SF_CUSTOM;
	  break;
	case 0x85:
	  subState=SF_SCAN;
	  break;
	case 0x86:
	  subState=SF_DONE;
	  state=HOSTS_AND_DEVICES;
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

void set_wifi_custom(void)
{
  screen_set_wifi_custom();
  input_line_set_wifi_custom(nc.ssid);
  subState=SF_PASSWORD;
}

void set_wifi_password(void)
{
  screen_set_wifi_password();
  input_line_set_wifi_password(nc.password);
  subState=SF_DONE;
}

void set_wifi_scan(void)
{
  char i;
  
  screen_set_wifi(io_get_adapter_config());

  numNetworks = io_scan_for_networks();

  if (numNetworks > 16)
    numNetworks = 16;

  if (io_error())
    {
      screen_error("COULD NOT SF_SCAN NETWORKS");
      die();
    }

  for (i=0;i<numNetworks;i++)
    {
      SSIDInfo *s = io_get_scan_result(i);
      screen_set_wifi_display_ssid(i,s);
    }

  subState=SF_SELECT;
}

void set_wifi_done(void)
{
  io_set_ssid(&nc);
  state=CONNECT_WIFI;
}

void set_wifi(void)
{
  while (state == SET_WIFI)
    {
      switch(subState)
	{
	case SF_SCAN:
	  set_wifi_scan();
	  break;
	case SF_SELECT:
	  set_wifi_select();
	  break;
	case SF_CUSTOM:
	  set_wifi_custom();
	  break;
	case SF_PASSWORD:
	  set_wifi_password();
	  break;
	case SF_DONE:
	  set_wifi_done();
	  break;
	}
    }
}
