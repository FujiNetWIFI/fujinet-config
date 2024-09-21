#ifdef BUILD_PC6001
/**
 * FujiNet CONFIG FOR #adam
 *
 * Screen Routines
 */

#include "screen.h"
#include "globals.h"
#include "bar.h"
#include <conio.h>
#include <sys/ioctl.h>
#include <eos.h>
#include <string.h>

extern bool copy_mode;

static const char *empty="Empty";

void screen_init(void)
{
}

void screen_debug(char *message)
{
  gotoxy(0,0); cprintf(message); 
}

void screen_error(const char *c)
{
}

void screen_set_wifi(AdapterConfig *ac)
{
  gotoxy(8,0);
  cprintf("MAC: %02X:%02X:%02X:%02X:%02X:%02X",ac->macAddress[0],ac->macAddress[1],ac->macAddress[2],ac->macAddress[3],ac->macAddress[4],ac->macAddress[5]);
}

void screen_set_wifi_display_ssid(char n, SSIDInfo *s)
{
  char meter[4]={0x20,0x20,0x20,0x00};
  char ds[32];

  memset(ds,0x20,28);
  strncpy(ds,s->ssid,28);
  
  if (s->rssi > -40)
    {
      meter[0] = 0x80;
      meter[1] = 0x81;
      meter[2] = 0x82;
    }
  else if (s->rssi > -60)
    {
      meter[0] = 0x80;
      meter[1] = 0x81;
    }
  else
    {
      meter[0] = 0x80;
    }

  gotoxy(0,n+1); cprintf("%s",meter);
  cprintf("%s",ds);
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

void screen_connect_wifi(NetConfig *nc)
{
}

char* screen_hosts_and_devices_slot(char *c)
{
  return c[0]==0x00 ? &empty[0] : c;
}

void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *d)
{
  gotoxy(0,11); cprintf("%32s","DISK SLOTS");

  for (char i=0;i<4;i++)
    {
      gotoxy(0,i+y); cprintf("%d%s",i+1,screen_hosts_and_devices_slot(d[i].file));
    }
}

void screen_hosts_and_devices_host_slots(HostSlot *h)
{
  gotoxy(0,0);  cprintf("%32s","HOST SLOTS");

  for (char i=0;i<8;i++)
    {
      gotoxy(0,i+1); cprintf("%d%s",i+1,screen_hosts_and_devices_slot(h[i])); 
    }  
}

void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d)
{
  clrscr();

  screen_hosts_and_devices_host_slots(h);
  screen_hosts_and_devices_device_slots(12,d);
  
}

// shown on initial screen
void screen_hosts_and_devices_hosts(void)
{
  bool slot_1_occupied = deviceSlots[0].file[0] != 0x00;
}

void screen_hosts_and_devices_devices(void)
{
}

void screen_hosts_and_devices_devices_clear_all(void)
{
}

void screen_hosts_and_devices_clear_host_slot(unsigned char i)
{
}

void screen_hosts_and_devices_edit_host_slot(unsigned char i)
{
}

void screen_hosts_and_devices_long_filename(char *f)
{
  if (strlen(f)>31)
    {
      gotoxy(0,17);
      cprintf("%s",f);
    }
}

void screen_show_info(AdapterConfig* ac)
{
  clrscr();
  gotoxy(0,7);
  
  cprintf("%32s","SSID");
  cprintf("%32s",ac->ssid);
  cprintf("%10s%s\n","HOSTNAME:",ac->hostname);
  cprintf("%10s%u.%u.%u.%u\n","IP:",ac->localIP[0],ac->localIP[1],ac->localIP[2],ac->localIP[3]);
  cprintf("%10s%u.%u.%u.%u\n","NETMASK:",ac->netmask[0],ac->netmask[1],ac->netmask[2],ac->netmask[3]);
  cprintf("%10s%u.%u.%u.%u\n","DNS:",ac->dnsIP[0],ac->dnsIP[1],ac->dnsIP[2],ac->dnsIP[3]);
  cprintf("%10s%02x:%02x:%02x:%02x:%02x:%02x\n","MAC:",ac->macAddress[0],ac->macAddress[1],ac->macAddress[2],ac->macAddress[3],ac->macAddress[4],ac->macAddress[5]);
  cprintf("%10s%02x:%02x:%02x:%02x:%02x:%02x\n","BSSID:",ac->bssid[0],ac->bssid[1],ac->bssid[2],ac->bssid[3],ac->bssid[4],ac->bssid[5]);
  cprintf("%10s%s\n","FNVER:",ac->fn_version);  
}

void screen_select_file(void)
{
}

void screen_select_file_display(char *p, char *f)
{
  // Update content area
  gotoxy(0,0); cprintf("%32s",copy_mode == true ? copy_host_name : selected_host_name);

  if (f[0]==0x00)
    cprintf("%32s",p);
  else
    cprintf("%22s|%8s|",p,f);
}

void screen_select_file_display_long_filename(char *e)
{
  gotoxy(0,19);
  cprintf("%-64s",e);
}

void screen_select_file_clear_long_filename(void)
{
  gotoxy(0,0);
}

void screen_select_file_prev(void)
{
  gotoxy(0,2); cprintf("%32s","[...]");
}

void screen_select_file_next(void)
{
  gotoxy(0,18); cprintf("%32s","[...]");
}

void screen_select_file_display_entry(unsigned char y, char* e, unsigned entryType)
{
  gotoxy(0,y+3);
  cprintf("%c%c",*e++,*e++);
  cprintf("%-30s",e);
}

// Shown on directory screen
void screen_select_file_choose(char visibleEntries)
{
  bool slot_1_occupied = deviceSlots[0].file[0] != 0x00;

  bar_set(2,2,visibleEntries,0); // TODO: Handle previous
}

void screen_select_file_filter(void)
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

void screen_select_slot(char *e)
{
  unsigned long *s;

  clrscr();

  gotoxy(0,7);
  cprintf("%32s","FILE DETAILS");
  cprintf("%8s 20%02u-%02u-%02u %02u:%02u:%02u\n","MTIME:",*e++,*e++,*e++,*e++,*e++,*e++);
  
  s=(unsigned long *)e; // Cast the next four bytes as a long integer.
  
  cprintf("%8s %lu K\n","SIZE:",*s >> 10); // Quickly divide by 1024

  e += sizeof(unsigned long) + 2; // I do not need the last two bytes.
  
  gotoxy(0,0);
  cprintf("%32s",e);

  screen_hosts_and_devices_device_slots(1,&deviceSlots[0]);  
}

void screen_select_slot_choose(void)
{
}

void screen_select_slot_eject(unsigned char ds)
{
  gotoxy(1,1+ds); cprintf("%s",empty);
  bar_jump(bar_get());
}

void screen_hosts_and_devices_eject(unsigned char ds)
{
  gotoxy(1,12+ds); cprintf(empty);
  bar_jump(bar_get());
}

void screen_hosts_and_devices_host_slot_empty(unsigned char hs)
{
  gotoxy(1,1+hs); cprintf(empty);
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

void screen_destination_host_slot(char *h, char *p)
{
  clrscr();
  gotoxy(0,10); cprintf("%32s","COPY FROM HOST SLOT");
  gotoxy(0,11); cprintf("%32s",h);
  gotoxy(0,12); cprintf("%-128s",p);
}

void screen_destination_host_slot_choose(void)
{
  gotoxy(0,0); cprintf("%32s","COPY TO HOST SLOT");
  bar_set(0,1,8,selected_host_slot);
}

void screen_perform_copy(char *sh, char *p, char *dh, char *dp)
{
  clrscr();
  gotoxy(0,0); cprintf("%32s","COPYING FILE FROM:");
  gotoxy(0,1); cprintf("%32s",sh);
  gotoxy(0,2); cprintf("%-128s",p);
  gotoxy(0,6); cprintf("%32s",dh);
  gotoxy(0,7); cprintf("%-128s",dp);
  while(1);
}

#endif /* BUILD_PC6001 */

