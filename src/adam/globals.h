#ifdef BUILD_ADAM
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
extern char source_path[224];
extern char copy_host_name[32];
extern char copySpec[256];
extern unsigned long selected_size;

extern DeviceSlot deviceSlots[8];
extern HostSlot hostSlots[8];
extern char mode;

extern DirectoryPosition pos;
extern DirectoryPosition old_pos;
extern char path[224];
extern char filter[32];
extern bool create;
extern bool dir_eof;
extern bool quick_boot;
extern bool backToFiles;
extern bool backFromCopy;

#define NUM_DEVICE_SLOTS 4

#endif /* GLOBALS_H */
#endif /* BUILD_ADAM */
