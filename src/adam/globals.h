/**
 * Global variables
 */

#include "../typedefs.h"
#include "fuji_typedefs.h"

#ifndef GLOBALS_H
#define GLOBALS_H

extern State state;
extern char response[1024];
extern char selected_host_slot;
extern char selected_host_name[32];

extern DeviceSlot deviceSlots[8];
extern HostSlot hostSlots[8];

extern DirectoryPosition pos;
extern char path[224];
extern char filter[32];

#endif /* GLOBALS_H */
