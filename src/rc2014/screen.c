#ifdef BUILD_RC2014
/**
 * #FujiNet CONFIG FOR #adam
 *
 * Screen Routines
 */

#include "screen.h"
#include "globals.h"
#include "bar.h"
#include <conio.h>
#include <sys/ioctl.h>
#include <string.h>

#define MAX_DISK_SLOTS (4)
#define STR_MAX_DISK_SLOTS "4"
#define STATUS_BAR 20

extern bool copy_mode;
extern char copy_host_name[32];
extern unsigned char copy_host_slot;
extern bool deviceEnabled[8];

static const char *empty="EMPTY";
static const char *off="OFF";

void screen_inverse_line(unsigned char y)
{
  char i;

//  for (i=0;i<40;i++)
//    ram[bar_coord(i,y)] &= 0x3f; // black char on white background is in lower half of char set
}

void screen_put_inverse(const char c)
{
  cprintf("%c[7m", 27);
  cputc(c);
  cprintf("%c[0m", 27);
}

void screen_print_inverse(const char *s)
{
  char i;
  for (i=0;i< strlen(s);i++)
  {
    screen_put_inverse(s[i]);
  }
}

void screen_print_menu(const char *si, const char *sc)
{
  screen_print_inverse(si);
  cprintf(sc);
}

void screen_clrscr(void)
{
  cprintf("%c[2J", 27);
}

void screen_gotoxy(char x, char y)
{
  cprintf("%c[%d;%dH", 27, y+1, x+1);
}

void screen_set_cursor(bool enable)
{
}

void screen_clearxy(char x, char y, int len)
{
  screen_gotoxy(x,y);
  for (int i = 0; i < len; i++)
    cputc(' ');
}

void screen_clear_status(void)
{
  screen_gotoxy(0,STATUS_BAR);
  cprintf("%c[2K", 27);
  screen_gotoxy(0,STATUS_BAR+1);
  cprintf("%c[2K", 27);
  screen_gotoxy(0,STATUS_BAR+2);
  cprintf("%c[2K", 27);
}

void chlinexy(char x, char y, int len)
{
  screen_gotoxy(x,y);
  for (int i = 0; i < len; i++)
    cputc('-');
}

void screen_debug(char *message)
{
  screen_gotoxy(0,0); cprintf(message);
}

void screen_error(const char *c)
{
  screen_clear_status();
  screen_gotoxy(0,STATUS_BAR + 1); cprintf("%-40s",c);
  screen_inverse_line(STATUS_BAR + 1);
}

void screen_init(void)
{
  screen_clrscr();
}

void screen_set_wifi(AdapterConfig *ac)
{
  screen_clrscr();
  screen_gotoxy(9,0); cprintf("WELCOME TO #FUJINET!");
  screen_gotoxy(0,2); cprintf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X",
    ac->macAddress[0],
    ac->macAddress[1],
    ac->macAddress[2],
    ac->macAddress[3],
    ac->macAddress[4],
    ac->macAddress[5]);
  screen_gotoxy(7,STATUS_BAR); cprintf("SCANNING FOR NETWORKS...");
}

void screen_set_wifi_display_ssid(char n, SSIDInfo *s)
{
  char meter[4]={0x20,0x20,0x20,0x00};
  char ds[SSID_MAXLEN+1];

  memset(ds,0x20,32);
  strncpy(ds,s->ssid,32);
  ds[32] = 0; // force null termination

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

  screen_gotoxy(2,n+4); cprintf("%s",ds);
  screen_gotoxy(36,n+4); cprintf("%s",meter);

}

void screen_set_wifi_select_network(unsigned char nn)
{
  screen_clear_status();
  bar_set(4,1,nn,0);
  screen_gotoxy(10,STATUS_BAR); cprintf("FOUND %d NETWORKS.\r\n", nn);
  screen_gotoxy(0, STATUS_BAR + 1);
  screen_print_menu("H","IDDEN SSID  ");
  screen_print_menu("R","ESCAN  ");
  screen_print_menu("S","KIP");
  screen_gotoxy(40, STATUS_BAR + 2);
  screen_print_menu("RETURN"," TO SELECT");
}

void screen_set_wifi_custom(void)
{
  screen_clear_status();
  screen_gotoxy(0,STATUS_BAR); cprintf("ENTER NAME OF HIDDDEN NETWORK");
}

void screen_set_wifi_password(void)
{
//  char ostype;
//  ostype = get_ostype() & 0xF0;

  screen_clear_status();
  screen_gotoxy(0,STATUS_BAR); cprintf("ENTER NET PASSWORD AND PRESS [RETURN]");
//  if (ostype == APPLE_II)
//  {
//    screen_gotoxy(0, STATUS_BAR - 1); cprintf("USE [ESC] TO SWITCH TO UPPER/LOWER CASE");
//  }
//  screen_gotoxy(0, STATUS_BAR + 1); cputc(']');
}

void screen_connect_wifi(NetConfig *nc)
{
  screen_clear_status();
  screen_gotoxy(0,STATUS_BAR); cprintf("CONNECTING TO NETWORK: %s",nc->ssid);
}

void screen_destination_host_slot(char *h, char *p)
{
  screen_clrscr();
  screen_gotoxy(0, 12); cprintf("COPY FROM HOST SLOT");
  chlinexy(0,13,40);
  screen_gotoxy(0, 14); cprintf("%-32s", h);
  screen_gotoxy(0, 15); cprintf("%-128s", p);
}

void screen_destination_host_slot_choose(void)
{
  screen_gotoxy(0, 0);
  cprintf("COPY TO HOST SLOT");
  chlinexy(0,1,40);

  screen_clear_status();
  screen_gotoxy(0,STATUS_BAR);
  screen_print_menu("1-8",":CHOOSE SLOT\r\n");
  screen_print_menu("RETURN",":SELECT SLOT\r\n");
  screen_print_menu("ESC"," TO ABORT");

  bar_set(2, 1, 8, selected_host_slot);
}

const char* screen_hosts_and_devices_device_slot(unsigned char hs, bool e, char *fn)
{
  if (fn[0]!=0x00)
    return fn;
  else if (e==false)
    return &off[0];
  else
    return &empty[0];
}

char* screen_hosts_and_devices_slot(char *c)
{
  return c[0]==0x00 ? "EMPTY" : c;
}

void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *d, bool *e)
{
  char i;

  for (i=0;i<4;i++)
    {
      screen_gotoxy(2,i+y); cprintf("%d %s",i+1,screen_hosts_and_devices_device_slot(d[i].hostSlot,e[i],d[i].file));
    }
}

void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d, bool *e)
{
  const char hl[] = "HOST LIST";
  const char ds[] = "DRIVE SLOTS";
  char i;

  screen_clrscr();
  screen_gotoxy(0,0);  cprintf("%40s",hl); // screen_inverse(0);
  chlinexy(0,0,40 - sizeof(hl));

  screen_gotoxy(0,10); cprintf("%40s",ds); // screen_inverse(10);
  chlinexy(0,10,40 - sizeof(ds));

  for (i=0;i<8;i++)
  {
    screen_gotoxy(2,i+1); cprintf("%d %s",i+1,screen_hosts_and_devices_slot(h[i]));
  }

  screen_hosts_and_devices_device_slots(11,d,e);
}

void screen_hosts_and_devices_hosts(void)
{
  bar_set(1,1,8,0);
  screen_clear_status();
  // screen_gotoxy(0,STATUS_BAR); cprintf("[C]ONFIG  [E]DIT SLOT  [ESC]BOOT\r\n[1-8]HOST SLOT  [RETURN]SELECT SLOT\r\n[TAB] DEVICE SLOTS");
  screen_gotoxy(0,STATUS_BAR);
  screen_print_menu("1-8", ":SLOT  ");
  screen_print_menu("E","DIT  ");
  screen_print_menu("RETURN",":SELECT FILES\r\n ");
  screen_print_menu("C","ONFIG  ");
  screen_print_menu("TAB",":DRIVE SLOTS  ");
  screen_print_menu("ESC",":BOOT");
  //cprintf("[1-8]SLOT  [E]DIT  [RETURN]SELECT FILES\r\n [C]ONFIG  [TAB]DRIVE SLOTS  [ESC]BOOT");
}

void screen_hosts_and_devices_host_slots(HostSlot *h)
{
  char i;

  for (i=0;i<8;i++)
    {
      screen_gotoxy(0,i+1); cprintf("%d %-32s",i+1,screen_hosts_and_devices_slot(h[i]));
    }
}

void screen_hosts_and_devices_devices(void)
{
  bar_set(11,1,4,0);
  screen_clear_status();
  screen_gotoxy(0,STATUS_BAR);
  screen_print_menu("E","JECT  ");
  screen_print_menu("R","EAD ONLY  ");
  screen_print_menu("W","RITE\r\n");
  screen_print_menu("TAB",":HOST SLOTS");
}

void screen_hosts_and_devices_clear_host_slot(unsigned char i)
{
  screen_clearxy(3,i+1,39);
}

void screen_hosts_and_devices_edit_host_slot(unsigned char i)
{
  screen_clear_status();
  screen_gotoxy(0,STATUS_BAR); cprintf("EDIT THE HOST NAME FOR SLOT %d\r\nPRESS [RETURN] WHEN DONE.",i);
}

void screen_perform_copy(char *sh, char *p, char *dh, char *dp)
{
  screen_clrscr();
  screen_gotoxy(0,0); cprintf("%32s","COPYING FILE FROM:");
  screen_gotoxy(0,1); cprintf("%32s",sh);
  screen_gotoxy(0,2); cprintf("%-128s",p);
  screen_gotoxy(0,6); cprintf("%32s",dh);
  screen_gotoxy(0,7); cprintf("%-128s",dp);
}

void screen_show_info(bool printerEnabled, AdapterConfig* ac)
{
  screen_clrscr();

  screen_gotoxy(4,5);
  cprintf("F U J I N E T      C O N F I G");
  screen_gotoxy(0,8);
  cprintf("%10s%s\r\n","SSID: ",ac->ssid);
  cprintf("%10s%s\r\n","HOSTNAME: ",ac->hostname);
  cprintf("%10s%u.%u.%u.%u\r\n","IP: ",ac->localIP[0],ac->localIP[1],ac->localIP[2],ac->localIP[3]);
  cprintf("%10s%u.%u.%u.%u\r\n","NETMASK: ",ac->netmask[0],ac->netmask[1],ac->netmask[2],ac->netmask[3]);
  cprintf("%10s%u.%u.%u.%u\r\n","DNS: ",ac->dnsIP[0],ac->dnsIP[1],ac->dnsIP[2],ac->dnsIP[3]);
  cprintf("%10s%02x:%02x:%02x:%02x:%02x:%02x\r\n","MAC: ",ac->macAddress[0],ac->macAddress[1],ac->macAddress[2],ac->macAddress[3],ac->macAddress[4],ac->macAddress[5]);
  cprintf("%10s%02x:%02x:%02x:%02x:%02x:%02x\r\n","BSSID: ",ac->bssid[0],ac->bssid[1],ac->bssid[2],ac->bssid[3],ac->bssid[4],ac->bssid[5]);
  cprintf("%10s%s\r\n","FNVER: ",ac->fn_version);

  screen_gotoxy(6,STATUS_BAR);
  screen_print_menu("C","HANGE SSID  ");
  screen_print_menu("R","ECONNECT\r\n");
  cprintf("   PRESS ANY KEY TO RETURN TO HOSTS\r\n");
  cprintf("      FUJINET PRINTER %s",(printerEnabled) ? "ENABLED" : "DISABLED");
  }

void screen_select_file(void)
{
  screen_clear_status();
  screen_gotoxy(0,STATUS_BAR); cprintf("OPENING...");
}

void screen_select_file_display(char *p, char *f)
{
  screen_clrscr();

  // Update content area
  screen_gotoxy(0,0); cprintf("%-40s",selected_host_name);
  screen_gotoxy(3,1);
  if (f[0]==0x00)
    cprintf("%-40s",p);
  else
    cprintf("%-32s%8s",p,f);
}

void screen_select_file_prev(void)
{
  screen_gotoxy(3,2); cprintf("%-40s","[...]");
}

void screen_select_file_display_long_filename(char *e)
{
  screen_gotoxy(3,19);
  cprintf("%-64s",e);
}

void screen_select_file_next(void)
{
  screen_gotoxy(3,18); cprintf("%-40s","[...]");
}

void screen_select_file_display_entry(unsigned char y, char* e, unsigned entryType)
{
  screen_gotoxy(3,y+3);
  // cprintf("%c%c",*e++,*e++);
  // cprintf("%-30s",e);
  cprintf("%-40s",e);
}

void screen_select_file_clear_long_filename(void)
{
  // TODO: Implement
}

void screen_select_file_new_type(void)
{
  // TODO: Implement
}

void screen_select_file_new_size(unsigned char k)
{
  // TODO: implement
}

void screen_select_file_new_custom(void)
{
  // TODO: Implement
}

void screen_select_file_choose(char visibleEntries)
{
  bar_set(3,2,visibleEntries,0); // TODO: Handle previous
  screen_clear_status();
  screen_gotoxy(0,STATUS_BAR);
  screen_print_menu("RETURN",":SELECT FILE TO MOUNT\r\n");
  screen_print_menu("ESC",":PARENT  ");
  screen_print_menu("F","ILTER  ");
  screen_print_menu("ESC",":BOOT");
}

void screen_select_file_filter(void)
{
  screen_clear_status();
  screen_gotoxy(0,STATUS_BAR); cprintf("ENTER A WILDCARD FILTER.\r\n E.G. *Apple*");
}

void screen_select_slot(char *e)
{
  unsigned long *s;

  screen_clrscr();

  screen_gotoxy(0,7);
  cprintf("%-40s","FILE DETAILS");
  cprintf("%8s 20%02u-%02u-%02u %02u:%02u:%02u\r\n","MTIME:",*e++,*e++,*e++,*e++,*e++,*e++);

  s=(unsigned long *)e; // Cast the next four bytes as a long integer.
  cprintf("%8s %lu K\r\n\r\n","SIZE:",*s >> 10); // Quickly divide by 1024

  e += sizeof(unsigned long) + 2; // I do not need the next two bytes.
  cprintf("%-40s",e);

  screen_hosts_and_devices_device_slots(1,&deviceSlots[0],&deviceEnabled[0]);

  bar_set(1,1,4,0);
}

void screen_select_slot_choose(void)
{
  screen_clear_status();
  screen_gotoxy(3,STATUS_BAR);
  screen_print_menu("1-4"," SELECT SLOT OR USE ARROW KEYS\r\n ");
  screen_print_menu("RETURN/R",":INSERT READ ONLY\r\n ");
  screen_print_menu("W",":INSERT READ/WRITE\r\n ");
  screen_print_menu("ESC"," TO ABORT.");
}

void screen_select_file_new_name(void)
{
  // TODO: implement
}

void screen_hosts_and_devices_long_filename(char *f)
{
  // TODO: implement
}

void screen_hosts_and_devices_devices_clear_all(void)
{
  // TODO: implement
}

void screen_select_file_new_creating(void)
{
  // TODO: implement
}

void screen_select_slot_mode(void)
{
  screen_clear_status();
  screen_gotoxy(1,STATUS_BAR);
  screen_print_menu("R","EAD ONLY  ");
  screen_print_menu("W",": READ/WRITE");
}

void screen_select_slot_eject(unsigned char ds)
{
  screen_clearxy(3,1+ds,39);
  screen_gotoxy(4,1+ds); cprintf("EMPTY");
  bar_jump(bar_get());
}

void screen_hosts_and_devices_eject(unsigned char ds)
{
  screen_clearxy(3,13+ds,39);
  screen_gotoxy(4,13+ds); cprintf("EMPTY");
  bar_jump(bar_get());
}

void screen_hosts_and_devices_host_slot_empty(unsigned char hs)
{
  screen_gotoxy(4,2+hs); cprintf("EMPTY");
}

#endif /* BUILD_RC2014 */

