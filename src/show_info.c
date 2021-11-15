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

#ifdef BUILD_APPLE2
#include "apple2/screen.h"
#include "apple2/input.h"
#include "apple2/globals.h"
#include "apple2/io.h"
#endif /* BUILD_APPLE2 */

#ifdef BUILD_C64
#include "c64/screen.h"
#include "c64/input.h"
#include "c64/globals.h"
#include "c64/io.h"
#endif /* BUILD_C64 */

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
