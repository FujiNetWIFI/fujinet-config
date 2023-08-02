#ifdef BUILD_PC8801
/**
 * Global variables
 */

#include <stdbool.h>
#include "../typedefs.h"
#include "../fuji_typedefs.h"

#ifndef GLOBALS_H
#define GLOBALS_H

extern State state;
extern char response[1024];
extern char selected_host_slot;
extern char selected_device_slot;
extern char selected_host_name[32];
extern unsigned long selected_size;

extern DeviceSlot deviceSlots[8];
extern HostSlot hostSlots[8];
extern char mode;

extern DirectoryPosition pos;
extern char path[224];
extern char filter[32];
extern bool create;
extern bool dir_eof;
extern bool quick_boot;
extern bool backToFiles;
extern bool backFromCopy;

#endif /* GLOBALS_H */
#endif /* BUILD_PC8801 */
