/**
 * Global variables
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include "typedefs.h"

extern State state;
extern char response[];
extern char selected_host_slot;
extern char selected_device_slot;
extern char selected_host_name[];
extern char source_path[];
extern char copySpec[];
extern char copy_host_name[];
extern unsigned long selected_size;

extern DeviceSlot deviceSlots[];
extern HostSlot hostSlots[];
extern char mode;

extern DirectoryPosition pos;
extern DirectoryPosition old_pos;
extern char path[];
extern bool create;
extern char filter[];
extern bool dir_eof;
extern bool quick_boot;
extern bool backToFiles;
extern bool backFromCopy;

extern bool screen_should_be_cleared;
extern unsigned char numNetworks;
extern unsigned char wifiEnabled;
extern char _visibleEntries;
extern unsigned short custom_numSectors;
extern unsigned short custom_sectorSize;

extern bool deviceEnabled[];
extern bool mounting;

#endif /* GLOBALS_H */
