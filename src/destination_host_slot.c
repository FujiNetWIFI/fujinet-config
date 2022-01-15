/**
 * FujiNet for #Adam configuration program
 *
 * Select a Destination Host Slot
 */

#include <stdlib.h>
#include <string.h>
#include "select_slot.h"

#ifdef BUILD_ADAM
#include <eos.h>
#include <conio.h>
#include "adam/screen.h"
#include "adam/input.h"
#include "adam/globals.h"
#include "adam/io.h"
#include "adam/bar.h"
#endif /* BUILD_ADAM */

#ifdef BUILD_APPLE2
#include "apple2/screen.h"
#include "apple2/input.h"
#include "apple2/globals.h"
#include "apple2/io.h"
#include "apple2/bar.h"
#endif /* BUILD_APPLE2 */

#ifdef BUILD_C64
#include "c64/screen.h"
#include "c64/input.h"
#include "c64/globals.h"
#include "c64/io.h"
#include "c64/bar.h"
#endif /* BUILD_APPLE2 */

#include "destination_host_slot.h"

DHSubState dh_subState;

void destination_host_slot_init()
{
}

void destination_host_slot_display()
{
}

void destination_host_slot_choose()
{
}

void destination_host_slot_done()
{
}

void destination_host_slot(void)
{
  dh_subState = DH_INIT;

  while (state==DESTINATION_HOST_SLOT)
    {
      switch(dh_subState)
	{
	case DH_INIT:
	  destination_host_slot_init();
	  break;
	case DH_DISPLAY:
	  destination_host_slot_display();
	  break;
	case DH_CHOOSE:
	  destination_host_slot_choose();
	  break;
	case DH_DONE:
	  destination_host_slot_done();
	  break;
	case DH_ABORT:
	  break;
	}
    }
}
