/**
 * #FUJINET Configuration
 *
 * State Machine and context
 */

#ifndef STATE_H
#define STATE_H

#include <stdbool.h>
#include "fuji_typedefs.h"

typedef enum _state
  {
   SET_WIFI,
   CONNECT_WIFI,
   DISKULATOR_HOSTS,
   DISKULATOR_SELECT,
   DISKULATOR_SLOT,
   DISKULATOR_INFO,
   DISKULATOR_COPY,
   MOUNT_AND_BOOT
  } State;

typedef enum _copySubState
  {
   DISABLED,
   SELECT_HOST_SLOT,
   SELECT_DESTINATION_FOLDER,
   DO_COPY
  } CopySubState;

typedef struct _context
{
  bool net_connected;                // Is network connected?
  CopySubState copySubState;         // Copy Sub State
  State state;                       // Current program state (state.h)
  char filter[32];                   // filter (wildcard)
  char directory[128];               // Current directory
  char directory_plus_filter[128]; 
  char filename[128];                // Current filename
  char full_path[256];               // Full path.
  char entry_widths[14];             // Widths of each entry for expansion.
  unsigned char host_slot;           // Current Host slot (0-7)
  unsigned char device_slot;         // Current Device slot (0-7)
  unsigned char host_slot_source;    // Source host slot (0-7) (for copy)
  unsigned char host_slot_dest;      // Destination host slot (1-8) (for copy)
  unsigned char mode;                // mode for device slot (1 or 2)
  unsigned short dir_page;           // directory page
  bool dir_eof;                      // End of current directory?
  unsigned char entries_displayed;   // # of entries displayed on current page.
  bool newDisk;                      // Are we making a new disk?
  unsigned short newDisk_ns;         // New disk # of sectors
  unsigned short newDisk_sz;         // New disk sector size
  DeviceSlots deviceSlots;           // Current device slots
  HostSlots hostSlots;               // Current host slots.
} Context;

/**
 * Set up initial context
 */
void context_setup(Context* context);

/**
 * State machine
 */
void state(Context *context);

#endif /* STATE_H */
