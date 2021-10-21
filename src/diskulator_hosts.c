/**
 * #FUJINET CONFIG
 * Diskulator Hosts/Devices
 */

#include <msx.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include "diskulator_hosts.h"
#include "fuji_adam.h"
#include "fuji_typedefs.h"
#include "input.h"

char empty[]="Empty";
char fn[256];

typedef enum _substate
  {
   HOSTS,
   DEVICES,
   DONE
  } SubState;

#define HOST_SLOTS_Y 1

/**
 * Display Hosts Slots
 */
void diskulator_hosts_display_host_slots(unsigned char y, Context *context)
{
  unsigned char i;

}

/**
 * Display device slots
 */
void diskulator_hosts_display_device_slots(unsigned char y, Context *context)
{
  unsigned char i;
  unsigned char d[6];

}

/**
 * Keys for Hosts mode
 */
void diskulator_hosts_keys_hosts(void)
{
}

/**
 * Keys for Devices mode
 */
void diskulator_hosts_keys_devices(void)
{
}

/**
 * Diskulator hosts setup
 */
void diskulator_hosts_setup(Context *context)
{
  unsigned char retry=5;

}

/**
 * Handle jump keys (1-8, shift 1-8)
 */
void diskulator_hosts_handle_jump_keys(unsigned char k,unsigned char *i, SubState *ss)
{
}

/**
 * Handle nav keys depending on sub-state (HOSTS or DEVICES)
 */
void diskulator_hosts_handle_nav_keys(unsigned char k, unsigned char *i, SubState *new_substate)
{
  unsigned char o;

}

/**
 * Edit a host slot
 */
void diskulator_hosts_edit_host_slot(unsigned char i, Context *context)
{
}

/**
 * Eject image from device slot
 */
void diskulator_hosts_eject_device_slot(unsigned char i, unsigned char pos, Context *context)
{
  char tmp[2]={0,0};

}

/**
 * Set device slot to read or write
 */
void diskulator_hosts_set_device_slot_mode(unsigned char i, unsigned char mode, Context* context)
{
  unsigned char tmp_hostSlot;
  unsigned char tmp_file[FILE_MAXLEN];

}

/**
 * Diskulator interactive - hosts
 */
void diskulator_hosts_hosts(Context *context, SubState *new_substate)
{
  unsigned char k;
  unsigned char i=0;

}

/**
 * Diskulator interactive - device slots
 */
void diskulator_hosts_devices(Context *context, SubState *new_substate)
{
  unsigned char k;
  unsigned char i=0;

}

/**
 * Clear file context
 */
void diskulator_hosts_clear_file_context(Context *context)
{
  memset(context->directory,0,sizeof(context->directory));
  memset(context->filter,0,sizeof(context->filter));
  context->dir_page=0;
}

/**
 * Connect wifi State
 */
State diskulator_hosts(Context *context)
{
  SubState ss=HOSTS;

  cprintf("DISKULATOR HOSTS");

  while (1) {}
  
  return context->state;
}
