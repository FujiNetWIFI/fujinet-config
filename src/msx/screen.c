/**
 * FujiNet CONFIG for MSX
 *
 * Screen Routines
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <arch/z80.h>
#include <video/tms99x8.h>
#include <graphics.h>
#include <conio.h>
#include <sys/ioctl.h>
#include "../screen.h"
#include "../constants.h"
#include "../globals.h"
#include "gfxutil.h"

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

// #define style_black_on_white() vdp_color(VDP_INK_BLACK, VDP_INK_WHITE, VDP_INK_DARK_BLUE)
#define style_black_on_white() vdp_color(VDP_INK_BLACK, VDP_INK_WHITE, VDP_INK_BLACK)
#define style_gray_on_white() vdp_color(VDP_INK_GRAY, VDP_INK_WHITE, VDP_INK_DARK_BLUE)
// #define style_white_on_black() vdp_color(VDP_INK_WHITE, VDP_INK_BLACK, VDP_INK_DARK_BLUE)
#define style_white_on_black() vdp_color(VDP_INK_WHITE, VDP_INK_BLACK, VDP_INK_BLACK)
#define style_white_on_blue() vdp_color(VDP_INK_WHITE, VDP_INK_DARK_BLUE, VDP_INK_DARK_BLUE)
#define style_highlighted() vdp_color(VDP_INK_WHITE, VDP_INK_DARK_BLUE, VDP_INK_DARK_BLUE)

bool screen_should_be_cleared = true;

extern bool copy_mode;
extern char copy_host_name;
extern unsigned char copy_host_slot;
extern bool deviceEnabled[8];

// static const char *empty="EMPTY";
static const char *empty="-empty-";
static const char *off="-off-";

bool any_slot_occupied()
{
  bool occupied = false;

  for (char i = 0; (i < MAX_DISK_SLOTS) && (!occupied); i++)
    occupied = deviceSlots[i].file[0] != 0x00;

  return occupied;
}

void keyboard_click(bool on) {
  z80_bpoke(0xF3DB, on ? 1 : 0);
}

void clear_status(void)
{
  vdp_vfill(0x1400, 0x00, 512);
}

void show_status(char *msg)
{
  uint8_t len = strlen(msg);
  style_white_on_black();
  clear_status();
  gotoxy((32-len)/2, 20);
  puts(msg);
}

void menu_clear()
{
  vdp_vfill(0x1600, 0x00, 512);
}

uint8_t print_menu_option(char *label) {
  bool highlight = false;
  uint8_t len = 0;
  unsigned char c;

  textcolor(DARKGRAY);

  while ((c = *label) != '\0') {
    if (c == '_') {
      highlight = true;
    }
    else if (highlight) {
      textcolor(WHITE);
      cputc(c);
      textcolor(DARKGRAY);
      highlight = false;
      len++;
    }
    else {
      cputc(c);
      len++;
    }
    label++;
  }

  return len;
}

void show_menu(char *f1_key, char *f1_lbl,
               char *f2_key, char *f2_lbl,
               char *f3_key, char *f3_lbl,
               char *f4_key, char *f4_lbl,
               char *f5_key, char *f5_lbl)
{
  // TODO: this has gotten a little wild, should probably use var args
  uint8_t len = 5;
  uint8_t x = 1;
  // style_white_on_blue();
  style_white_on_black();
  menu_clear();

  if (f1_lbl != NULL) {
    strcpy((char *)F1_ADDR, f1_key);
    gotoxy(x, 23);
    len = print_menu_option(f1_lbl);
    x += len < 6 ? 6 : len + 1;
  }
  else {
    x += 6;
  }

  if (f2_lbl != NULL) {
    strcpy((char *)F2_ADDR, f2_key);
    gotoxy(x, 23);
    len = print_menu_option(f2_lbl);
    x += len < 6 ? 6 : len + 1;
  }
  else {
    x += 6;
  }

  if (f3_lbl != NULL) {
    strcpy((char *)F3_ADDR, f3_key);
    gotoxy(x, 23);
    len = print_menu_option(f3_lbl);
    x += len < 6 ? 6 : len + 1;
  }
  else {
    x += 6;
  }

  if (f4_lbl != NULL) {
    strcpy((char *)F4_ADDR, f4_key);
    gotoxy(x, 23);
    len = print_menu_option(f4_lbl);
    x += len < 6 ? 6 : len + 1;
  }
  else {
    x += 6;
  }

  if (f5_lbl != NULL) {
    strcpy((char *)F5_ADDR, f5_key);
    gotoxy(x, 23);
    print_menu_option(f5_lbl);
  }
}

void hide_menu(void)
{
  show_menu(NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
}

void draw_card(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t margin, char *title)
{
  uint8_t title_len = 0;
  if (title != NULL)
    title_len = strlen(title);

  textcolor(WHITE); textbackground(BLACK);
  gotoxy(x,y); putchar(CH_BOX_UL);
  for (uint8_t i = 0; i < w-title_len-3; i++) putchar(CH_BOX_U);
  // for (uint8_t i = 0; i < 23; i++) putchar(CH_BOX_U);
  putchar(CH_TAB_L);

  textcolor(BLACK); textbackground(WHITE);
  if (title != NULL)
    cputs(title);

  textcolor(WHITE); textbackground(BLACK);
  putchar(CH_TAB_R);

  for (uint8_t i = 0; i < h-2; i++) {
    gotoxy(x, y+i+1);
    putchar(CH_BOX_L);
    gotoxy(x+w-1, y+i+1);
    putchar(CH_BOX_R);
  }

  gotoxy(x,y+h-1); putchar(CH_BOX_BL);
  for (uint8_t i = 0; i < w-2; i++) putchar(CH_BOX_B);
  putchar(CH_BOX_BR);

  uint16_t addr = MODE2_ATTR + (y+1) * 0x100;
  for (uint8_t row = 0; row < h-2; row++) {
    vdp_vwrite(row_pattern, addr, 256);
    if (margin) vdp_vfill(addr+8, 0x1F, margin << 3);
    addr += 256;
  }
}

void set_mode_default(void)
{
  vdp_set_mode(2);
  // style_white_on_blue();
  style_white_on_black();
  clrscr();
}

void screen_init(void)
{
  void *param = &udg;
  console_ioctl(IOCTL_GENCON_SET_UDGS, &param);
  set_mode_default();
  keyboard_click(false);
}


void screen_error(const char *c)
{
  show_status(c);
}


void screen_set_wifi(AdapterConfig* ac)
{
  vdp_noblank();
  style_white_on_black();
  clrscr();

  draw_card(0, 0, 32, 18, 3, "Select Network");

  gotoxy(3,18);
  cprintf("MAC: %02X:%02X:%02X:%02X:%02X:%02X",ac->macAddress[0],ac->macAddress[1],ac->macAddress[2],ac->macAddress[3],ac->macAddress[4],ac->macAddress[5]);

  show_menu("s","_skip",NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
  show_status("Scanning for networks...");

  vdp_blank();
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

  // style_black_on_white();
  gotoxy(1,n+1); cprintf("%s ",meter);
  // style_white_on_blue();
  cprintf("%s",ds);
}

void screen_set_wifi_select_network(unsigned char nn)
{
  char message[33];

  show_menu("h","_hidden","r","_rescan","s","_skip", NULL, NULL, NULL, NULL);
  // style_white_on_black();
  // gotoxy(0,0);  cprintf("%32s","Select Network ");
  bar_set(0,5,nn,0);

  draw_card(0, 0, 32, 18, 3, "Select Network");

  sprintf(message,"%d networks found",nn);
  show_status(message);
}

void screen_set_wifi_custom(void)
{
  hide_menu();
  show_status("Enter network name");
}

void screen_set_wifi_password(void)
{
  hide_menu();
  show_status("Enter network password");
}


void screen_connect_wifi(NetConfig *nc)
{
  vdp_noblank();
  style_white_on_black();
  clrscr();

  hide_menu();
  sprintf(response,"     Connecting to network\n  %s",nc->ssid);
  show_status(response);

  vdp_blank();
}


void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d, bool *e)
{
  if (screen_should_be_cleared)
  {
    vdp_noblank();
    style_white_on_black();
    clrscr();
    bar_clear(false);
    screen_should_be_cleared = false;

    screen_hosts_and_devices_host_slots(h);
    screen_hosts_and_devices_device_slots(10,d,e);

    vdp_blank();
  }
}

void screen_hosts_and_devices_hosts(void)
{
  show_menu("b","_boot","e","_edit","d","_disks", "s","ba_sic", "c","_config");
  clear_status();
  bar_set(0,3,8,selected_host_slot);
}

void screen_hosts_and_devices_devices(void)
{
  show_menu("b","_boot","e","_eject","h","_hosts", "r"," _r/w", "_c","config");
  bar_set(10,3,8,selected_device_slot);
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
  for (uint8_t i = 0; i < 8; i++) {
    gotoxy(1, i+1);
    cputc('1'+i);
    cprintf(" %.28s", screen_hosts_and_devices_host_slot(h[i]));
  }

  draw_card(0, 0, 32, 10, 1, "Hosts");
}

void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *d, bool *e)
{
  // bool has_disks = false;
  // uint8_t disk_n = 0;
  // uint8_t slot_n = 0;

  for (char i=0;i<MAX_DISK_SLOTS;i++)
  {
    gotoxy(1,i+y+1);
    char icon = ' ';
    char label = '\0';
    char *filename = d[i].file;

    if (strstr(filename, ".cas") != NULL || strstr(filename, ".CAS") != NULL) {
      icon = 0x8A;
    }
    else if (strstr(filename, ".dsk") != NULL || strstr(filename, ".DSK") != NULL) {
      icon = 0x88;
      // label = 'A'+disk_n;
      // disk_n++;

      // if (!has_disks) {
        // slot_n++;
        // has_disks = true;
      // }
    }
    else if (strstr(filename, ".rom") != NULL || strstr(filename, ".ROM") != NULL || strstr(filename, ".bin") != NULL || strstr(filename, ".BIN") != NULL) {
      icon = 0x89;
      // label = '1'+slot_n;
      // slot_n++;
    }
    cputc(icon);
    // if (label == '\0') {
    //   cprintf(" %-29s",screen_hosts_and_devices_device_slot(d[i].hostSlot,e[i],d[i].file));
    // }
    // else {
    //   cprintf(" %c=%-27s",label, screen_hosts_and_devices_device_slot(d[i].hostSlot,e[i],d[i].file));
    // }

    if (filename[0] == '\0') {
      cprintf(" %.28s",
        screen_hosts_and_devices_device_slot(d[i].hostSlot,e[i],d[i].file));
    }
    else {
      cprintf(" %u:%.25s",
        d[i].hostSlot+1,
        screen_hosts_and_devices_device_slot(d[i].hostSlot,e[i],d[i].file));
    }
  }

  draw_card(0, y, 32, 10, 1, "Devices");
}


void screen_hosts_and_devices_devices_clear_all(void)
{
  hide_menu();
  show_status("CLEARING ALL SLOTS...");
}

void screen_hosts_and_devices_clear_host_slot(uint_fast8_t i)
{
  gotoxy(1,i+1);
  vdp_vfill((i+1)*0x0100+8,0x00,248);
}

void screen_hosts_and_devices_edit_host_slot(uint_fast8_t i)
{
  menu_clear();
  show_status("Enter new host name");
  textcolor(BLACK);
  textbackground(WHITE);
}


void screen_hosts_and_devices_eject(uint8_t ds)
{
  // vdp_vfill(0x0c00+(ds<<8)+8,0x00,248);
  textcolor(BLACK);
  textbackground(WHITE);
  gotoxy(1,11+ds); cprintf(empty);
  bar_jump(bar_get());
}

void screen_hosts_and_devices_host_slot_empty(uint_fast8_t hs)
{
  textcolor(BLACK);
  textbackground(WHITE);
  gotoxy(1,1+hs); cprintf(empty);
}


void screen_hosts_and_devices_long_filename(char *f)
{
  // vdp_vfill(0x1100,0x00,1024); // Clear area first
  if (strlen(f)>31)
  {
    gotoxy(0,17);
    cprintf("%s",f);
  }
}


void screen_show_info(bool printerEnabled,AdapterConfig* ac)
{
  vdp_noblank();
  clrscr();
  bar_clear(false);

  gotoxy(0,7);

  cprintf(" %8s %-20s\n","SSID",ac->ssid);
  cprintf(" %8s %s\n","HOSTNAME",ac->hostname);
  cprintf(" %8s %u.%u.%u.%u\n","IP",ac->localIP[0],ac->localIP[1],ac->localIP[2],ac->localIP[3]);
  cprintf(" %8s %u.%u.%u.%u\n","NETMASK",ac->netmask[0],ac->netmask[1],ac->netmask[2],ac->netmask[3]);
  cprintf(" %8s %u.%u.%u.%u\n","DNS",ac->dnsIP[0],ac->dnsIP[1],ac->dnsIP[2],ac->dnsIP[3]);
  cprintf(" %8s %02x:%02x:%02x:%02x:%02x:%02x\n","MAC",ac->macAddress[0],ac->macAddress[1],ac->macAddress[2],ac->macAddress[3],ac->macAddress[4],ac->macAddress[5]);
  cprintf(" %8s %02x:%02x:%02x:%02x:%02x:%02x\n","BSSID",ac->bssid[0],ac->bssid[1],ac->bssid[2],ac->bssid[3],ac->bssid[4],ac->bssid[5]);
  cprintf(" %8s %s\n","FN VER",ac->fn_version);
  cprintf(" %8s %s\n","CONFIG",GIT_VERSION);

  draw_card(0, 5, 32, 13, 8, "Configuration");

  show_menu("e","_edit", "r","_reconnect", NULL,NULL, NULL,NULL, NULL,NULL);
  vdp_blank();
}


void screen_show_info_extended(bool printerEnabled, AdapterConfigExtended* ace)
{
  screen_show_info(printerEnabled, (AdapterConfig *) ace);
}


void screen_select_file(void)
{
  vdp_noblank();

  style_white_on_black();
  clrscr();
  bar_clear(false);

  draw_card(0, 0, 32, 19, 0, hostSlots[selected_host_slot]);

  hide_menu();
  show_status("Opening...");

  vdp_blank();
}

void screen_select_file_display(char *p, char *f)
{
  gotoxy(1,1);
  if (f[0]==0x00)
    cprintf("%30s",p);
  else
    cprintf("%-8s|%22s",p,f);

  // TODO: swap for optimized function
  uint16_t addr = MODE2_ATTR + 0x200 + 8;
  for (uint8_t y = 0; y < ENTRIES_PER_PAGE; y++) {
    gotoxy(1,y+2);
    cprintf("%30s"," ");
    vdp_vwrite(row_pattern+8, addr, 30<<3);
    addr += 0x100;
  }
}

void screen_select_file_display_long_filename(char *e)
{
  gotoxy(1,19);
  cprintf("%-64s",e);
}

void screen_select_file_clear_long_filename(void)
{
  // gotoxy(0,0);
  // vdp_vfill(0x1300,0,512);
}

void screen_select_file_prev(void)
{
  textcolor(WHITE);
  textbackground(BLUE);
  gotoxy(2,2); cprintf("%-28s","...");
}

void screen_select_file_next(void)
{
  textcolor(WHITE);
  textbackground(BLUE);
  gotoxy(2,17); cprintf("%-28s","...");
}

void screen_select_file_display_entry(unsigned char y, char* e, unsigned entryType)
{
  gotoxy(2,y+2);
  textbackground(BLUE);
  cprintf("%.28s",e);

  // TODO: swap for optimized function
  uint16_t addr = MODE2_ATTR + 0x200 + 0x100*y + 16;
  vdp_vwrite(row_pattern + 8, addr, 28<<3);
  // gfx_putsxy("hello", 2, y+2);
}

void screen_select_file_choose(char visibleEntries)
{
  bool occupied = any_slot_occupied();

  screen_should_be_cleared = true;
  bar_set(1,2,visibleEntries,0); // TODO: Handle previous

  if (copy_mode == true) {
    show_status("Select destination");
    show_menu("c","_copy", NULL,NULL, NULL,NULL, NULL,NULL, NULL,NULL);
  }
  else {
    show_status("Select a file to mount");
    show_menu("m","_mount", "b"," _boot", "c"," _copy", "n","  _new", "f"," _filter");
  }
}

void screen_select_file_filter(void)
{
  show_status("Enter filter");
  hide_menu();
}

void screen_select_file_new_type(void)
{
  show_status("What type of disk?");
  show_menu("1","MSX-DOS _1", "2"," MSX-DOS _2", NULL,NULL, NULL,NULL, "b","_blank");
}

void screen_select_file_new_size(unsigned char k)
{
  show_status("Disk size?");
  show_menu("3","_360K", "7","_720K", NULL,NULL, NULL,NULL, "c","_custom");
}

void screen_select_file_new_custom(void)
{
  show_status("Enter disk size");
  hide_menu();
}

void screen_select_file_new_name(void)
{
  show_status("Enter disk filename");
  hide_menu();
}

void screen_select_file_new_creating(void)
{
  show_status("Creating disk...");
  hide_menu();
}


void screen_select_slot(char *e)
{
  vdp_noblank();
  // style_white_on_blue();
  clrscr();

  // TODO: Also need to display filename?

  screen_hosts_and_devices_device_slots(0,&deviceSlots[0],&deviceEnabled[0]);

  bar_set(0,3,8,0);

  vdp_blank();
}

void screen_select_slot_choose(void)
{
  show_status("Choose where to mount");
  if (create) {
    show_menu("r","_read-only", NULL,NULL, NULL,NULL, NULL,NULL, "w","read/_write");
  }
  else {
    hide_menu();
  }
}

void screen_select_slot_eject(unsigned char ds)
{
  textcolor(BLACK);
  textbackground(WHITE);
  gotoxy(1,1+ds); cprintf("%s",empty);
  bar_jump(bar_get());
}


void screen_destination_host_slot(char *h, char *p)
{
  vdp_noblank();

  // style_white_on_blue();
  clrscr();

  gotoxy(0,10); cprintf("%32s","COPY FROM HOST SLOT");
  gotoxy(0,11); cprintf("%32s",h);
  // vdp_color(1,15,7);
  // gotoxy(0,12); cprintf("%-128s",p);

  vdp_blank();
}

void screen_destination_host_slot_choose(void)
{
  show_status("Choose a destination");
  hide_menu();
}


void screen_perform_copy(char *sh, char *p, char *dh, char *dp)
{
  vdp_noblank();

  // style_white_on_blue();
  clrscr();

  show_status("Copying file...");
  hide_menu();

  gotoxy(0,0); vdp_color(15,4,7); cprintf("%32s","COPYING FILE FROM:");
  gotoxy(0,1); cprintf("%32s",sh);
  gotoxy(0,2); vdp_color(1,15,7); cprintf("%-128s",p);
  gotoxy(0,6); vdp_color(15,4,7); cprintf("%32s",dh);
  gotoxy(0,7); vdp_color(1,15,7); cprintf("%-128s",dp);

  vdp_blank();
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
