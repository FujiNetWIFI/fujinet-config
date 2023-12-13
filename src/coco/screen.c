#ifdef _CMOC_VERSION_

/**
 * FujiNet Configuration Program: screen functions
 * The screen functions are a common set of graphics and text string functions to display information to the video output device.
 * These screen functions is comprised of two files; screen.c and screen.h.  Screen.c sets up the display dimensions, memory and
 * initializes the display as well as provide various text manipulations functions for proper display.  The screen.h file include
 * defines for various items such as keyboard codes, functions and screen memory addresses.
 *
 **/

#include <cmoc.h>
#include <coco.h>
#include "screen.h"
#include "io.h"
#include "globals.h"
#include "bar.h"
#include "input.h"

#define SCREEN_RAM_TOP 0x0400
#define INVERSE_MASK   0xBF   // Bit 6 clear for inverse character

unsigned char *video_ptr;  // a pointer to the memory address containing the screen contents
unsigned char *cursor_ptr; // a pointer to the current cursor position on the screen
char _visibleEntries;
extern bool copy_mode;
char text_empty[] = "Empty";
char fn[256];
extern HDSubState hd_subState;

void set_active_screen(unsigned char screen)
{
}

/**********************
 * Print ATASCII string to display memory.  Note: ATASCII is not a 1:1 mapping for ASCII.  It is a ven diagram with significant overlap.
 */
void put_char(char c)
{
}

void screen_append(char *s)
{
}

void screen_puts(unsigned char x, unsigned char y, char *s)
{
}

void font_init()
{
}

void screen_mount_and_boot()
{
}

void screen_set_wifi(AdapterConfigExtended *ac)
{
}

void screen_set_wifi_print_rssi(SSIDInfo *s, unsigned char i)
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
void screen_show_info(int printerEnabled, AdapterConfigExtended *ac)
{
}

void screen_select_slot(char *e)
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

void screen_select_file_display_long_filename(char *e)
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

void screen_select_file_display_entry(unsigned char y, char *e, unsigned entryType)
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

void screen_clear_line(unsigned char y)
{
}

void screen_error(const char *msg)
{
  printf("%s",msg);
}

void screen_hosts_and_devices_hosts_print(HostSlot *h)
{
}

void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d, unsigned char *e)
{
  switch(hd_subState)
    {
    case HD_HOSTS:
      /* screen_hosts_and_devices_hosts(); */
      screen_hosts_and_devices_host_slots(h);
      break;
    case HD_DEVICES:
      /* screen_hosts_and_devices_devices(); */
      screen_hosts_and_devices_device_slots(1,d,NULL);
      break;
    }  
}

void screen_clear()
{
}

// Show the keys that are applicable when we are on the Hosts portion of the screen.
void screen_hosts_and_devices_hosts()
{
  cls(3);
  locate(0,0);
  printf("%32s","host\x80slots");
  memset(0x400,0x20,22);
  (*(unsigned char *)0x041a) = 0x20;

  locate(0,14);
  printf("\x80\x80\x80\x31-8slotEditENTERbrowseLobby\x80\x80\x80\x80\x80\x80\x43onfigTABdrivesBREAKboot\x80\x80\x80");
}

// Show the keys that are applicable when we are on the Devices portion of the screen.
void screen_hosts_and_devices_devices()
{
  memset(0x400,0x20,32);
  cls(4);
  locate(0,0); 
  printf("%32s","device\x80\slots");

  locate(0,14);
  printf("\x80\x80\x80\x31-8slotEditENTERbrowseLobby\x80\x80\x80\x80\x80\x80\x43onfigTABdrivesBREAKboot\x80\x80\x80");  
}

void screen_hosts_and_devices_host_slots(HostSlot *h)
{
  char *p = &h[0]; // We need this because cmoc doesn't like untangling multi-dimensional typedefs
  char *sp = (unsigned char *)SCREEN_RAM_TOP;

  sp += 32;  // start one line down. 
  locate(0,1); 

  for (int i=0;i<8;i++)
    {
      printf("%u%-31s",i+1,p);
      p += 32;  // Next entry
      *sp &= INVERSE_MASK;
      sp += 32; // next line
    }
}

// Since 'deviceSlots' is a global, do we need to access the input parameter at all?
// Maybe globals.h wasn't supposed in be part of screen? I needed it for something..
void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *dslot, unsigned char *e)
{
  char *sp = (unsigned char *)SCREEN_RAM_TOP;

  sp += 32;  // start one line down. 

  for (int i=0;i<8;i++)
    {
      locate(0,i+1);
      printf("%u%u",i+1,dslot->hostSlot);
      printf("%c",dslot->mode == 0x02 ? 0x9A : 0xAA);
      printf("%s",dslot->file);
      dslot++;
    }
}

void screen_hosts_and_devices_devices_clear_all(void)
{
  screen_clear_line(11);
  screen_puts(0, 11, "EJECTING ALL.. WAIT");
}

void screen_hosts_and_devices_clear_host_slot(unsigned char i)
{
  // nothing to do, edit_line handles clearing correct space on screen, and doesn't touch the list numbers
}

void screen_hosts_and_devices_edit_host_slot(unsigned char i)
{
  // nothing to do, edit_line handles clearing correct space on screen, and doesn't touch the list numbers
}

void screen_hosts_and_devices_eject(unsigned char ds)
{
}

void screen_hosts_and_devices_host_slot_empty(unsigned char hs)
{
}

void screen_hosts_and_devices_long_filename(char *f)
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

void screen_dlist_select_file(void)
{
}

void screen_dlist_select_slot(void)
{
}

void screen_dlist_show_info(void)
{
}

void screen_dlist_set_wifi(void)
{
}

void screen_dlist_mount_and_boot(void)
{
}

void screen_connect_wifi(NetConfig *nc)
{
}

void screen_dlist_hosts_and_devices(void)
{
}

#endif
