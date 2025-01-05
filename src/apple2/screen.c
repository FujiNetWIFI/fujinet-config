#ifdef BUILD_APPLE2
/**
 * FujiNet CONFIG FOR Apple2
 *
 * Screen Routines
 */

#include "screen.h"
#include "globals.h"
#include "bar.h"
#include "sp.h"
#include "diskii.h"
#include <string.h>
#ifdef __ORCAC__
#include <coniogs.h>
#include <apple2gs.h>
#include <texttool.h>
#else
#include <conio.h>
#include <apple2.h>
#include <peekpoke.h>
#include <6502.h>
#endif

#define MAX_SMARTPORT	4

// https://retrocomputing.stackexchange.com/questions/8652/why-did-the-original-apple-e-have-two-sets-of-inverse-video-characters:
// $00..$1F Inverse  Uppercase Letters
// $20..$3F Inverse  Symbols/Numbers
// $40..$5F Normal   MouseText
// $60..$7F Inverse  Lowercase Letters (the reason why flashing got dropped)
// $80..$9F Normal   Uppercase Letters
// $A0..$BF Normal   Symbols/Numbers   (like ASCII + $80)
// $C0..$DF Normal   Uppercase Letters (like ASCII + $80)
// $E0..$FF Normal   Lowercase Letters (like ASCII + $80)

// Because cputc() does an EOR #$80 internally:
// $00..$1F Normal   Uppercase Letters
// $20..$3F Normal   Symbols/Numbers   (like ASCII)
// $40..$5F Normal   Uppercase Letters (like ASCII)
// $50..$7F Normal   Lowercase Letters (like ASCII)
// $80..$9F Inverse  Uppercase Letters
// $A0..$BF Inverse  Symbols/Numbers
// $C0..$DF Normal   MouseText
// $E0..$FF Inverse  Lowercase Letters

#define STATUS_BAR 21

#define UNUSED(x) (void)(x);

static const char *empty="Empty";
static const char *off="Off";

static bool lowercase;
static bool mousetext;

extern bool copy_mode;
extern unsigned char copy_host_slot;
extern bool deviceEnabled[8];
extern char copySpec[256];

static void iputc(char c)
{
  if (c >= 0x40 && c <= 0x5F /* uppercase */)
  {
    c += 0x40;
  }
  else if (c >= 0x20 /* printable */)
  {
    c += 0x80;
  }
  cputc(c);
}

static void hline(unsigned char l)
{
  if (mousetext)
  {
  #ifdef __ORCAC__
    WriteChar(0x9b);  // Mousetext on
    WriteChar(0x8f);  // Inverse
  #endif
    while (l--)
    {
      cputc(0xD3); // ─
    }
  #ifdef __ORCAC__
    WriteChar(0x98);  // Mousetext off
    WriteChar(0x8e);  // Normal
  #endif
  }
  else
  {
    chline(l);
  }
}

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
    mousetext = true;
  #else
    // init canonical video mode like the ROM RESET code, which starts with
    //   JSR SETNORM
    //   JSR INIT
    struct regs r;
    r.pc = 0xFE84;	// SETNORM: set normal (not inverse) text mode
    _sys(&r);
    r.pc = 0xFB2F;	// INIT: set GR/HGR off, Text page 1
    _sys(&r);
    r.a = 0x91;     // Set 40 column mode, for IIgs startup in 80 col
    r.pc = 0xFDF0;  // COUT1
    _sys(&r);
    if (get_ostype() >= APPLE_IIIEM)
    {
      allow_lowercase(true);
      lowercase = true;
      if (get_ostype() >= APPLE_IIEENH)
      {
        POKE(0xC00F,0); // SETALTCHAR
        mousetext = true;
      }
    }
  #endif
  clrscr();
  #ifndef __ORCAC__
    POKE(0x2000,0x80); // \
    POKE(0x2001,0x80); //  > Overwrite JMP
    POKE(0x2002,0x80); // /
    if (get_ostype() == APPLE_IIIEM) // Satan Mode
    {
      POKE(0xC057,0); // GRAPH
    }
    else
    {
      POKE(0xC057,0); // HIRES
      POKE(0xC053,0); // MIXED
      POKE(0xC050,0); // GRAPH
    }
    cputsxy(13,23,"Initializing");

    {
      unsigned char dots;
      unsigned int delay;
      for (dots = 0; dots < 4; dots++)
      {
        for (delay = 0; delay < 6000; delay++);
        cputc('.');
      }
    }

    // Putting this here so it happens after the splash screen, even
    // though it's more of an io_init kind of thing.
    diskii_find();

    cclearxy(13,23,16);
    if (get_ostype() == APPLE_IIIEM) // Satan Mode
    {
      POKE(0xC056,0); // TEXT
    }
    else
    {
      POKE(0xC051,0); // TEXT
    }
  #endif
}

void screen_put_inverse(char c)
{
  if (lowercase)
  {
    iputc(c);
  }
  else
  {
    revers(true);
    cputc(c);
    revers(false);
  }
}

void screen_print_inverse(const char *s)
{
  if (lowercase)
  {
    char c;
    while (c = *s++)
    {
      iputc(c);
    }
  }
  else
  {
    revers(true);
    cputs(s);
    revers(false);
  }
}

void screen_print_menu(const char *si, const char *sc)
{
  screen_print_inverse(si);
  cputs(sc);
}

void screen_error(const char *s)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR + 1);
  screen_print_inverse(s);
  revers(true);
  cclear(40-wherex());
  revers(false);
}

void screen_putlcc(char c)
{
  if (c <= 0x5F /* uppercase */ || lowercase)
  {
    cputc(c);
  }
  else
  {
    screen_put_inverse(c);
  }
}

void screen_set_wifi(AdapterConfigExtended *acx)
{
  clrscr();
  cputsxy(9,0,"Welcome to FujiNet!");
  gotoxy(0,2); cprintf("MAC Address: %18s", acx->sMacAddress);
  cputsxy(7,STATUS_BAR,"Scanning for networks...");
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

  cputsxy(2,n+4,ds);
  cputsxy(36,n+4,meter);

}

void screen_set_wifi_select_network(unsigned char nn)
{
  cclearxy(0,STATUS_BAR,120);
  bar_set(4,1,nn,0);
  gotoxy(10,STATUS_BAR); cprintf("Found %d networks.\r\n", nn);
  gotoxy(4, STATUS_BAR + 1);
  screen_print_menu("H","idden SSID  ");
  screen_print_menu("R","escan  ");
  screen_print_menu("S","kip\r\n");
  gotoxy(10, STATUS_BAR + 2);
  screen_print_menu("RETURN"," to select");
}

void screen_set_wifi_custom(void)
{
  cclearxy(0,STATUS_BAR,120);
  cputsxy(0,STATUS_BAR,"Enter name of hidden network");
}

void screen_set_wifi_password(void)
{
  cclearxy(0, STATUS_BAR, 120);
  cputsxy(0,STATUS_BAR,"Enter net password and press [RETURN]");
  if (!lowercase)
  {
    cputsxy(0,STATUS_BAR - 1,"Use [ESC] to switch to upper/lower case");
  }
  cputcxy(0,STATUS_BAR + 1,']');
}

void screen_connect_wifi(NetConfig *nc)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR); cprintf("Connecting to network: %s",nc->ssid);
}

void screen_destination_host_slot(char *h, char *p)
{
  clrscr();
  cputsxy(0,12,"Copy from host slot");
  gotoxy(0,13);
  hline(40);
  gotoxy(0, 14); cprintf("%-32s", h);
  gotoxy(0, 15); cprintf("%-128s", p);
}

void screen_destination_host_slot_choose(void)
{
  static const char ct[] = " Copy to host slot";

  gotoxy(0, 0);
  hline(40 - (sizeof(ct)-1));
  cputs(ct);

  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR);
  screen_print_menu("1-8",":Choose slot\r\n");
  screen_print_menu("RETURN",":Select slot\r\n");
  screen_print_menu("ESC"," to abort");

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
  return c[0]==0x00 ? empty : c;
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
    if (i > MAX_SMARTPORT - 1) {
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
    if (i < MAX_SMARTPORT)
      cprintf("%d", i+1);
    else if (diskii_slotdrive[i-MAX_SMARTPORT].slot == 15)
      cprintf("NA");
    else if (diskii_slotdrive[i-MAX_SMARTPORT].slot == 0)
      cprintf("%d", i - MAX_SMARTPORT + 1);
    else
    {
      if (get_ostype() == APPLE_IIIEM) // Satan Mode
        cprintf("D%d", diskii_slotdrive[i - MAX_SMARTPORT].drive);
      else
        cprintf("S%dD%d", diskii_slotdrive[i - MAX_SMARTPORT].slot,
	        diskii_slotdrive[i - MAX_SMARTPORT].drive);
    }
    cprintf("%c %c%c%s", rw_mode, host_slot, separator, screen_hosts_and_devices_device_slot(d[i].hostSlot, e[i], (char *)d[i].file));
  }

}

void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d, bool *e)
{
  static const char hl[] = " Host List";
  static const char sd[] = " SmartPort Drives";
  static const char dd[] = " Disk II Drives";
  char i;

  clrscr();
  hline(40 - (sizeof(hl)-1));
  cputs(hl);

  cputsxy(0,10,"DR-H");
  hline(40 - wherex() - (sizeof(sd)-1));
  cputs(sd);

  gotoxy(0,15);
  hline(40 - (sizeof(dd)-1));
  cputs(dd);

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
  if (!lowercase && get_ostype() != APPLE_IIGS) // Show for II/II+ only
  {
    cputsxy(0,STATUS_BAR-1,"Use I J K M for navigation, T for TAB");
  }
  gotoxy(0,STATUS_BAR);
  screen_print_menu("1-8", ":Host  ");
  screen_print_menu("E","dit  ");
  screen_print_menu("RETURN",":Select files\r\n");
  screen_print_menu("C","onfig ");
  screen_print_menu("TAB",":Drives ");
  screen_print_menu("S","pDevs ");
  screen_print_menu("L","obby ");
  #ifdef __ORCAC__
    screen_print_menu("ESC",":Exit");
  #else
    screen_print_menu("ESC",":Boot");
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
  screen_print_menu("E","ject  ");
  screen_print_menu("R","ead only  ");
  screen_print_menu("W","rite\r\n");
  screen_print_menu("TAB",":Host slots  ");
  screen_print_menu("L","obby ");
  screen_print_menu("ESC", ":Boot");
}

void screen_hosts_and_devices_devices_selected(char selected_slot)
{
  bar_set_split(11,1,6,selected_slot,1);
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR);
  screen_print_menu("E","ject  ");
  screen_print_menu("R","ead only  ");
  screen_print_menu("W","rite\r\n");
  screen_print_menu("TAB",":Host slots  ");
  screen_print_menu("ESC", ":Boot");
}

void screen_hosts_and_devices_clear_host_slot(unsigned char i)
{
  cclearxy(1,i+1,39);
}

void screen_hosts_and_devices_edit_host_slot(unsigned char i)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR); cprintf("Edit the host name for slot %d\r\nPress [RETURN] when done.", i + 1);
}

void screen_perform_copy(char *sh, char *p, char *dh, char *dp)
{
  clrscr();    cprintf("%32s","Copying file from:");
  gotoxy(0,2); cprintf("%32s",sh);
  gotoxy(0,3); cprintf("%-128s",p);
  gotoxy(0,7); cprintf("%32s","Copying file to:");
  gotoxy(0,9); cprintf("%32s",dh);
  gotoxy(0,10); cprintf("%-128s",dp);
}

void screen_show_info(bool printerEnabled, AdapterConfigExtended* acx)
{
  clrscr();

  revers(true);
  cputsxy(3,5," F U J I N E T      C O N F I G ");
  revers(false);
  gotoxy(0,8);
  cprintf("%10s%s\r\n","SSID: ",acx->ssid);
  cprintf("%10s%s\r\n","Hostname: ",acx->hostname);
  cprintf("%10s%s\r\n","IP: ",acx->sLocalIP);
  cprintf("%10s%s\r\n","Netmask: ",acx->sNetmask);
  cprintf("%10s%s\r\n","DNS: ",acx->sDnsIP);
  cprintf("%10s%s\r\n","MAC: ",acx->sMacAddress);
  cprintf("%10s%s\r\n","BSSID: ",acx->sBssid);
  cprintf("%10s%s\r\n","FNVer: ",acx->fn_version);

  gotoxy(6,STATUS_BAR);
  screen_print_menu("C","hange SSID  ");
  screen_print_menu("R","econnect\r\n");
  cputs("   Press any key to return to hosts\r\n");
  cprintf("      FujiNet printer %s",(printerEnabled) ? "enabled" : "disabled");
  }

void screen_select_file(void)
{
  cclearxy(0,STATUS_BAR,120);
  cputsxy(0,STATUS_BAR,"Opening...");
}

void screen_select_file_display(char *p, char *f)
{
  clrscr();

  // Update content area
  cprintf("%-40s",selected_host_name);
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

#pragma warn (unused-param, push, off)
void screen_select_file_display_long_filename(char *e)
{
  // it wasn't this.
  /* gotoxy(0,19); */
  /* cprintf("%-40s",e); */
}
#pragma warn (unused-param, pop)

void screen_select_file_next(void)
{
  gotoxy(0,18); cprintf("%-40s","[...]");
}

#pragma warn (unused-param, push, off)
void screen_select_file_display_entry(unsigned char y, char* e, unsigned entryType)
{
  gotoxy(0,y+3);
  cprintf("%-40s",&e[2]); // skip the first two chars from FN (hold over from Adam)
}
#pragma warn (unused-param, pop)

void screen_select_file_clear_long_filename(void)
{
  // Is it this?
  // cclearxy(0,13,80);
}

void screen_select_file_new_type(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR);
  screen_print_menu(" New media: Select type \r\n","");
  screen_print_menu("P","O  ");
  screen_print_menu("2","MG  ");
}

void screen_select_file_new_size(unsigned char k)
{
  UNUSED(k); // Image type is not used.
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR);
  screen_print_menu(" New media: Select size \r\n","");
  screen_print_menu("1","40K  ");
  screen_print_menu("8","00K  ");
  screen_print_menu("3","2MB  ");
}

void screen_select_file_new_custom(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR);
  screen_print_menu(" New media: Custom size \r\n","");
  screen_print_menu("Enter size in # of 512-byte blocks\r\n","");
}

void screen_select_file_choose(char visibleEntries)
{
  bar_set(3,2,visibleEntries,0); // TODO: Handle previous
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR);
  if (copy_mode == true)
  {
    screen_print_menu("RETURN",":Select directory\r\n");
    screen_print_menu("ESC",":Abort  ");
    screen_print_menu("C","opy start   ");
  }
  else
  {
    screen_print_menu("RETURN",":Selsect file to mount\r\n");
    screen_print_menu("<-","Updir  ");
    screen_print_menu("ESC",":Abort  ");
    screen_print_menu("F","ilter  ");
    screen_print_menu("N","ew  ");
    screen_print_menu("C","opy  ");
  }
}

void screen_select_file_filter(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(0,STATUS_BAR); cprintf("Enter a wildcard filter.\r\n E.G. *Apple*");
}

void screen_select_slot(char *e)
{
  unsigned long *s;
  static const char ss[] = " SmartPort Drives";
  static const char ds[] = " Disk II Drives";

  clrscr();

  gotoxy(0,1);
  hline(40 - (sizeof(ss)-1));
  cputs(ss);

  gotoxy(0,6);
  hline(40 - (sizeof(ds)-1));
  cputs(ds);

  gotoxy(0,12);
  cprintf("%-40s","File details");
  cprintf("%8s 20%02u-%02u-%02u %02u:%02u:%02u\r\n","MTime:",*e++,*e++,*e++,*e++,*e++,*e++);

  s=(unsigned long *)e; // Cast the next four bytes as a long integer.
  cprintf("%8s %lu K\r\n\r\n","Size:",*s >> 10); // Quickly divide by 1024

  e += sizeof(unsigned long) + 2; // I do not need the next two bytes.
  cprintf("%-40s",e);

  screen_hosts_and_devices_device_slots(2,&deviceSlots[0],&deviceEnabled[0]);

  bar_set_split(2,1,6,0,1);
}

void screen_select_slot_choose(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(1,STATUS_BAR);
  screen_print_menu("1-6"," Select drive or use arrow keys\r\n ");
  screen_print_menu("RETURN/R",":Insert read only\r\n ");
  screen_print_menu("W",":Insert read/write  ");
  screen_print_menu("ESC",":Abort");
}

void screen_select_file_new_name(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(3,STATUS_BAR);
  screen_print_menu(" New media: Enter filename \r\n","");
}

void screen_hosts_and_devices_long_filename(char *f)
{
  // TODO: implement
  if (strlen(f)>31)
  {
    cputsxy(0,STATUS_BAR-3,f);
  }
  else
    cclearxy(0,STATUS_BAR-3,120);
}

void screen_hosts_and_devices_devices_clear_all(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(3,STATUS_BAR);
  screen_print_menu(" Clearing all device slots ","");
}

void screen_select_file_new_creating(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(3,STATUS_BAR);
  screen_print_menu(" Creating disk image \r\n","Please wait...");
}

void screen_select_slot_mode(void)
{
  cclearxy(0,STATUS_BAR,120);
  gotoxy(1,STATUS_BAR);
  screen_print_menu("R","ead only  ");
  screen_print_menu("W",": Read/write");
}

void screen_select_slot_eject(unsigned char ds)
{
  cclearxy(1,1+ds,39);
  cputsxy(2,1+ds,empty);
  bar_jump(bar_get());
}

void screen_hosts_and_devices_eject(unsigned char ds)
{
  if (ds > 3) // diskII split
  {
	  cclearxy(1,12+ds,39);
    cputsxy(5,12+ds,empty);
  }
  else
  {
	  cclearxy(1,11+ds,39);
    cputsxy(5,11+ds,empty);
  }
  bar_jump(bar_get());
}

void screen_hosts_and_devices_host_slot_empty(unsigned char hs)
{
  cputsxy(2,1+hs,empty);
}

bool screen_mount_and_boot_lobby(void)
{
  unsigned char k;

  // Confirm we want to go to there
  cclearxy(0,STATUS_BAR,120);
  gotoxy(3,STATUS_BAR);
  screen_print_menu(" Boot to Lobby?", " Y/N");

  k=cgetc();

  switch (k)
  {
  case 'Y':
  case 'y':
    return true;
  default:
    return false;
  }
}
#endif /* BUILD_APPLE2 */
