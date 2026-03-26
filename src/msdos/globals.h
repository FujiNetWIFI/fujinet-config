#ifdef BUILD_MSDOS
/**
 * Global variables
 */
#include <stdbool.h>

#include "../typedefs.h"
#include "../constants.h"
#include <fujinet-fuji.h>

#ifndef GLOBALS_H
#define GLOBALS_H

#define COLOR_SETTING_NETWORK 0x66
#define COLOR_SETTING_FAILED 0x33
#define COLOR_SETTING_SUCCESSFUL 0xB4
#define COLOR_CHECKING_NETWORK 0x26

// Y position on screen where the list of hosts starts.
#define HOSTS_START_Y 3

// Y position on screen where the list of hosts ends
#define HOSTS_END_Y (HOSTS_START_Y + NUM_HOST_SLOTS - 1)

// Y position on screen where the device slots start.
#define DEVICES_START_Y 14

// Y position on screen where the device slots end.
#define DEVICES_END_Y   (DEVICES_START_Y + NUM_DEVICE_SLOTS - 1)

// When mounting an image the device list/drive slots are on the screen by themselves at a different location than when the hosts are also shown.
#define DEVICES_START_MOUNT_Y   3
#define DEVICES_END_MOUNT_Y (DEVICES_START_MOUNT_Y + NUM_DEVICE_SLOTS-1)

// Y position on screen where the list of available wireless networks start.
// Row 3 = first inner row of the "Available Networks" box (box top border at row 2).
#define NETWORKS_START_Y    3

// Y position of the start of the file list from host.
#define FILES_START_Y   8

// Display field widths for new-disk prompts.
#define NEW_NAME_WIDTH    36   /* max filename chars shown/accepted */
#define NEW_SECTORS_WIDTH  7   /* max sector-count digits (9999999) */

// Rows used for new-disk prompts (below the file list box, above the status bar).
#define NEW_PROMPT_Y  22
#define NEW_INPUT_Y   23

/**
 * @brief Pointer to video memory
 */
extern unsigned char far *video;

extern State state;
extern char selected_host_slot;
extern char selected_device_slot;
extern unsigned char selected_network;
extern char selected_host_name[32];
extern char source_path[224];
extern char copy_host_name[32];
extern char copySpec[256];
extern unsigned long selected_size;
extern unsigned char numNetworks;

extern DeviceSlot deviceSlots[NUM_DEVICE_SLOTS];
extern HostSlot hostSlots[NUM_HOST_SLOTS];
extern char mode;

extern DirectoryPosition pos;
extern DirectoryPosition old_pos;
extern char path[224];
extern char filter[32];
extern bool create;
extern bool dir_eof;
extern bool quick_boot;
extern bool deviceEnabled[NUM_DEVICE_SLOTS];
extern bool backToFiles;
extern bool backFromCopy;
extern char _visibleEntries;

extern unsigned short custom_numSectors;
extern unsigned short custom_sectorSize;

extern bool mounting;

extern unsigned char wifiEnabled;

#endif /* GLOBALS_H */
#endif /* BUILD_MSDOS */
