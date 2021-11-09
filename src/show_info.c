/**
 * FujiNet for #Adam configuration program
 *
 * Show adapter info
 */

#include "show_info.h"
#include "screen.h"
#include "input.h"
#include "globals.h"
#include "io.h"

void show_info(void)
{
  screen_show_info(io_get_adapter_config());

  while (state==SHOW_INFO)
    {
      char k=input();
      switch(k)
	{
	case 0x00:
	  break;
	case 0x85:
	  state=SET_WIFI;
	  break;
	case 0x86:
	  state=CONNECT_WIFI;
	  break;
	default:
	  if (k>0x01 && k<0xFF)
	    state=HOSTS_AND_DEVICES;
	  break;
	}
    }
}
