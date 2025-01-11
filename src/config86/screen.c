#ifdef BUILD_CONFIG86

/**
 * FujiNet Configuration Program: screen functions
 * The screen functions are a common set of graphics and text string functions to display information to the video output device.
 * These screen functions is comprised of two files; screen.c and screen.h.  Screen.c sets up the display dimensions, memory and
 * initializes the display as well as provide various text manipulations functions for proper display.  The screen.h file include
 * defines for various items such as keyboard codes, functions and screen memory addresses.
 *
 **/

#include <stdbool.h>
#include "screen.h"
#include "io.h"
#include "globals.h"
#include "bar.h"
#include "input.h"

unsigned char *video_ptr;  // a pointer to the memory address containing the screen contents
unsigned char *cursor_ptr; // a pointer to the current cursor position on the screen
char _visibleEntries;
extern bool copy_mode;
char text_empty[] = "Empty";
char fn[256];
extern HDSubState hd_subState;
extern DeviceSlot deviceSlots[NUM_DEVICE_SLOTS];
extern HostSlot hostSlots[8];

char uppercase_tmp[32]; // temp space for strupr(s) output.
                               // so original strings doesn't get changed.

void screen_mount_and_boot()
{
}

void screen_set_wifi(AdapterConfig *ac)
{
}

void screen_set_wifi_display_ssid(char n, SSIDInfo *s)
{
}

void screen_set_wifi_select_network(unsigned char nn)
{
}

void screen_set_wifi_custom(void)
{
}

void screen_set_wifi_password(void)
{
}

/*
 * Display the 'info' screen
 */
void screen_show_info(int printerEnabled, AdapterConfig *ac)
{
}

void screen_select_slot(const char *e)
{
}

void screen_select_slot_mode(void)
{
}

void screen_select_slot_choose(void)
{
}

void screen_select_slot_eject(unsigned char ds)
{
}

void screen_select_slot_build_eos_directory(void)
{
}

void screen_select_slot_build_eos_directory_label(void)
{
}

void screen_select_slot_build_eos_directory_creating(void)
{
}

void screen_select_file(void)
{
}

void screen_select_file_display(char *p, char *f)
{
}

void screen_select_file_display_long_filename(const char *e)
{
}

void screen_select_file_clear_long_filename(void)
{
}

void screen_select_file_filter(void)
{
}

void screen_select_file_next(void)
{
}

void screen_select_file_prev(void)
{
}

void screen_select_file_display_entry(unsigned char y, const char *e, unsigned entryType)
{
}

void screen_select_file_choose(char visibleEntries)
{
}

void screen_select_file_new_type(void)
{
}

void screen_select_file_new_size(unsigned char k)
{
}

void screen_select_file_new_custom(void)
{
}

void screen_select_file_new_name(void)
{
}

void screen_select_file_new_creating(void)
{
}

void screen_error(const char *msg)
{
}

void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d, bool *e)
{
}

// Show the keys that are applicable when we are on the Hosts portion of the screen.
void screen_hosts_and_devices_hosts()
{
}

// Show the keys that are applicable when we are on the Devices portion of the screen.
void screen_hosts_and_devices_devices()
{
}

void screen_hosts_and_devices_host_slots(HostSlot *h)
{
}

void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *dslot, bool *e)
{
}

void screen_hosts_and_devices_devices_clear_all(void)
{
}

void screen_hosts_and_devices_clear_host_slot(int i)
{
}

void screen_hosts_and_devices_edit_host_slot(int i)
{
}

void screen_hosts_and_devices_eject(unsigned char ds)
{
  screen_hosts_and_devices_devices();
}

void screen_hosts_and_devices_host_slot_empty(int hs)
{
}

void screen_hosts_and_devices_long_filename(const char *f)
{
}

void screen_init(void)
{
}

void screen_destination_host_slot(char *h, char *p)
{
}

void screen_destination_host_slot_choose(void)
{
}

void screen_perform_copy(char *sh, char *p, char *dh, char *dp)
{
}

void screen_connect_wifi(NetConfig *nc)
{
}

#endif
