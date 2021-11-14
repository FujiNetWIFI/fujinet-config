/**
 * FujiNet for #Adam configuration program
 *
 * Show adapter info
 */

#include "show_info.h"

#ifdef BUILD_ADAM
#include "adam/screen.h"
#include "adam/input.h"
#include "adam/globals.h"
#include "adam/io.h"
#endif /* BUILD_ADAM */

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
	case 0x0D:
	case 0x1B:
	case 0x20:
	case 0x85:
	  state=HOSTS_AND_DEVICES;
	  break;
	  state=SET_WIFI;
	  break;
	case 0x86:
	  state=CONNECT_WIFI;
	  break;
	}
    }
}
