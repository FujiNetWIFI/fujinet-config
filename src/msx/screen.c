/**
 * FujiNet CONFIG for MSX
 *
 * Screen Routines
 */

#include <stdbool.h>
#include <string.h>
#include <video/tms99x8.h>
#include <graphics.h>
#include <conio.h>
#include <sys/ioctl.h>
#include "../screen.h"
#include "../globals.h"

#define F1_ADDR          0xF87F
#define F2_ADDR          0xF88F
#define F3_ADDR          0xF89F
#define F4_ADDR          0xF8AF
#define F5_ADDR          0xF8BF
#define F6_ADDR          0xF8CF
#define F7_ADDR          0xF8DF
#define F8_ADDR          0xF8EF
#define F9_ADDR          0xF8FF
#define F10_ADDR         0xF90F

#define MAX_DISK_SLOTS (8)

#define style_black_on_white() vdp_color(VDP_INK_BLACK, VDP_INK_WHITE, VDP_INK_DARK_BLUE)
#define style_gray_on_white() vdp_color(VDP_INK_GRAY, VDP_INK_WHITE, VDP_INK_DARK_BLUE)
#define style_white_on_black() vdp_color(VDP_INK_WHITE, VDP_INK_BLACK, VDP_INK_DARK_BLUE)
#define style_white_on_blue() vdp_color(VDP_INK_WHITE, VDP_INK_DARK_BLUE, VDP_INK_DARK_BLUE)
#define style_highlighted() vdp_color(VDP_INK_WHITE, VDP_INK_DARK_BLUE, VDP_INK_DARK_BLUE)

bool screen_should_be_cleared = true;

extern bool copy_mode;
extern char copy_host_name;
extern unsigned char copy_host_slot;
extern bool deviceEnabled[8];

static char udg[] =
{
  0,0,0,0,0,0,3,51,                               // WIFI 1
  0,0,3,3,51,51,51,51,                            // WIFI 2
  48,48,48,48,48,48,48,48,                        // WIFI 3
  0x00,0x07,0x08,0x0F,0x0F,0x0F,0x0F,0x00,        // FOLDER 1 0x83
  0x00,0x80,0x70,0xF0,0xF0,0xF0,0xF0,0x00,        // FOLDER 2 0x84
  0x0F,0x08,0x08,0x0A,0x08,0x08,0x0F,0x00,        // DDP 1    0x85
  0xF0,0x10,0x10,0x50,0x10,0x10,0xF0,0x00,        // DDP 2    0x86
  0x0F,0x08,0x08,0x08,0x0A,0x08,0x0F,0x00,        // DSK 1    0x87
  // 0xF0,0x10,0xD0,0xD0,0xD0,0x10,0xF0,0x00,        // DSK 2    0x88
	0x00,0x7C,0x4A,0x42,0x7E,0x7E,0x7E,0x00,        // DSK 2    0x88
	0x00,0x00,0x5A,0x42,0x7E,0x7E,0x00,0x00,        // ROM 1    0x89
 	0x00,0x00,0x7E,0x42,0x5A,0x66,0x00,0x00,        // Cassette 0x8A
  // 0x0F,0x08,0x0B,0x0B,0x0B,0x08,0x0F,0x00,        // ROM 1    0x89
  // 0xF0,0x10,0xD0,0xD0,0xD0,0x10,0xF0,0x00,        // ROM 2    0x8a
 	// 0x00,0x00,0xFE,0xEE,0xEE,0xEE,0xFE,0x00,        // ROM 1    0x89
  // 0x00,0x7E,0x5A,0x5A,0x42,0x7E,0x00,0x00,        // ROM 1    0x89
	// 0x00,0x00,0xFE,0xD6,0xD6,0xD6,0xFE,0x00,        // ROM 2    0x8a
  0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55,  // Password smudge 0x8b
  0x00,0x70,0x4c,0x64,0x44,0x44,0x0e,0x00,        // F1       0x8c
  0x00,0x70,0x4c,0x62,0x44,0x48,0x0e,0x00,        // F2       0x8d
  0x00,0x70,0x4c,0x62,0x44,0x42,0x0c,0x00,        // F3       0x8e
  0x00,0x70,0x4a,0x6a,0x4e,0x42,0x02,0x00,        // F4       0x8f
  0x00,0x70,0x4e,0x68,0x4e,0x42,0x0c,0x00,        // F5       0x90
  // 0x00,0x70,0x46,0x68,0x4e,0x4a,0x06,0x00,        // F6       0x91
  // 0x00,0x70,0x4e,0x62,0x44,0x48,0x08,0x00,        // F7       0x92
  // 0x00,0x70,0x4e,0x6a,0x44,0x4a,0x0e,0x00,        // F8       0x93
  // 0x00,0x70,0x4c,0x6a,0x4e,0x42,0x0c,0x00,        // F9       0x94
  // 0x00,0xe0,0x97,0xd5,0x95,0x95,0x17,0x00,        // F10       0x95
  // 0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,        // Underline 0x96
};

// static const char *empty="EMPTY";
static const char *empty="-empty-";
static const char *off="-empty-";

bool any_slot_occupied()
{
  bool occupied = false;

  for (char i = 0; (i < MAX_DISK_SLOTS) && (!occupied); i++)
    occupied = deviceSlots[i].file[0] != 0x00;

  return occupied;
}

void show_status(char *msg)
{
  vdp_vfill(0x1400, 0x00, 256);
  gotoxy(0, 20);
  cprintf(msg);
}

void menu_clear()
{
  vdp_vfill(0x1600, 0x00, 512);
}

void show_menu(char *f1_key, char *f1_lbl,
               char *f2_key, char *f2_lbl,
               char *f3_key, char *f3_lbl,
               char *f4_key, char *f4_lbl,
               char *f5_key, char *f5_lbl)
{
  // TODO: this has gotten a little wild, should probably use var args
  uint8_t len = 5;
  uint8_t x = 0;
  style_white_on_blue();
  menu_clear();

  if (f1_lbl != NULL) {
    strcpy((char *)F1_ADDR, f1_key);
    gotoxy(0, 23);
    cputs(f1_lbl);
    len = strlen(f1_lbl);
    x += len < 6 ? 6 : len + 1;
  }
  else {
    x += 6;
  }

  if (f2_lbl != NULL) {
    strcpy((char *)F2_ADDR, f2_key);
    gotoxy(x, 23);
    cputs(f2_lbl);
    len = strlen(f2_lbl);
    x += len < 6 ? 6 : len + 1;
  }
  else {
    x += 6;
  }

  if (f3_lbl != NULL) {
    strcpy((char *)F3_ADDR, f3_key);
    gotoxy(x, 23);
    cputs(f3_lbl);
    len = strlen(f3_lbl);
    x += len < 6 ? 6 : len + 1;
  }
  else {
    x += 6;
  }

  if (f4_lbl != NULL) {
    strcpy((char *)F4_ADDR, f4_key);
    gotoxy(x, 23);
    cputs(f4_lbl);
    len = strlen(f4_lbl);
    x += len < 6 ? 6 : len + 1;
  }
  else {
    x += 6;
  }

  if (f5_lbl != NULL) {
    strcpy((char *)F5_ADDR, f5_key);
    gotoxy(x, 23);
    cputs(f5_lbl);
  }
}

void set_mode_default(void)
{
  vdp_set_mode(2);
  style_white_on_blue();
  clrscr();
}

void screen_init(void)
{
  void *param = &udg;
  console_ioctl(IOCTL_GENCON_SET_UDGS, &param);
  set_mode_default();
  // show_status("Welcome to FujiNet");
}


void screen_error(const char *c)
{
  show_status(c);
}


void screen_set_wifi(AdapterConfig* ac)
{
  style_white_on_blue();
  clrscr();
  vdp_vfill(MODE2_ATTR,0xF1,0x100);
  // vdp_vfill(MODE2_ATTR+256,0x1F,0xF00);
  vdp_vfill(MODE2_ATTR+256,0x1F,0x1000);
  gotoxy(3,16);
  style_gray_on_white();
  cprintf("MAC: %02X:%02X:%02X:%02X:%02X:%02X",ac->macAddress[0],ac->macAddress[1],ac->macAddress[2],ac->macAddress[3],ac->macAddress[4],ac->macAddress[5]);

  show_menu("s","skip",NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
  show_status("Scanning for networks...");
}


void screen_set_wifi_extended(AdapterConfigExtended *ace)
{
  screen_set_wifi((AdapterConfig *) ace);
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

  style_black_on_white();
  gotoxy(0,n+1); cprintf("%s",meter);
  // style_white_on_blue();
  cprintf("%s",ds);
}

void screen_set_wifi_select_network(unsigned char nn)
{
  char message[33];

  show_menu("h","hidden","r","rescan","s","skip", NULL, NULL, NULL, NULL);
  style_white_on_black();
  gotoxy(0,0);  cprintf("%32s","Select Network ");
  bar_set(0,3,nn,0);

  sprintf(message,"%d networks found",nn);
  style_white_on_blue();
  show_status(message);
}

void screen_set_wifi_custom(void)
{
  show_menu(NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
  show_status("  ENTER NAME OF HIDDEN NETWORK");
}

void screen_set_wifi_password(void)
{
  show_menu(NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
  show_status("  ENTER NETWORK PASSWORD\n  AND PRESS [RETURN]");
}


void screen_connect_wifi(NetConfig *nc)
{
  vdp_noblank();
  clrscr();

  show_menu(NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
  sprintf(response,"  CONNECTING TO NETWORK\n  %s",nc->ssid);
  show_status(response);

  vdp_blank();
}


void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d, bool *e)
{
  if (screen_should_be_cleared)
  {
    vdp_noblank();
    clrscr();
    screen_should_be_cleared = false;
    screen_hosts_and_devices_host_slots(h);
    screen_hosts_and_devices_device_slots(10,d,e);
    vdp_blank();
  }
}

void screen_hosts_and_devices_hosts(void)
{
  show_menu("b","boot","e","edit","d","disks","l"," lobby","c"," config");
  // show_status("  [RETURN] SELECT HOST\n  [1-8] SELECT SLOT\n  [TAB] GO TO DISK SLOTS");
  bar_clear(false);
  bar_set(0,1,8,selected_host_slot);
}

void screen_hosts_and_devices_devices(void)
{
  // show_menu("b","boot","e","eject","h","hosts","o","on/off","c","config");
  show_menu("b","boot","e","eject","h","hosts", "r","  r/w  ", "c","config");
  bar_clear(false);
  bar_set(10,1,8,selected_device_slot);
}

const char* screen_hosts_and_devices_device_slot(uint8_t hs, bool e, const char *fn)
{
  if (fn[0]!=0x00)
    return fn;
  else if (e==false)
    return &off[0];
  else
    return &empty[0];
}

char* screen_hosts_and_devices_host_slot(char *hs)
{
  return hs[0]==0x00 ? &empty[0] : hs;
}

void screen_hosts_and_devices_host_slots(HostSlot *h)
{
  textcolor(WHITE);
  textbackground(BLACK);
  gotoxy(0,0);  cprintf("%32s","Hosts ");

  vdp_vfill(MODE2_ATTR,0xF1,256); // white text, black bg
  vdp_vfill(MODE2_ATTR+0x0100,0x1F,2048); // black text, white bg

  for (char i=0;i<8;i++)
  {
      gotoxy(0,i+1);
      // textbackground(1);
      textcolor(WHITE);
      textbackground(LIGHTBLUE);
      cprintf("%d",i+1);
      // textcolor(h[i][0] == '\0' ? DARKGRAY : BLACK);
      textbackground(WHITE);
      textcolor(BLACK);
      cprintf("%-31s", screen_hosts_and_devices_host_slot(h[i]));
  }
}

#if 1
void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *d, bool *e)
{
  unsigned short y2 = y << 8;

  gotoxy(0,y); cprintf("%32s","Devices ");

  vdp_vfill(MODE2_ATTR+y2,0xF1,256); // white text, black bg
  vdp_vfill(MODE2_ATTR+y2+256,0x1F,1024); // black text, white bg

  bool has_disks = false;
  uint8_t disk_n = 0;
  uint8_t slot_n = 0;

  for (char i=0;i<MAX_DISK_SLOTS;i++)
  {
      // textcolor(15);
      gotoxy(0,i+y+1);
      textcolor(WHITE);
      textbackground(d[i].mode == 0x02 ? GREEN : LIGHTBLUE);
      char icon = ' ';
      char label = '\0';
      char *filename = d[i].file;

      if (strstr(filename, ".cas") != NULL || strstr(filename, ".CAS") != NULL) {
        icon = 0x8A;
      }
      else if (strstr(filename, ".dsk") != NULL || strstr(filename, ".DSK") != NULL) {
        icon = 0x88;
        label = 'A'+disk_n;
        disk_n++;

        if (!has_disks) {
          slot_n++;
          has_disks = true;
        }
      }
      else if (strstr(filename, ".rom") != NULL || strstr(filename, ".ROM") != NULL || strstr(filename, ".bin") != NULL || strstr(filename, ".BIN") != NULL) {
        icon = 0x89;
        label = '1'+slot_n;
        slot_n++;
      }
      cputc(icon);
      // cprintf("%d",i+1);
      // switch (i) {
      //   case 6:
      //     cputc(0x89);
      //     break;
      //   case 7:
      //     cputc(0x8A);
      //     break;
      //   default:
      //     cputc('A'+i);
      // }
      // textcolor(d[i].file[0] == '\0' ? DARKGRAY : BLACK);
      textcolor(BLACK);
      textbackground(WHITE);
      if (label == '\0') {
        cprintf("%-31s",screen_hosts_and_devices_device_slot(d[i].hostSlot,e[i],d[i].file));
      }
      else {
        cputc(label);
        cputc('=');
        cprintf("%-29s",screen_hosts_and_devices_device_slot(d[i].hostSlot,e[i],d[i].file));
      }
  }
}
#else
void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *d, bool *e)
{
  unsigned short y2 = y << 8;
  unsigned short y3 = (y+8) << 8;

  gotoxy(0,y); cprintf("%32s","Disk Drives ");

  vdp_vfill(MODE2_ATTR+y2,0xF1,256); // white text, black bg
  vdp_vfill(MODE2_ATTR+y2+256,0x1F,1024); // black text, white bg

  for (char i=0;i<MAX_DISK_SLOTS-2;i++)
  {
      // textcolor(15);
      gotoxy(0,i+y+1);
      textcolor(WHITE);
      textbackground(d[i].mode == 0x02 ? GREEN : LIGHTBLUE);
      cputc('A'+i);
      // textcolor(d[i].file[0] == '\0' ? DARKGRAY : BLACK);
      textcolor(BLACK);
      textbackground(WHITE);
      cprintf("%-31s",screen_hosts_and_devices_device_slot(d[i].hostSlot,e[i],d[i].file));
  }

  gotoxy(0,y+8); cprintf("%32s","Cartridges ");

  vdp_vfill(MODE2_ATTR+y3,0xF1,256); // white text, black bg
  vdp_vfill(MODE2_ATTR+y3+256,0x1F,512); // black text, white bg

  for (char i=0;i<2;i++)
  {
      // textcolor(15);
      gotoxy(0,i+y+9);
      textcolor(WHITE);
      textbackground(d[i].mode == 0x02 ? GREEN : LIGHTBLUE);
      cputc('1'+i);
      // textcolor(d[i].file[0] == '\0' ? DARKGRAY : BLACK);
      textcolor(BLACK);
      textbackground(WHITE);
      cprintf("%-31s",screen_hosts_and_devices_device_slot(d[i+6].hostSlot,e[i+6],d[i+6].file));
  }
}
#endif


void screen_hosts_and_devices_devices_clear_all(void)
{
  show_menu(NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
  show_status("  CLEARING ALL SLOTS...");
}

void screen_hosts_and_devices_clear_host_slot(uint_fast8_t i)
{
  gotoxy(1,i+1);
  vdp_vfill((i+1)*0x0100+8,0x00,248);
}

void screen_hosts_and_devices_edit_host_slot(uint_fast8_t i)
{

}


void screen_hosts_and_devices_eject(uint8_t ds)
{

}

void screen_hosts_and_devices_host_slot_empty(uint_fast8_t hs)
{

}


void screen_hosts_and_devices_long_filename(char *f)
{

}


void screen_show_info(bool printerEnabled,AdapterConfig* ac)
{
  vdp_noblank();
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

  vdp_vfill(MODE2_ATTR+0x0700,0xF4,256);
  vdp_vfill(MODE2_ATTR+0x0800,0x1F,256);

  for (char i = 0; i < 7 ; i++) {
    vdp_vfill(MODE2_ATTR+(i*256)+0x900,0xF4,80);
    vdp_vfill(MODE2_ATTR+(i*256)+0x900+80,0x1F,176);
  }

  show_menu("c","change ssid ", "r","reconnect", NULL,NULL, NULL,NULL, NULL,NULL);
  vdp_blank();
}


void screen_show_info_extended(bool printerEnabled, AdapterConfigExtended* ace)
{
  screen_show_info(printerEnabled, (AdapterConfig *) ace);
}


void screen_select_file(void)
{
  vdp_noblank();

  style_white_on_blue();
  clrscr();

  textcolor(WHITE);
  textbackground(BLACK);
  gotoxy(0,0);  cprintf("%32s","Select file ");

  vdp_vfill(MODE2_ATTR,0xF1,256); // white text, black bg
  vdp_vfill(MODE2_ATTR+0x0100,0x1F,0x800); // black text, white bg

  menu_clear();
  show_status("  OPENING...");

  vdp_blank();
}

void screen_select_file_display(char *p, char *f)
{
// Title
  textcolor(WHITE);
  textbackground(BLACK);
  gotoxy(0,0); cprintf("%32s", hostSlots[selected_host_slot]);

  vdp_vfill(MODE2_ATTR+0xF1,0xF1,0x200);
  vdp_vfill(MODE2_ATTR+0x0200,0x1F,0x0800);

  gotoxy(0,1);
  textbackground(WHITE);
  textcolor(BLACK);
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
  // gotoxy(0,0);
  // vdp_vfill(0x1300,0,512);
}

void screen_select_file_prev(void)
{
  // vdp_color(1,5,7);
  gotoxy(0,2); cprintf("%32s","[...]");
}

void screen_select_file_next(void)
{
  // vdp_color(1,5,7);
  gotoxy(0,18); cprintf("%32s","[...]");
}

void screen_select_file_display_entry(unsigned char y, char* e, unsigned entryType)
{
  gotoxy(0,y+2);
  // vdp_color(15,5,7);
  cprintf("%c%c",*e++,*e++);
  // *e++;*e++;
  // vdp_color(1,15,7);
  cprintf("%-30s",e);
}

void screen_select_file_choose(char visibleEntries)
{
  bool occupied = any_slot_occupied();

  screen_should_be_cleared = true;
  bar_set(2,2,visibleEntries,0); // TODO: Handle previous

  if (copy_mode == true) {
    // smartkeys_display(NULL,NULL,NULL,(strcmp(path,"/") == 0) ? NULL: "   UP"," FILTER", " PERFORM\n  COPY");
    show_menu("f","filter", "c","copy", NULL,NULL, NULL,NULL, NULL,NULL);
    show_status("Select destination");
  }
  else {
    // smartkeys_display(NULL,NULL,NULL,(strcmp(path,"/") == 0) ? NULL: "   UP"," FILTER", occupied ? "  BOOT" : "  QUICK\n  BOOT");
    show_menu("f","filter", "m","mount", "b","boot", NULL,NULL, NULL,NULL);
    show_status("Select file to mount");
  }
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

}

void screen_select_slot_choose(void)
{

}

void screen_select_slot_eject(unsigned char ds)
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

void screen_mount_and_boot()
{
  clrscr();
  bar_clear(false);
  show_status("Mounting and boot...");
}


void screen_end(void)
{
  return;
}
