#include <cmoc.h>
#include "../../globals.h"
#include "../../constants.h"

/* Shared state that hosts_and_devices.c defines for the MAIN app. */
DeviceSlot deviceSlots[NUM_DEVICE_SLOTS];
bool deviceEnabled[NUM_DEVICE_SLOTS];
HostSlot hostSlots[8];
char selected_host_slot = 0;
char selected_device_slot = 0;
char selected_host_name[32];
bool slots_dirty = true;
