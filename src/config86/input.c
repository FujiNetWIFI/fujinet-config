#ifdef BUILD_CONFIG86

/**
 * Input routines
 */

#include "input.h"
#include "io.h"
#include "bar.h"
#include "screen.h"
#include "globals.h"
#include "mount_and_boot.h"
#include "../hosts_and_devices.h"
#include "../select_file.h"
#include "../set_wifi.h"
#include "../select_slot.h"

unsigned char selected_network;
extern bool copy_mode;
extern unsigned char copy_host_slot;
unsigned short custom_numSectors;
unsigned short custom_sectorSize;
extern char fn[256];
bool mounting = false;
extern unsigned short entry_timer;
extern HDSubState hd_subState;

char input()
{
    return 0;
}

unsigned char input_ucase()
{
    return 0;
}

unsigned char input_handle_joystick(void)
{
  return 0;
}

// Old cursor position
char ox=-1;
char oy=-1;
char oc=0;

void input_cursor(char x, char y)
{
}

void input_line(unsigned char x, unsigned char y, char *c, unsigned char l, bool password)
{
}

void input_line_set_wifi_custom(char *c)
{
  input_line(0,15,c,32,false);
}

void input_line_set_wifi_password(char *c)
{
  input_line(0,15,c,64,true);
}

void input_line_hosts_and_devices_host_slot(int i, unsigned int o, char *c)
{
  bar_clear(true);
  input_line(1,(unsigned char)i+1,c,32,false);
}

void input_line_filter(char *c)
{
    input_line(0,15,c,31,false);
}

unsigned char input_select_file_new_type(void)
{
    return 1; // We always wanna make a DSK
}

unsigned long input_select_file_new_size(unsigned char t)
{
    return 0UL;
}

unsigned long input_select_file_new_custom(void)
{
  return 0;
}

void input_select_file_new_name(char *c)
{
    input_line(0,15,c,255,false);
}

bool input_select_slot_build_eos_directory(void)
{
  return false;
}

void input_select_slot_build_eos_directory_label(char *c)
{
}

WSSubState input_set_wifi_select(void)
{
  return WS_SELECT;
}

/**
 * Handle global console keys.
 */
unsigned char input_handle_console_keys(void)
{
  return 0;
}

HDSubState input_hosts_and_devices_hosts(void)
{
  return HD_HOSTS;
}

HDSubState input_hosts_and_devices_devices(void)
{
  return HD_DEVICES;
}

SFSubState input_select_file_choose(void)
{  
  return SF_CHOOSE;
}

SSSubState input_select_slot_choose(void)
{
  return SS_CHOOSE;
}

unsigned char input_select_slot_mode(char *mode)
{
  return 1;
}

/*
 *  Handle inupt for the "show info" screen.
 *
 *  'C' - Reconnect Wifi
 *  'S' - Change SSID
 *  Any other key - return to main hosts and devices screen
 *
 */
SISubState input_show_info(void)
{
  return SI_SHOWINFO;
}

DHSubState input_destination_host_slot_choose(void)
{
  return DH_CHOOSE;
}

void set_device_slot_mode(unsigned char slot, unsigned char mode)
{
}

#endif /* BUILD_CONFIG86 */
