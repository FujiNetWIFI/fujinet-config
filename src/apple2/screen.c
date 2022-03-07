#ifdef BUILD_APPLE2
/**
 * #FujiNet CONFIG FOR Apple2
 *
 * Screen Routines
 */

#include "screen.h"
#include "globals.h"
#include "bar.h"
#include <conio.h>
#include <string.h>
#include <apple2.h>

#define STATUS_BAR 21

unsigned char *mousetext = (unsigned char *)0xC00E;

void screen_init(void)
{
  clrscr();
}

void screen_error(const char *c)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR + 1); cprintf("%s",c);
}

void screen_putlcc(const char *c)
{
  char modifier = 128;
  char ostype;
  ostype = get_ostype() & 0xF0;

  switch (ostype)
  {
  case APPLE_IIE: /* Apple //e                   */
    modifier = 0;
    break;
  case APPLE_IIC:  /* Apple //c                   */
    if ((c > 63) && (c < 96))
      // upper case
      modifier = 64;
    break;
  case APPLE_IIGS: /* Apple IIgs                  */
    break;
  case APPLE_II:
  default:
    if ((c > 63) && (c < 96))
      // upper case
      modifier = 64;
    else if (c > 95)
      // lower case
      modifier = -96;
    break;
  }

  if (ostype == APPLE_IIC)
    mousetext[1] = 1; // turn on mouse text
  
  ram[bar_coord(wherex(), wherey())] = c + modifier;
  
  if (ostype == APPLE_IIC)
    mousetext[0] = 1; // turn off mouse text
}

void screen_set_wifi(AdapterConfig *ac)
{
  clrscr();
  gotoxy(9,0); cprintf("WELCOME TO #FUJINET!");
  gotoxy(0,2); cprintf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X",ac->macAddress[0],ac->macAddress[1],ac->macAddress[2],ac->macAddress[3],ac->macAddress[4],ac->macAddress[5]);
  gotoxy(7,STATUS_BAR); cprintf("SCANNING FOR NETWORKS...");
}

void screen_set_wifi_display_ssid(char n, SSIDInfo *s)
{
  char meter[4]={0x20,0x20,0x20,0x00};
  char ds[32];

  memset(ds,0x20,32);
  strncpy(ds,s->ssid,32);
  
  if (s->rssi > -40)
    {
      meter[0] = '*';
      meter[1] = '*';
      meter[2] = '*';
    }
  else if (s->rssi > -60)
    {
      meter[0] = '*';
      meter[1] = '*';
    }
  else
    {
      meter[0] = '*';
    }
  
  gotoxy(2,n+4); cprintf("%s",ds);
  gotoxy(36,n+4); cprintf("%s",meter);
  
}

//void screen_set_wifi_select_network(unsigned char nn)
void screen_set_wifi_select_network()
{
  cclearxy(0,STATUS_BAR,120);
  //gotoxy(0,STATUS_BAR); cprintf("FOUND %d NETWORKS\r\n[H]IDDEN SSID  [R]ESCAN  [S]KIP\r\nOR PRESS NUMBER TO SELECT NETWORK.", nn);
  //gotoxy(0,STATUS_BAR); cprintf("[H]IDDEN SSID  [R]ESCAN  [S]KIP\r\nOR PRESS NUMBER TO SELECT NETWORK.");
  gotoxy(4, STATUS_BAR + 1);
  cprintf("[H]IDDEN SSID  [R]ESCAN  [S]KIP\r\n");
  gotoxy(10, STATUS_BAR + 2);
  cprintf("[RETURN] TO SELECT");
}

void screen_set_wifi_custom(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR); cprintf("ENTER NAME OF HIDDDEN NETWORK");
}

void screen_set_wifi_password(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR); cprintf("ENTER NETWORK PASSWORD AND PRESS [RETURN]");
  gotoxy(0, STATUS_BAR + 1); cputc(']');
}

void screen_connect_wifi(NetConfig *nc)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR); cprintf("CONNECTING TO NETWORK: %s",nc->ssid);
}

void screen_destination_host_slot(char *h, char *p) {
  // TODO: implement
}

void screen_destination_host_slot_choose(void) {
  // TODO: implement
}

char* screen_hosts_and_devices_slot(char *c)
{
  return c[0]==0x00 ? "Empty" : c;
}

void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *d)
{
  char i;
  
  for (i=0;i<4;i++)
    {
      gotoxy(0,i+y); cprintf("%d %s",i+1,screen_hosts_and_devices_slot(d[i].file));
    } 
}

void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d)
{
  char i;
  gotoxy(0,0);  cprintf("%40s","HOST SLOTS");
  gotoxy(0,11); cprintf("%40s","DISK SLOTS");

  chlinexy(0,1,40);
  chlinexy(0,12,40);
  chlinexy(0,20,40);

  for (i=0;i<8;i++)
    {
      gotoxy(0,i+2); cprintf("%d %s",i+1,screen_hosts_and_devices_slot(h[i])); 
    }

  screen_hosts_and_devices_device_slots(13,d);  
}

void screen_hosts_and_devices_hosts(void)
{
  bar_set(2,1,8,0);
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR); cprintf("[C]ONFIG  [E]DIT SLOT  [ESC]BOOT\r\n[1-8]HOST SLOT  [RETURN]SELECT SLOT\r\n[TAB] DEVICE SLOTS");
}

void screen_hosts_and_devices_host_slots(HostSlot *h) {
  // TODO: Implement
}

void screen_hosts_and_devices_devices(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR); cprintf("[E]JECT  [R]EAD ONLY  [W]RITE  [TAB] HOST SLOTS");
}

void screen_hosts_and_devices_clear_host_slot(unsigned char i)
{
  cclearxy(1,i+1,39);
}

void screen_hosts_and_devices_edit_host_slot(unsigned char i)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR); cprintf("EDIT THE HOST NAME FOR SLOT %d\r\nPRESS [RETURN] WHEN DONE.");
}

void screen_perform_copy(char *sh, char *p, char *dh, char *dp) {
  // TODO: Implement
}

void screen_show_info(AdapterConfig* ac)
{
  clrscr();
  
  gotoxy(4,5);
  cprintf("F U J I N E T      C O N F I G");
  gotoxy(0,8);
  cprintf("%10s%s\r\n","SSID: ",ac->ssid);
  cprintf("%10s%s\r\n","HOSTNAME: ",ac->hostname);
  cprintf("%10s%u.%u.%u.%u\r\n","IP: ",ac->localIP[0],ac->localIP[1],ac->localIP[2],ac->localIP[3]);
  cprintf("%10s%u.%u.%u.%u\r\n","NETMASK: ",ac->netmask[0],ac->netmask[1],ac->netmask[2],ac->netmask[3]);
  cprintf("%10s%u.%u.%u.%u\r\n","DNS: ",ac->dnsIP[0],ac->dnsIP[1],ac->dnsIP[2],ac->dnsIP[3]);
  cprintf("%10s%02x:%02x:%02x:%02x:%02x:%02x\r\n","MAC: ",ac->macAddress[0],ac->macAddress[1],ac->macAddress[2],ac->macAddress[3],ac->macAddress[4],ac->macAddress[5]);
  cprintf("%10s%02x:%02x:%02x:%02x:%02x:%02x\r\n","BSSID: ",ac->bssid[0],ac->bssid[1],ac->bssid[2],ac->bssid[3],ac->bssid[4],ac->bssid[5]);
  cprintf("%10s%s\r\n","FNVER: ",ac->fn_version);

  gotoxy(6,STATUS_BAR); 
  cprintf("[C]HANGE SSID  [R]ECONNECT\r\n");
  cprintf("   PRESS ANY KEY TO RETURN TO HOSTS");
}

void screen_select_file(void)
{
  gotoxy(0,STATUS_BAR); cprintf("OPENING...");
}

void screen_select_file_display(char *p, char *f)
{
  // Clear content area
  cclearxy(0,0,560);

  // Update content area
  gotoxy(0,0); cprintf("%40s",selected_host_name);

  if (f[0]==0x00)
    cprintf("%40s",p);
  else
    cprintf("%STATUS_BARs%8s",p,f);
}

void screen_select_file_prev(void)
{
  gotoxy(0,2); cprintf("%40s","[...]");
}

void screen_select_file_display_long_filename(char *e) 
{
  // TODO: Implement
}

void screen_select_file_next(void)
{
  gotoxy(0,18); cprintf("%40s","[...]");
}

void screen_select_file_display_entry(unsigned char y, char* e)
{
  gotoxy(0,y+3);
  cprintf("%c%c",*e++,*e++);
  cprintf("%-30s",e);
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
  bar_set(2,2,visibleEntries,0); // TODO: Handle previous
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR); cprintf("SELECT FILE TO MOUNT  [A] UP  [Z] DOWN\r\n[ESC] PARENT  [F]ILTER  [ESC]BOOT");  
}

void screen_select_file_filter(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR); cprintf("ENTER A WILDCRD FILTER.\r\n E.G. *Apple*");
}

void screen_select_slot(char *e)
{
  unsigned long *s;
  
  gotoxy(0,7);
  cprintf("%40s","FILE DETAILS");
  cprintf("%8s 20%02u-%02u-%02u %02u:%02u:%02u\r\n","MTIME:",*e++,*e++,*e++,*e++,*e++,*e++);
  
  s=(unsigned long *)e; // Cast the next four bytes as a long integer.
  
  cprintf("%8s %lu K\r\n","SIZE:",*s >> 10); // Quickly divide by 1024

  e += sizeof(unsigned long) + 2; // I do not need the last two bytes.
  
  gotoxy(0,0);
  cprintf("%40s",e);

  screen_hosts_and_devices_device_slots(1,&deviceSlots[0]);
    
  bar_set(0,1,4,0);
}

void screen_select_slot_choose(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR); cprintf(" [1-4] SELECT SLOT\r\n [RETURN] INSERT INTO SLOT\r\n [ESC] TO ABORT.");
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
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR); cprintf(" [R]EAD ONLY  [W] READ/WRITE");
}

void screen_select_slot_eject(unsigned char ds)
{
  cclearxy(1,1+ds,39);
  gotoxy(1,1+ds); cprintf("Empty");
  bar_jump(bar_get());
}

void screen_hosts_and_devices_eject(unsigned char ds)
{
  cclearxy(1,12,39);
  gotoxy(1,12+ds); cprintf("Empty");
  bar_jump(bar_get());
}

void screen_hosts_and_devices_host_slot_empty(unsigned char hs)
{
  gotoxy(1,1+hs); cprintf("Empty");
}  
#endif /* BUILD_APPLE2 */
