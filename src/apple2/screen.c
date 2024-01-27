#ifdef BUILD_APPLE2
/**
 * #FujiNet CONFIG FOR Apple2
 *
 * Screen Routines
 */

#include "screen.h"
#include "globals.h"
#include "bar.h"
#include "sp.h"
#include <conio.h>
#include <string.h>
#include <apple2.h>
#ifdef __ORCAC__
#include <texttool.h>
#else
#include <6502.h>
#endif


#define STATUS_BAR 21

#define UNUSED(x) (void)(x);

static const char *empty="EMPTY";
static const char *off="OFF";

extern bool copy_mode;
extern unsigned char copy_host_slot;
extern bool deviceEnabled[8];
extern char copySpec[256];

void screen_init(void)
{
  #ifdef __ORCAC__
    TextStartUp();
    SetInGlobals(0x7f, 0x00);
    SetOutGlobals(0xff, 0x80);
    SetInputDevice(basicType, 3);
    SetOutputDevice(basicType, 3);
    InitTextDev(input);
    InitTextDev(output);
    WriteChar(0x91);  // Set 40 col
  #else
    // init canonical video mode like the ROM RESET code, which starts with
    //   JSR SETNORM
    //   JSR INIT
    struct regs r;
    r.pc = 0xFE84;	// SETNORM: set normal (not inverse) text mode
    _sys(&r);
    r.pc = 0xFB2f;	// INIT: set GR/HGR off, Text page 1
    _sys(&r);
    r.a = 0x91;     // Set 40 column mode, for IIgs startup in 80 col
    r.pc = 0xFDF0;  // COUT1
    _sys(&r);
  #endif
  clrscr();
  sp_init(); // moved here so we do after screen is setup, and before logo
 #ifndef BUILD_A2CDA
  screen_fujinetlogo();
 #endif
}

void screen_put_inverse(const char c)
{
  revers(1);
  cputc(c);
  revers(0);
}

void screen_print_inverse(const char *s)
{
  revers(1);
  cprintf(s);
  revers(0);
}

void screen_print_menu(const char *si, const char *sc)
{
  screen_print_inverse(si);
  cprintf(sc);
}

void screen_fujinetlogo(void)
{
  unsigned char i, j;

  gotoxy(20,12);
  cprintf("O");

  for (i = 0; i < 11; i++)
  {
      gotoxy(i+4,12);    // fuji scrolling across left to centre
      cprintf(" FUJI*");
      gotoxy(31-i,12);   // net scrolling back right to centre
      cprintf("*NET ");
      gotoxy(20,i);      // * coming down from the top
      cprintf(" ");
      gotoxy(20,i+1);
      cprintf("*");
      gotoxy(21,23-i);   // * coming up from bottom
      cprintf("*");
      gotoxy(21,23-i+1);
      cprintf(" ");
      for(j = 0; j < 255; j++);
   }
   for(i = 0; i < 127; i++) // delay a bit
   {
      for(j = 0; j < 255; j++);
   }
}

void screen_error(const char *c)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR + 1);
  revers(1);
  cprintf("%-40s",c);
  revers(0);
}

void screen_putlcc(char c)
{
  char modifier = 128;
  char ostype;
  ostype = get_ostype() & 0xF0;

  switch (ostype)
  {
  case APPLE_IIE: /* Apple //e                   */
    if ((c > 63) && (c < 96))
      // upper case
      modifier = 64;
    else if (c > 95)
      // lower case
      modifier = 128;
    break;
  case APPLE_IIC:  /* Apple //c                   */
    if ((c > 63) && (c < 96))
      // upper case
      modifier = 64;
    break;
  case APPLE_IIGS: /* Apple IIgs                  */
  case APPLE_IIIEM: /* Apple /// (emulation with lowercase) */
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

  CURRENT_LINE[wherex()] = c + modifier;
}

void screen_set_wifi(AdapterConfig *ac)
{
  clrscr();
  gotoxy(9,0); cprintf("WELCOME TO #FUJINET!");
  gotoxy(0,2); cprintf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X",
    ac->macAddress[0],
    ac->macAddress[1],
    ac->macAddress[2],
    ac->macAddress[3],
    ac->macAddress[4],
    ac->macAddress[5]);
  gotoxy(7,STATUS_BAR); cprintf("SCANNING FOR NETWORKS...");
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

  gotoxy(2,n+4); cprintf("%s",ds);
  gotoxy(36,n+4); cprintf("%s",meter);

}

void screen_set_wifi_select_network(unsigned char nn)
{
  cclearxy(0,STATUS_BAR,120);
  bar_set(4,1,nn,0);
  gotoxy(10,STATUS_BAR); cprintf("FOUND %d NETWORKS.\r\n", nn);
  gotoxy(4, STATUS_BAR + 1);
  screen_print_menu("H","IDDEN SSID  ");
  screen_print_menu("R","ESCAN  ");
  screen_print_menu("S","KIP\r\n");
  gotoxy(10, STATUS_BAR + 2);
  screen_print_menu("RETURN"," TO SELECT");
}

void screen_set_wifi_custom(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR); cprintf("ENTER NAME OF HIDDEN NETWORK");
}

void screen_set_wifi_password(void)
{
  char ostype;
  ostype = get_ostype() & 0xF0;

  cclearxy(0, STATUS_BAR, 120);
  gotoxy(0,STATUS_BAR); cprintf("ENTER NET PASSWORD AND PRESS [RETURN]");
  if (ostype == APPLE_II)
  {
    gotoxy(0, STATUS_BAR - 1); cprintf("USE [ESC] TO SWITCH TO UPPER/LOWER CASE");
  }
  gotoxy(0, STATUS_BAR + 1); cputc(']');
}

void screen_connect_wifi(NetConfig *nc)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR); cprintf("CONNECTING TO NETWORK: %s",nc->ssid);
}

void screen_destination_host_slot(char *h, char *p)
{
  clrscr();
  gotoxy(0, 12); cprintf("COPY FROM HOST SLOT");
  chlinexy(0,13,40);
  gotoxy(0, 14); cprintf("%-32s", h);
  gotoxy(0, 15); cprintf("%-128s", p);
}

void screen_destination_host_slot_choose(void)
{
  static const char ct[] = "COPY TO HOST SLOT";

  gotoxy(0, 0);
  cprintf("%40s", ct);
  chlinexy(0,0,40 - sizeof(ct));

  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR);
  screen_print_menu("1-8",":CHOOSE SLOT\r\n");
  screen_print_menu("RETURN",":SELECT SLOT\r\n");
  screen_print_menu("ESC"," TO ABORT");

  bar_set(1, 1, 8, selected_host_slot);
}

const char* screen_hosts_and_devices_device_slot(unsigned char hs, bool e, char *fn)
{
  UNUSED(hs);
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
  unsigned char line;
  char rw_mode;
  char host_slot;
  char separator;

  for (i = 0; i < NUM_DEVICE_SLOTS; i++)
  {
    line = y + i;
    if (i > 3) {
      // skip over diskII heading
      line++; 
    }

    if (d[i].file[0]) {
        switch (d[i].mode) {
          case MODE_READ:
            rw_mode = 'R';
            break;
          case MODE_WRITE:
            rw_mode = 'W';
            break;
          default:
            // should not happen ... but we've got bugs
            rw_mode = '?';
            break;
        }
        host_slot = '1' + d[i].hostSlot;
        separator = ':';
    } else {
        rw_mode = ' ';
        host_slot = ' ';
        separator = ' ';
    }

    gotoxy(0, line);
    cprintf("%d%c %c%c%s", i+1, rw_mode, host_slot, separator, screen_hosts_and_devices_device_slot(d[i].hostSlot, e[i], (char *)d[i].file));
  }

}

void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d, bool *e)
{
  static const char hl[] = "HOST LIST";
  static const char ss[] = "SMARTPORT DRIVES";
  static const char ds[] = "DISKII DRIVES";
  char i;

  clrscr();
  gotoxy(0,0);  cprintf("%40s",hl);
  chlinexy(0,0,40 - sizeof(hl));

  gotoxy(0,10); cprintf("%40s",ss);
  chlinexy(0,10,40 - sizeof(ss));

  gotoxy(0,15); cprintf("%40s",ds);
  chlinexy(0,15,40 - sizeof(ds));

  for (i=0;i<8;i++)
    {
      gotoxy(0,i+1); cprintf("%d %s",i+1,screen_hosts_and_devices_slot((char *)h[i]));
    }

  screen_hosts_and_devices_device_slots(11,d,e);
}

void screen_hosts_and_devices_hosts(void)
{
  bar_set(1, 1, 8, selected_host_slot);
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR);
  screen_print_menu("1-8", ":HOST  ");
  screen_print_menu("E","DIT  ");
  screen_print_menu("RETURN",":SELECT FILES\r\n");
  screen_print_menu("C","ONFIG ");
  screen_print_menu("TAB",":DRIVES ");
  screen_print_menu("D","EVICES ");
  #ifdef __ORCAC__
    screen_print_menu("ESC",":EXIT");
  #else
    screen_print_menu("ESC",":BOOT");
  #endif
}

void screen_hosts_and_devices_host_slots(HostSlot *h)
{
  char i;

  for (i=0;i<8;i++)
    {
      gotoxy(0,i+1); cprintf("%d %-32s",i+1,screen_hosts_and_devices_slot((char *)h[i]));
    }
}

void screen_hosts_and_devices_devices(void)
{
  bar_set_split(11,1,6,0,1);
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR);
  screen_print_menu("E","JECT  ");
  screen_print_menu("R","EAD ONLY  ");
  screen_print_menu("W","RITE\r\n");
  screen_print_menu("TAB",":HOST SLOTS  ");
  screen_print_menu("ESC", ":BOOT");
}

void screen_hosts_and_devices_clear_host_slot(unsigned char i)
{
  cclearxy(1,i+1,39);
}

void screen_hosts_and_devices_edit_host_slot(unsigned char i)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR); cprintf("EDIT THE HOST NAME FOR SLOT %d\r\nPRESS [RETURN] WHEN DONE.", i + 1);
}

void screen_perform_copy(char *sh, char *p, char *dh, char *dp)
{
  clrscr();
  gotoxy(0,0); cprintf("%32s","COPYING FILE FROM:");
  gotoxy(0,2); cprintf("%32s",sh);
  gotoxy(0,3); cprintf("%-128s",p);
  gotoxy(0,7); cprintf("%32s","COPYING FILE TO:");
  gotoxy(0,9); cprintf("%32s",dh);
  gotoxy(0,10); cprintf("%-128s",dp);
}

void screen_show_info(bool printerEnabled, AdapterConfig* ac)
{
  clrscr();

  gotoxy(4,5);
  revers(1);
  cprintf("F U J I N E T      C O N F I G");
  revers(0);
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
  screen_print_menu("C","HANGE SSID  ");
  screen_print_menu("R","ECONNECT\r\n");
  cprintf("   PRESS ANY KEY TO RETURN TO HOSTS\r\n");
  cprintf("      FUJINET PRINTER %s",(printerEnabled) ? "ENABLED" : "DISABLED");
  }

void screen_select_file(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR); cprintf("OPENING...");
}

void screen_select_file_display(char *p, char *f)
{
  clrscr();

  // Update content area
  gotoxy(0,0); cprintf("%-40s",selected_host_name);
  gotoxy(0,1);
  if (f[0]==0x00)
    cprintf("%-40s",p);
  else
    cprintf("%-32s%8s",p,f);
}

void screen_select_file_prev(void)
{
  gotoxy(0,2); cprintf("%-40s","[...]");
}

void screen_select_file_display_long_filename(char *e)
{
  // it wasn't this.
  /* gotoxy(0,19); */
  /* cprintf("%-40s",e); */
}

void screen_select_file_next(void)
{
  gotoxy(0,18); cprintf("%-40s","[...]");
}

void screen_select_file_display_entry(unsigned char y, char* e, unsigned entryType)
{
  gotoxy(0,y+3);
  cprintf("%-40s",&e[2]); // skip the first two chars from FN (hold over from Adam)
}

void screen_select_file_clear_long_filename(void)
{
  // Is it this?
  // cclearxy(0,13,80);
}

void screen_select_file_new_type(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR);
  screen_print_menu(" NEW MEDIA: SELECT TYPE \r\n","");
  screen_print_menu("P","O  ");
  screen_print_menu("2","MG  ");
}

void screen_select_file_new_size(unsigned char k)
{
  UNUSED(k); // Image type is not used.
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR);
  screen_print_menu(" NEW MEDIA: SELECT SIZE \r\n","");
  screen_print_menu("1","40K  ");
  screen_print_menu("8","00K  ");
  screen_print_menu("3","2MB  ");
}

void screen_select_file_new_custom(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR);
  screen_print_menu(" NEW MEDIA: CUSTOM SIZE \r\n","");
  screen_print_menu("ENTER SIZE IN # OF 512-BYTE BLOCKS\r\n","");
}

void screen_select_file_choose(char visibleEntries)
{
  bar_set(3,2,visibleEntries,0); // TODO: Handle previous
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR);
  if (copy_mode == true)
  {
    screen_print_menu("RETURN",":SELECT DIRECTORY\r\n");
    screen_print_menu("ESC",":ABORT  ");
    screen_print_menu("C",":OPY START  ");
  }
  else
  {
    screen_print_menu("RETURN",":SELECT FILE TO MOUNT\r\n");
    screen_print_menu("ESC",":ABORT  ");
    screen_print_menu("F","ILTER  ");
    screen_print_menu("N","EW  ");
    screen_print_menu("C","OPY  ");
  }
}

void screen_select_file_filter(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR); cprintf("ENTER A WILDCARD FILTER.\r\n E.G. *Apple*");
}

void screen_select_slot(char *e)
{
  unsigned long *s;
  static const char ss[] = "SMARTPORT DRIVES";
  static const char ds[] = "DISKII DRIVES";

  clrscr();

  gotoxy(0,1); cprintf("%40s",ss);
  chlinexy(0,1,40 - sizeof(ss));

  gotoxy(0,6); cprintf("%40s",ds);
  chlinexy(0,6,40 - sizeof(ds));

  gotoxy(0,12);
  cprintf("%-40s","FILE DETAILS");
  cprintf("%8s 20%02u-%02u-%02u %02u:%02u:%02u\r\n","MTIME:",*e++,*e++,*e++,*e++,*e++,*e++);

  s=(unsigned long *)e; // Cast the next four bytes as a long integer.
  cprintf("%8s %lu K\r\n\r\n","SIZE:",*s >> 10); // Quickly divide by 1024

  e += sizeof(unsigned long) + 2; // I do not need the next two bytes.
  cprintf("%-40s",e);

  screen_hosts_and_devices_device_slots(2,&deviceSlots[0],&deviceEnabled[0]);

  bar_set_split(2,1,6,0,1);
}

void screen_select_slot_choose(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(1,STATUS_BAR);
  screen_print_menu("1-6"," SELECT DRIVE OR USE ARROW KEYS\r\n ");
  screen_print_menu("RETURN/R",":INSERT READ ONLY\r\n ");
  screen_print_menu("W",":INSERT READ/WRITE  ");
  screen_print_menu("ESC",":ABORT");
}

void screen_select_file_new_name(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(3,STATUS_BAR);
  screen_print_menu(" NEW MEDIA: ENTER FILENAME \r\n","");
}

void screen_hosts_and_devices_long_filename(char *f)
{
  // TODO: implement
  if (strlen(f)>31)
    {
      gotoxy(0,STATUS_BAR-3);
      cprintf("%s",f);
    }
  //else
  //  cclearxy(0,STATUS_BAR-3,120); // this was wiping the diskII, take out for now
}

void screen_hosts_and_devices_devices_clear_all(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(3,STATUS_BAR);
  screen_print_menu(" CLEARING ALL DEVICE SLOTS ","");
}

void screen_select_file_new_creating(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(3,STATUS_BAR);
  screen_print_menu(" CREATING DISK IMAGE \r\n","PLEASE WAIT...");
}

void screen_select_slot_mode(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(1,STATUS_BAR);
  screen_print_menu("R","EAD ONLY  ");
  screen_print_menu("W",": READ/WRITE");
}

void screen_select_slot_eject(unsigned char ds)
{
  cclearxy(1,1+ds,39);
  gotoxy(2,1+ds); cprintf("Empty");
  bar_jump(bar_get());
}

void screen_hosts_and_devices_eject(unsigned char ds)
{
  if (ds > 3) // diskII split
  {
	  cclearxy(1,12+ds,39);
      gotoxy(5,12+ds); cprintf("Empty");
  }
  else
  {
	  cclearxy(1,11+ds,39);
      gotoxy(5,11+ds); cprintf("Empty");
  }
  bar_jump(bar_get());
}

void screen_hosts_and_devices_host_slot_empty(unsigned char hs)
{
  gotoxy(2,1+hs); cprintf("Empty");
}
#endif /* BUILD_APPLE2 */
