/**
 * Select a Destination Host Slot
 */

#include "destination_host_slot.h"
#include "select_slot.h"
#include "typedefs.h"
#include "screen.h"
#include "input.h"
#include "globals.h"

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
  fuji_get_host_slots(&hostSlots[0], NUM_HOST_SLOTS);
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
