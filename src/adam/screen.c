#ifdef BUILD_ADAM
/**
 * #FujiNet CONFIG FOR #adam
 *
 * Screen Routines
 */

#include "screen.h"
#include "globals.h"
#include "bar.h"
#include <msx.h>
#include <smartkeys.h>
#include <conio.h>
#include <sys/ioctl.h>
#include <eos.h>
#include <string.h>

static char udg[] =
  {
   0,0,0,0,0,0,3,51,                               // WIFI 1
   0,0,3,3,51,51,51,51,                            // WIFI 2
   48,48,48,48,48,48,48,48,                        // WIFI 3
   0x00,0x07,0x08,0x0F,0x0F,0x0F,0x0F,0x0F,        // FOLDER 1 0x83
   0x00,0x80,0x70,0xF0,0xF0,0xF0,0xF0,0xF0,        // FOLDER 2 0x84
   0x0F,0x08,0x08,0x0A,0x08,0x08,0x0F,0x00,        // DDP 1    0x85
   0xF0,0x10,0x10,0x50,0x10,0x10,0xF0,0x00,        // DDP 2    0x86
   0x0F,0x08,0x08,0x08,0x0A,0x08,0x0F,0x00,        // DSK 1    0x87
   0xF0,0x10,0xD0,0xD0,0xD0,0x10,0xF0,0x00,        // DSK 2    0x88
   0x0F,0x08,0x0B,0x0B,0x0B,0x08,0x0F,0x00,        // ROM 1    0x89
   0xF0,0x10,0xD0,0xD0,0xD0,0x10,0xF0,0x00,        // ROM 2    0x8a
   0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55  // Password smudge 0x8b
  };

static const char *empty="Empty";

void screen_init(void)
{
  void *param = &udg;
  console_ioctl(IOCTL_GENCON_SET_UDGS,&param);
  smartkeys_set_mode();
  smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
  smartkeys_status("  WELCOME TO #FUJINET");
  eos_start_read_keyboard();
}

void screen_debug(char *message)
{
  gotoxy(0,0); cprintf(message); 
}

void screen_error(const char *c)
{
  smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
  smartkeys_status(c);
}

void screen_set_wifi(AdapterConfig *ac)
{
  smartkeys_set_mode();
  msx_vfill(MODE2_ATTR,0xF5,0x100);
  msx_vfill(MODE2_ATTR+0x100,0x1F,0x1100);
  msx_vfill_v(MODE2_ATTR,0xF5,144);
  msx_vfill_v(MODE2_ATTR+8,0xF5,144);
  msx_vfill_v(MODE2_ATTR+16,0xF5,144);
  msx_color(15,5,7); gotoxy(8,0); cprintf("MAC: %02X:%02X:%02X:%02X:%02X:%02X",ac->macAddress[0],ac->macAddress[1],ac->macAddress[2],ac->macAddress[3],ac->macAddress[4],ac->macAddress[5]);

  smartkeys_display(NULL,NULL,NULL,NULL,NULL,"   SKIP");
  smartkeys_status("  SCANNING FOR NETWORKS...");
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
  
  msx_color(15,5,7);
  gotoxy(0,n+1); cprintf("%s",meter);
  msx_color(1,15,7);
  cprintf("%s",ds);
}

void screen_set_wifi_select_network(unsigned char nn)
{
  smartkeys_display(NULL,NULL,NULL," HIDDEN\n  SSID"," RESCAN","  SKIP");
  sprintf(response,"  WELCOME TO #FUJINET!\n  %u NETWORKS FOUND\n  SELECT A NETWORK.",nn);
  smartkeys_status(response);
}

void screen_set_wifi_custom(void)
{
  smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
  smartkeys_status("  ENTER NAME OF HIDDEN NETWORK");
}

void screen_set_wifi_password(void)
{
  smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
  smartkeys_status("  ENTER NETWORK PASSWORD\n  AND PRESS [RETURN]");
}

void screen_connect_wifi(NetConfig *nc)
{
  smartkeys_set_mode();
  
  smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
  sprintf(response,"  CONNECTING TO NETWORK\n  %s",nc->ssid);
  smartkeys_status(response);
}

char* screen_hosts_and_devices_slot(char *c)
{
  return c[0]==0x00 ? &empty[0] : c;
}

void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *d)
{
  for (char i=0;i<4;i++)
    {
      gotoxy(0,i+y); cprintf("%d%s",i+1,screen_hosts_and_devices_slot(d[i].file));
    }
}

void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d)
{
  smartkeys_set_mode();
  eos_start_read_keyboard();
  gotoxy(0,0);  cprintf("%32s","HOST SLOTS");
  gotoxy(0,11); cprintf("%32s","DISK SLOTS");
  
  for (char i=0;i<8;i++)
    {
      gotoxy(0,i+1); cprintf("%d%s",i+1,screen_hosts_and_devices_slot(h[i])); 
    }

  screen_hosts_and_devices_device_slots(12,d);
  
  msx_vfill(MODE2_ATTR,0xF4,256);
  msx_vfill(MODE2_ATTR+0x0B00,0xF4,256);
  msx_vfill(MODE2_ATTR+0x0100,0x1F,2048);
  msx_vfill(MODE2_ATTR+0x0C00,0x1F,1024);
  msx_vfill_v(MODE2_ATTR+0x0100,0xF4,64);
  msx_vfill_v(MODE2_ATTR+0x0C00,0xF4,32);
}

// shown on initial screen
void screen_hosts_and_devices_hosts(void)
{
  bool slot_1_occupied = deviceSlots[0].file[0] != 0x00;

  smartkeys_display(NULL,NULL,NULL,"  SHOW\n CONFIG","  EDIT\n  SLOT",slot_1_occupied ? " SLOT 1\n   BOOT" : NULL);
  smartkeys_status("  [RETURN] SELECT HOST\n  [1-8] SELECT SLOT\n  [TAB] GO TO DISK SLOTS");
  bar_clear(false);
  bar_set(0,1,8,selected_host_slot);
}

void screen_hosts_and_devices_devices(void)
{
  smartkeys_display(NULL,NULL,NULL," EJECT","  READ\n  ONLY","  READ\n WRITE");
  smartkeys_status("  [TAB] GO TO HOST SLOTS");
  bar_clear(false);
  bar_set(11,1,4,selected_device_slot);
}

void screen_hosts_and_devices_clear_host_slot(unsigned char i)
{
  gotoxy(1,i+1);
  msx_vfill((i+1)*0x0100+8,0x00,248);
}

void screen_hosts_and_devices_edit_host_slot(unsigned char i)
{
  smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
  sprintf(response,"  EDIT THE HOST NAME FOR SLOT %u\n  PRESS [RETURN] WHEN DONE.",i+1);
  smartkeys_status(response);
}

void screen_hosts_and_devices_long_filename(char *f)
{
  if (strlen(f)>31)
    {
      gotoxy(0,17);
      cprintf("%s",f);
    }
  else
    msx_vfill(0x1100,0x00,1024);
}

void screen_show_info(AdapterConfig* ac)
{
  smartkeys_set_mode();

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

  msx_vfill(MODE2_ATTR+0x0700,0xF4,256);
  msx_vfill(MODE2_ATTR+0x0800,0x1F,256);
  
  for (char i=0;i<7;i++)
    {
      msx_vfill(MODE2_ATTR+(i*256)+0x900,0xF4,80);
      msx_vfill(MODE2_ATTR+(i*256)+0x900+80,0x1F,176);
    }
  smartkeys_display(NULL,NULL," KEYBD?\n  YES","PRINTER?\n  YES"," CHANGE\n  SSID","RECONNECT");    
}

void screen_select_file(void)
{
  smartkeys_set_mode();
  msx_color(15,4,7);
  msx_vfill(MODE2_ATTR,0xF4,512);

  // Paint content area
  msx_vfill(MODE2_ATTR+0x200,0xF5,256);
  msx_vfill(MODE2_ATTR+0x300,0x1F,0x0F00);
  smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
  smartkeys_status("  OPENING...");
}

void screen_select_file_display(char *p, char *f)
{
  // Clear content area
  msx_vfill(0x0000,0x00,0x1400);
  msx_vfill(MODE2_ATTR+0x0100,0xF5,256);
  msx_vfill(MODE2_ATTR+0x1200,0xF5,256);
  msx_vfill_v(MODE2_ATTR+0x0200,0xF5,136);
  msx_vfill_v(MODE2_ATTR+0x0200+8,0xF5,136);

  // Update content area
  msx_color(15,4,7);
  gotoxy(0,0); cprintf("%32s",selected_host_name);

  if (f[0]==0x00)
    cprintf("%32s",p);
  else
    cprintf("%22s|%8s|",p,f);

}

void screen_select_file_prev(void)
{
  msx_color(1,5,7);
  gotoxy(0,2); cprintf("%32s","[...]");
}

void screen_select_file_next(void)
{
  msx_color(1,5,7);
  gotoxy(0,18); cprintf("%32s","[...]");
}

void screen_select_file_display_entry(unsigned char y, char* e)
{
  gotoxy(0,y+3);
  msx_color(15,5,7);
  cprintf("%c%c",*e++,*e++);
  msx_color(1,15,7);
  cprintf("%-30s",e);
}

// Shown on directory screen
void screen_select_file_choose(char visibleEntries)
{
  bool slot_1_occupied = deviceSlots[0].file[0] != 0x00;

  bar_set(2,2,visibleEntries,0); // TODO: Handle previous

  smartkeys_display(NULL,NULL,NULL,(strcmp(path,"/") == 0) ? NULL: "   UP"," FILTER", slot_1_occupied ? " SLOT 1\n BOOT" : "  QUICK\n  BOOT");
  smartkeys_status("  SELECT FILE TO MOUNT\n  [INSERT] CREATE NEW\n  [ESC] ABORT");
}

void screen_select_file_filter(void)
{
  smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
  smartkeys_status("  ENTER A WILDCARD FILTER.\n  E.G. *Coleco*");
}

void screen_select_file_new_type(void)
{
  smartkeys_display(NULL,NULL,NULL,NULL,"  DDP"," DISK");
  smartkeys_status("  NEW MEDIA:\n  SELECT MEDIA TYPE.");
}

void screen_select_file_new_size(unsigned char k)
{
  if (k==1) // DDP
    smartkeys_display(NULL,NULL," 128K", " 256K", " 320K", " CUSTOM");
  else if (k==2) // DSK
    smartkeys_display(NULL,NULL," 160K"," 320K","8192K"," CUSTOM");
  smartkeys_status("  SIZE?");
}

void screen_select_file_new_custom(void)
{
  smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
  smartkeys_status("  PLEASE ENTER DESIRED CUSTOM SIZE\n  IN NUMBER OF 1K BLOCKS\n");
}

void screen_select_file_new_name(void)
{
  smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
  smartkeys_status("  PLEASE ENTER A FILENAME\n  FOR THIS DISK/DDP:");
}

void screen_select_file_new_creating(void)
{
  smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
  smartkeys_status("  CREATING FILE... PLEASE WAIT.");
}

void screen_select_slot(char *e)
{
  unsigned long *s;
  
  smartkeys_set_mode();

  gotoxy(0,7);
  cprintf("%32s","FILE DETAILS");
  cprintf("%8s 20%02u-%02u-%02u %02u:%02u:%02u\n","MTIME:",*e++,*e++,*e++,*e++,*e++,*e++);
  
  s=(unsigned long *)e; // Cast the next four bytes as a long integer.
  
  cprintf("%8s %lu K\n","SIZE:",*s >> 10); // Quickly divide by 1024

  e += sizeof(unsigned long) + 2; // I do not need the last two bytes.
  
  gotoxy(0,0);
  cprintf("%32s",e);

  screen_hosts_and_devices_device_slots(1,&deviceSlots[0]);
  
  msx_vfill(MODE2_ATTR,0xF4,256);
  msx_vfill(MODE2_ATTR+0x100,0x1F,0x400);
  msx_vfill_v(MODE2_ATTR,0xF4,40);

  msx_vfill(MODE2_ATTR+0x700,0xF6,320);
  msx_vfill(MODE2_ATTR+0x800+64,0x1F,192);
  msx_vfill(MODE2_ATTR+0x900,0xF6,64);
  msx_vfill(MODE2_ATTR+0x900+64,0x1F,192);
  
  bar_set(0,1,4,0);
}

void screen_select_slot_choose(void)
{
  smartkeys_display(NULL,NULL,NULL," EJECT"," READ\n ONLY","  READ\n  WRITE");
  smartkeys_status(" [1-4] SELECT SLOT\n [RETURN] INSERT INTO SLOT\n [ESC] TO ABORT.");
}

void screen_select_slot_eject(unsigned char ds)
{
  msx_vfill(0x0100+(ds<<8)+8,0x00,248);
  gotoxy(1,1+ds); cprintf("%s",empty);
  bar_jump(bar_get());
}

void screen_hosts_and_devices_eject(unsigned char ds)
{
  msx_vfill(0x0c00+(ds<<8)+8,0x00,248);
  gotoxy(1,12+ds); cprintf(empty);
  bar_jump(bar_get());
}

void screen_hosts_and_devices_host_slot_empty(unsigned char hs)
{
  gotoxy(1,1+hs); cprintf(empty);
}  

void screen_select_slot_build_eos_directory(void)
{
  smartkeys_display(NULL,NULL,NULL,NULL,"  YES","   NO");
  smartkeys_status("  DO YOU WISH TO WRITE\n  AN EOS DIRECTORY\n  TO THIS IMAGE?");
}

void screen_select_slot_build_eos_directory_label(void)
{
  smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
  smartkeys_status("  ENTER A VOLUME LABEL. (12 CHARACTERS MAX)");
}

void screen_select_slot_build_eos_directory_creating(void)
{
  smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
  smartkeys_status("  CREATING THE DIRECTORY.\n  PLEASE WAIT.");
}

#endif /* BUILD_ADAM */

