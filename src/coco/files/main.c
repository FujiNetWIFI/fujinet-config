#include <cmoc.h>
#include "../../select_file.h"
#include "../../select_slot.h"
#include "../../perform_copy.h"
#include "../../destination_host_slot.h"
#include "../coco_screen.h"
#include "../../typedefs.h"
#include "../../globals.h"
#include "../../constants.h"
#include "../launch/entry.h"

extern void runm(const char *filename);

State state;
bool backToFiles = false;
bool backFromCopy = false;

int main(void)
{
  if (entry_take() == ENTRY_FILES)
  {
    selected_host_slot = ENTRY_DATA->host_slot;
    selected_device_slot = ENTRY_DATA->device_slot;
  }

  screen_init();
  fuji_get_host_slots(&hostSlots[0], NUM_HOST_SLOTS);
  fuji_get_device_slots(deviceSlots, NUM_DEVICE_SLOTS);
  strcpy(selected_host_name, (char *)hostSlots[selected_host_slot]);
  state = SELECT_FILE;

  while (true)
  {
    switch (state)
    {
    case SELECT_FILE:
      select_file();
      break;
    case SELECT_SLOT:
      select_slot();
      break;
    case DESTINATION_HOST_SLOT:
      destination_host_slot();
      break;
    case PERFORM_COPY:
      perform_copy();
      break;
    default:
      entry_set(ENTRY_HOSTS, selected_host_slot, selected_device_slot);
      screen_loading("Hosts and Devices");
      screen_handoff();
      runm("MAIN");
      break;
    }
  }

  return 0;
}
