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
extern DeviceSlot deviceSlots[NUM_DEVICE_SLOTS];
extern HostSlot hostSlots[8];

char uppercase_tmp[32]; // temp space for strupr(s) output.
                               // so original strings doesn't get changed.

char *screen_upper(char *s)
{
    memset(uppercase_tmp,0,sizeof(uppercase_tmp));
    strcpy(uppercase_tmp,s);

    return strupr(uppercase_tmp);
}

int screen_offset(int x, int y)
{
  return (y * 32) + x;
}

void screen_add_shadow(int y, int c)
{
  unsigned char *p = (unsigned char *)SCREEN_RAM_TOP + screen_offset(0,y);

  *p = (unsigned char)c | 0x0b;  
  memset(p+1,c | 0x03, 31);
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
  cls(1);
  printf("MOUNTING ALL SLOTS...\n");
}

void screen_set_wifi(AdapterConfig *ac)
{
  cls(6);
  locate(0,0);
  printf("%32s","WELCOME TO FUJINET");
  printf("%15s%02x:%02x:%02x:%02x:%02x:%02x","MAC:",
	 ac->macAddress[0],ac->macAddress[1],ac->macAddress[2],ac->macAddress[3],ac->macAddress[4],ac->macAddress[5]);
  printf("%32s","SCANNING FOR NETWORKS...");

  screen_add_shadow(3,CYAN);
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

  locate(0,n+2);  printf("%-32s",screen_upper(ds));
  locate(28,n+2); printf("%s",meter);
}

void screen_set_wifi_select_network(unsigned char nn)
{
  locate(0,14);
  printf("        up/down TO SELECT       ");
  printf("hIDDEN SSID rESCAN enter SELECT");
  bar_draw(0,false);
  bar_set(2,1,nn,0);

  screen_add_shadow(nn+2,CYAN);
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
void screen_show_info(int printerEnabled, AdapterConfig *ac)
{
  cls(7);
  printf("     FUJINET CONFIGURATION      ");
  printf("%32s","SSID:");
  printf("%32s",screen_upper(ac->ssid));
  printf("%32s","HOSTNAME:");
  printf("%32s",screen_upper(ac->hostname));
  printf("%10s%u.%u.%u.%u\n","IP: ",ac->localIP[0],ac->localIP[1],ac->localIP[2],ac->localIP[3]);
  printf("%10s%u.%u.%u.%u\n","NETMASK: ",ac->netmask[0],ac->netmask[1],ac->netmask[2],ac->netmask[3]);
  printf("%10s%u.%u.%u.%u\n","DNS: ",ac->dnsIP[0],ac->dnsIP[1],ac->dnsIP[2],ac->dnsIP[3]);
  printf("%10s%02X:%02X:%02X:%02X:%02X:%02X\n","MAC: ",ac->macAddress[0],ac->macAddress[1],ac->macAddress[2],ac->macAddress[3],ac->macAddress[4],ac->macAddress[5]);
  printf("%10s%02X:%02X:%02X:%02X:%02X:%02X\n","BSSID: ",ac->bssid[0],ac->bssid[1],ac->bssid[2],ac->bssid[3],ac->bssid[4],ac->bssid[5]);
  printf("%10s%s","FNVER: ",screen_upper(ac->fn_version));
  printf("\n\n\n");
  printf(" cHANGE SSID          rECONNECT ");
  printf("OR  ANY KEY  TO RETURN TO HOSTS");

  bar_draw(1,false);
  bar_draw(3,false);

  screen_add_shadow(12,PURPLE);
  screen_add_shadow(15,PURPLE);
}

void screen_select_slot(const char *e)
{
  unsigned long *s;

  struct _additl_info
  {
    byte year;
    byte month;
    byte day;
    byte hour;
    byte min;
    byte sec;
    unsigned long size;
    byte isdir;
    byte trunc;
    byte type;
    byte *filename;
  } *i = (struct _additl_info *)e;
  
  cls(4);

  printf("%32s","PLACE IN DEVICE SLOT:");
  bar_draw(0,false);

  screen_hosts_and_devices_device_slots(1,&deviceSlots[0],&deviceEnabled[0]);

  locate(0,6);
  printf("%32s","FILE DETAILS");
  bar_draw(6,false);

  printf("%8s 20%02u-%02u-%02u %02u:%02u:%02u\n","MTIME:",i->year,i->month,i->day,i->hour,i->min,i->sec);

  printf("%8s %lu K\n","SIZE:",i->size >> 10); // Quickly divide by 1024

  printf("%96s",screen_upper((char *)&e[13]));

  locate(0,13);
  printf("   arrow keys  TO SELECT SLOT   ");
  printf(" enter R/O w R/W OR break ABORT ");

  screen_add_shadow(5,RED);
  screen_add_shadow(12,RED);
  screen_add_shadow(15,RED);
  
  bar_set(1,1,NUM_DEVICE_SLOTS,0);
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
  cls(8);
  printf("%32s","OPENING");

  screen_add_shadow(2,ORANGE);  
}

void screen_select_file_display(char *p, char *f)
{
  cls(8);
  locate(0,0); printf("%-32s",screen_upper(selected_host_name));
  locate(0,1);

  if (f[0]==0x00)
      printf("%-32s",screen_upper(p));
  else {
      printf("%-24s",screen_upper(p));
	  locate(24,1);
	  printf ("%8s",screen_upper(f));
  }
  screen_add_shadow(2,ORANGE);
}

void screen_select_file_display_long_filename(const char *e)
{
}

void screen_select_file_clear_long_filename(void)
{
}

void screen_select_file_filter(void)
{
    locate(0,14);
    printf("%-63s","ENTER FILTER:");
    locate(0,15);
}

void screen_select_file_next(void)
{
  screen_add_shadow(13,ORANGE);
  locate(12,13); printf("[...]");
}

void screen_select_file_prev(void)
{
  screen_add_shadow(2,ORANGE);
  locate(12,2); printf("[...]");
}

void screen_select_file_display_entry(unsigned char y, const char *e, unsigned entryType)
{
  locate(0,y+3);
  printf("%-32s",screen_upper((char *)e)); // skip the first two chars from FN (hold over from Adam)
}

void screen_select_file_choose(char visibleEntries)
{
  locate(0,14);
  printf("%-32s","_ ../ up/dn MOVE ^up/^dn PAGE");
  asm {
	PSHS	B
	LDB		#$1F
	STB		$5C0
	DECB
	STB		$5D1
	STB		$5D5
	PULS	B
  }
  
  if (copy_mode==true)
    {
      printf("%-31s","enter OR break ABORT cOPY");
    }
  else
    {
      printf("%-31s","enter OR break fILTER nEW cOPY");
    }

  bar_set(3,0,visibleEntries,0);

  if (visibleEntries<10)
    screen_add_shadow(3+visibleEntries,ORANGE);
}

void screen_select_file_new_type(void)
{
}

void screen_select_file_new_size(unsigned char k)
{
    locate(0,14);
    printf("%-63s","ENTER # OF DRIVES TO CREATE");
    locate(0,15);
}

void screen_select_file_new_custom(void)
{
}

void screen_select_file_new_name(void)
{
    locate(0,14);
    printf("%-63s","ENTER FILENAME:");
    locate(0,15);
}

void screen_select_file_new_creating(void)
{
    locate(0,13);
    printf("%-63s","CREATING IMAGE. PLEASE WAIT.");
    locate(0,14);
}

void screen_error(const char *msg)
{
  locate(0,15);
  printf("%-31s",msg);
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

// Show the keys that are applicable when we are on the Hosts portion of the screen.
void screen_hosts_and_devices_hosts()
{
  cls(3);
  locate(0,0);
  printf("%32s","host\x80slots");
  
  memset(0x400,0xAF,22);
  (*(unsigned char *)0x041a) = 0x20;

  locate(0,13);
  printf("1-8 slot Edit ENTER browse Lobby");
  printf("  Config  -> drives  BREAK quit");

  screen_add_shadow(9,BLUE);
  screen_add_shadow(15,BLUE);
    
  screen_hosts_and_devices_host_slots(&hostSlots[0]);
  bar_set(1,1,8,selected_host_slot);
}

// Show the keys that are applicable when we are on the Devices portion of the screen.
void screen_hosts_and_devices_devices()
{
  cls(4);
  locate(0,0); 
  printf("%32s","drive""\x80""slots");
  memset(0x400,0xBF,21);
  (*(unsigned char *)0x041a) = 0x20;
  
  locate(0,14);
  printf("\x80\x80\x80\x31-8slotEditENTERbrowseLobby\x80\x80\x80\x80\x80\x80\x43onfigTABdrivesBREAKboot\x80\x80\x80");

  locate(0,13);
  printf("1-8 slot Eject  CLEAR  all slots");
  printf("<- hosts Read Write Config Lobby");

  screen_add_shadow(15,RED);
  screen_add_shadow(5,RED);
    
  screen_hosts_and_devices_device_slots(1,&deviceSlots[0],NULL);
  bar_set(1,1,NUM_DEVICE_SLOTS,selected_device_slot);
}

void screen_hosts_and_devices_host_slots(HostSlot *h)
{
  byte *p = &h[0]; // We need this because cmoc doesn't like untangling multi-dimensional typedefs
  byte *sp = (unsigned char *)SCREEN_RAM_TOP;

  sp += 32;  // start one line down. 
  locate(0,1); 

  // Color the first column
  for (int i=0;i<8;i++)
    {
        printf("%u%-31s",i+1,screen_upper((char *)p));
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
    return hostSlot+'1';
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
  byte *sp = (unsigned char *)SCREEN_RAM_TOP;

  sp += 32;  // start one line down. 

  for (int i=0;i<NUM_DEVICE_SLOTS;i++)
    {
      locate(0,(unsigned char)i+1);
      printf("%u%c",i,host_slot_char(dslot->hostSlot));
      printf("%c",device_slot_mode(dslot->mode));
      printf("%-29s",screen_upper((char *)dslot->file));
      dslot++;
      *sp &= 0xBF;
      sp++;
      *sp &= 0xBF;
      sp += 31;
    }
}

void screen_hosts_and_devices_devices_clear_all(void)
{
  locate(0,11);
  printf("EJECTING ALL... PLEASE WAIT.");
}

void screen_hosts_and_devices_clear_host_slot(int i)
{
  // nothing to do, edit_line handles clearing correct space on screen, and doesn't touch the list numbers
}

void screen_hosts_and_devices_edit_host_slot(int i)
{
  // nothing to do, edit_line handles clearing correct space on screen, and doesn't touch the list numbers
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
  // TODO: figure out lowercase.
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
  printf("     CONNECTING TO NETWORK:     %32s",screen_upper(nc->ssid));

  screen_add_shadow(9,BLUE); // change to CYAN
}

#endif
