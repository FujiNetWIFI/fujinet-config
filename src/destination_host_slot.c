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

#ifdef BUILD_ATARI
#include "atari/screen.h"
#include "atari/input.h"
#include "atari/globals.h"
#include "atari/io.h"
#include "atari/bar.h"
#endif /* BUILD_ATARI */

#ifdef BUILD_C64
#include "c64/screen.h"
#include "c64/input.h"
#include "c64/globals.h"
#include "c64/io.h"
#include "c64/bar.h"
#endif /* BUILD_APPLE2 */

#ifdef BUILD_PC8801
#include "pc8801/screen.h"
#include "pc8801/input.h"
#include "pc8801/globals.h"
#include "pc8801/io.h"
#include "pc8801/bar.h"
#endif /* BUILD_PC8801 */

#ifdef BUILD_PC6001
#include "pc6001/screen.h"
#include "pc6001/input.h"
#include "pc6001/globals.h"
#include "pc6001/io.h"
#include "pc6001/bar.h"
#endif /* BUILD_PC6001 */

#include "destination_host_slot.h"

DHSubState dh_subState;

extern HostSlot hostSlots[8];
extern char copy_destination_path[128];
extern unsigned char copy_host_slot;
extern bool copy_mode;
extern SFSubState sf_subState;

void destination_host_slot_init()
{
  screen_destination_host_slot((char *)hostSlots[selected_host_slot],(char *)path);
  dh_subState=DH_DISPLAY;
}

void destination_host_slot_display()
{
  io_get_host_slots(&hostSlots[0]);
  screen_hosts_and_devices_host_slots(&hostSlots[0]);
  dh_subState=DH_CHOOSE;
}

void destination_host_slot_choose()
{
  screen_destination_host_slot_choose();
  while (dh_subState==DH_CHOOSE)
    dh_subState=input_destination_host_slot_choose();
}

void destination_host_slot_done()
{
  state=SELECT_FILE;
  sf_subState=SF_INIT;
  copy_mode=true;
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
