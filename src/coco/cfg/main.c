#include <cmoc.h>
#include "../../hosts_and_devices.h"
#include "../coco_screen.h"
#include "../../typedefs.h"
#include "../../system.h"
#include "../../globals.h"
#include "../launch/entry.h"

extern void runm(const char *filename);

State state;
bool backToFiles = false;
bool backFromCopy = false;
bool quick_boot = false;

int main(void)
{
  if (entry_take() == ENTRY_HOSTS)
  {
    selected_host_slot = ENTRY_DATA->host_slot;
    selected_device_slot = ENTRY_DATA->device_slot;
  }

  screen_init();
  state = HOSTS_AND_DEVICES;

  while (true)
  {
    switch (state)
    {
    case HOSTS_AND_DEVICES:
      hosts_and_devices();
      break;
    case SELECT_FILE:
      entry_set(ENTRY_FILES, selected_host_slot, selected_device_slot);
      screen_loading("File Browser");
      screen_handoff();
      runm(APP("FILES"));
      break;
    case SHOW_INFO:
      entry_set(ENTRY_INFO, selected_host_slot, selected_device_slot);
      screen_loading("Adapter Info");
      screen_handoff();
      runm(APP("WIFI"));
      break;
    case DONE:
      fuji_set_boot_config(0);
      screen_handoff();
      system_boot();
      break;
    default:
      state = HOSTS_AND_DEVICES;
      break;
    }
  }

  return 0;
}
