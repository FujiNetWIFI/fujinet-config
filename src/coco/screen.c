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
extern DeviceSlot deviceSlots[8];
extern HostSlot hostSlots[8];

int screen_offset(int x, int y)
{
  return (y * 32) + x;
}

byte screen_get(int x, int y)
{
  int o = screen_offset(x,y);
  unsigned char *p = (unsigned char *)SCREEN_RAM_TOP;

  p += o;

  return *p;
}

void screen_put(int x, int y, byte c)
{
  int o = screen_offset(x,y);
  byte *p = (unsigned char *)SCREEN_RAM_TOP;

  p += o;

  *p = c;
}

void screen_mount_and_boot()
{
}

void screen_set_wifi(AdapterConfigExtended *ac)
{
  cls(6);
  locate(0,0);
  printf("%32s","WELCOME TO #FUJINET");
  printf("%15s%02x:%02x:%02x:%02x:%02x:%02x","MAC:",
	 ac->macAddress[0],ac->macAddress[1],ac->macAddress[2],ac->macAddress[3],ac->macAddress[4],ac->macAddress[5]);
  printf("%32s","SCANNING FOR NETWORKS...");

  // Add line.
  (*(unsigned char *)0x460) = 0xDB;
  memset(0x461,0xD3,31);
}

void screen_set_wifi_display_ssid(char n, SSIDInfo *s)
{
  char meter[4]={0x20,0x20,0x20,0x00};
  char ds[32];

  memset(ds,0x20,32);
  strncpy(ds,s->ssid,32);

  if (s->rssi > -50)
    {
      meter[0] = '*';
      meter[1] = '*';
      meter[2] = '*';
    }
  else if (s->rssi > -70)
    {
      meter[0] = '*';
      meter[1] = '*';
    }
  else
    {
      meter[0] = '*';
    }

  locate(0,n+2);  printf("%-32s",ds);
  locate(28,n+2); printf("%s",meter);
}

void screen_set_wifi_select_network(unsigned char nn)
{
  locate(0,14);
  printf("        up/down TO SELECT       ");
  printf("hIDDEN SSID rESCAN enter SELECT");
  bar_draw(0,false);
  bar_set(2,1,nn,0);

  // Add line.
  (*(unsigned char *)0x5A0) = 0xDB;
  memset(0x5A1,0xD3,31);
}

void screen_set_wifi_custom(void)
{
  locate(0,14);
  printf("  ENTER NAME OF HIDDEN NETWORK  ");
  printf("%31s","");
}

void screen_set_wifi_password(void)
{
  locate (0,14);
  printf("ENTER NET PASSWORD, PRESS enter.");
  printf("%31s","");
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

void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d, unsigned char *e)
{
  switch(hd_subState)
    {
    case HD_HOSTS:
      screen_hosts_and_devices_hosts();
      break;
    case HD_DEVICES:
      screen_hosts_and_devices_devices();
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

  locate(0,13);
  printf("1-8 slot Edit ENTER browse Lobby");
  printf("  Config  -> drives  BREAK quit");

  // Add line
  (*(unsigned char *)0x5E0) = 0xAB;
  memset(0x5E1,0xA3,31);

  // Add line.
  (*(unsigned char *)0x520) = 0xAB;
  memset(0x521,0xA3,31);
  
  screen_hosts_and_devices_host_slots(&hostSlots[0]);
  bar_set(1,1,8,selected_host_slot);
}

// Show the keys that are applicable when we are on the Devices portion of the screen.
void screen_hosts_and_devices_devices()
{
  cls(4);
  locate(0,0); 
  printf("%32s","device""\x80""slots");
  memset(0x400,0x20,20);
  (*(unsigned char *)0x041a) = 0x20;
  
  locate(0,14);
  printf("\x80\x80\x80\x31-8slotEditENTERbrowseLobby\x80\x80\x80\x80\x80\x80\x43onfigTABdrivesBREAKboot\x80\x80\x80");

  locate(0,13);
  printf("1-8 slot Eject  CLEAR  all slots");
  printf("<- hosts Read Write Config Lobby");

  // Add line
  (*(unsigned char *)0x5E0) = 0xBB;
  memset(0x5E1,0xB3,31);

  // Add line.
  (*(unsigned char *)0x520) = 0xBB;
  memset(0x521,0xB3,31);
  
  screen_hosts_and_devices_host_slots(&hostSlots[0]);

  screen_hosts_and_devices_device_slots(1,&deviceSlots[0],NULL);
  bar_set(1,1,8,selected_device_slot);
}

void screen_hosts_and_devices_host_slots(HostSlot *h)
{
  char *p = &h[0]; // We need this because cmoc doesn't like untangling multi-dimensional typedefs
  char *sp = (unsigned char *)SCREEN_RAM_TOP;

  sp += 32;  // start one line down. 
  locate(0,1); 

  // Color the first column
  for (int i=0;i<8;i++)
    {
      printf("%u%-31s",i+1,p);
      p += 32;  // Next entry
      *sp &= INVERSE_MASK;
      sp += 32; // next line
    }
}

const char host_slot_char(unsigned char hostSlot)
{
  if (hostSlot==0xff)
    return ' ';
  else
    return hostSlot+'0';
}

const char device_slot_mode(unsigned char mode)
{
  switch(mode)
    {
    case 0:
      return 0x80;
    case 1:
      return 0xAF;
    case 2:
      return 0x9F;
    }
}

void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *dslot, unsigned char *e)
{
  char *sp = (unsigned char *)SCREEN_RAM_TOP;

  sp += 32;  // start one line down. 

  for (int i=0;i<8;i++)
    {
      locate(0,i+1);
      printf("%u%c",i+1,host_slot_char(dslot->hostSlot));
      printf("%c",device_slot_mode(dslot->mode));
      printf("%-29s",dslot->file);
      dslot++;
      *sp &= 0xBF;
      sp++;
      *sp &= 0xBF;
      sp += 31;
    }
}

void screen_hosts_and_devices_devices_clear_all(void)
{
  /* screen_clear_line(11); */
  /* screen_puts(0, 11, "EJECTING ALL.. WAIT"); */
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

void screen_connect_wifi(NetConfig *nc)
{
  cls(3);
  locate(0,7);
  printf("     CONNECTING TO NETWORK:     %32s",nc->ssid);
}

void screen_dlist_hosts_and_devices(void)
{
}

#endif
