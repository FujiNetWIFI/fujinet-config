/**
 * FujiNet for #Adam configuration program
 *
 * Set new WiFi connection
 */

#include <string.h>
#include "io.h"
#include "fuji_typedefs.h"
#include "screen.h"
#include "set_wifi.h"
#include "die.h"
#include "bar.h"
#include "input.h"
#include "globals.h"

static enum
  {
   SCAN,
   SELECT,
   CUSTOM,
   PASSWORD,
   DONE
  } subState=SCAN;

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

  while(subState==SELECT)
    {
      k=input();
      
      switch(k)
	{
	case 0x0D:
	  set_wifi_set_ssid(bar_get());
	  subState=PASSWORD;
	  break;
	case 0x84:
	  subState=CUSTOM;
	  break;
	case 0x85:
	  subState=SCAN;
	  break;
	case 0x86:
	  subState=DONE;
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
  subState=PASSWORD;
}

void set_wifi_password(void)
{
  screen_set_wifi_password();
  input_line_set_wifi_password(nc.password);
  subState=DONE;
}

void set_wifi_scan(void)
{
  screen_set_wifi(io_get_adapter_config());

  numNetworks = io_scan_for_networks();

  if (numNetworks > 16)
    numNetworks = 16;

  if (io_error())
    {
      screen_error("COULD NOT SCAN NETWORKS");
      die();
    }

  for (char i=0;i<numNetworks;i++)
    {
      SSIDInfo *s = io_get_scan_result(i);
      screen_set_wifi_display_ssid(i,s);
    }

  subState=SELECT;
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
	case SCAN:
	  set_wifi_scan();
	  break;
	case SELECT:
	  set_wifi_select();
	  break;
	case CUSTOM:
	  set_wifi_custom();
	  break;
	case PASSWORD:
	  set_wifi_password();
	  break;
	case DONE:
	  set_wifi_done();
	  break;
	}
    }
}
