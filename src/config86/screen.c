#ifdef BUILD_CONFIG86

/**
 * FujiNet Configuration Program: screen functions
 * The screen functions are a common set of graphics and text string functions to display information to the video output device.
 * These screen functions is comprised of two files; screen.c and screen.h.  Screen.c sets up the display dimensions, memory and
 * initializes the display as well as provide various text manipulations functions for proper display.  The screen.h file include
 * defines for various items such as keyboard codes, functions and screen memory addresses.
 *
 **/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <dos.h>
#include "screen.h"
#include "io.h"
#include "globals.h"
#include "bar.h"
#include "input.h"

unsigned char *video_ptr;  // a pointer to the memory address containing the screen contents
unsigned char *cursor_ptr; // a pointer to the current cursor position on the screen
char _visibleEntries;
extern bool copy_mode;
char text_empty[] = "Empty";
char fn[256];
extern HDSubState hd_subState;
extern DeviceSlot deviceSlots[NUM_DEVICE_SLOTS];
extern HostSlot hostSlots[8];

unsigned char _screen_mode = 0;
unsigned char _screen_cols = 0;
unsigned char _screen_color = 0x07;
unsigned char _screen_x = 0;
unsigned char _screen_y = 0;

char uppercase_tmp[32]; // temp space for strupr(s) output.
                               // so original strings doesn't get changed.

#define SCREEN_CHAR_HLINE 0xCD

/**
 * @brief Place cursor at home position (0,0)
 */
void screen_gotoxy(unsigned char x, unsigned char y)
{
    union REGS r;

    _screen_x = x;
    _screen_y = y;
    
    r.h.ah = 0x02;
    r.h.bh = 0x00;
    r.h.dl = x;
    r.h.dh = y;

    int86(0x10, &r, 0);
}

/**
 * @brief Clear screen
 */
void screen_clear(void)
{
    union REGS r;
    
    screen_gotoxy(0,0);
    
    /* Fill screen with spaces */
    r.h.ah = 0x09;
    r.h.al = 0x20; /* SPACE */
    r.h.bl = _screen_color;
    r.h.bh = 0;
    r.x.cx = _screen_cols * 25;
    int86(0x10,&r,0);
    
    screen_gotoxy(0,0);
}

/**
 * @brief Clear line at y
 */
void screen_clear_line(unsigned char y)
{
    union REGS r;

    screen_gotoxy(0,0);

    /* Fill line with spaces */
    r.h.ah = 0x09;
    r.h.al = 0x20; /* SPACE */
    r.h.bl = _screen_color;
    r.h.bh = 0;
    r.x.cx = _screen_cols;
    
    int86(0x10,&r,0);
}

/**
 * @brief Put single character at cursor
 */
void screen_putc(const char c)
{
    union REGS r;

    screen_gotoxy(_screen_x, _screen_y);
    
    r.h.ah = 0x09;
    r.h.al = c;
    r.h.bl = _screen_color;
    r.h.bh = 0;
    r.x.cx = 1;

    int86(0x10,&r,0);
    _screen_x++;
}

/**
 * @brief BIOS only screen puts. Yes this is slow, but it's COMPATIBLE.
 */
void screen_puts(const char *s)
{
    char c=0;
    
    while (c = *s++)
        screen_putc(c);
}

/**
 * @brief puts but right aligned on Y, taking _screen_cols into account.
 */
void screen_putsr(unsigned char y, const char *s)
{
    screen_gotoxy(_screen_cols-strlen(s),y);
    screen_puts(s);
}

/**
 * @brief Put horizontal line at Y
 * @param y Position to place horizontal line.
 */
void screen_hline(unsigned char y)
{
    union REGS r;
    
    screen_gotoxy(0,y);

    r.h.ah = 0x09;
    r.h.al = SCREEN_CHAR_HLINE;
    r.h.bl = _screen_color;
    r.h.bh = 0;
    r.x.cx = _screen_cols;
    int86(0x10,&r,0);
}

void screen_mount_and_boot()
{
    unsigned char x = (_screen_cols == 40 ? 20 : 35);
    unsigned char y = 12;
    
    screen_clear();
    screen_gotoxy(x,y);

    screen_puts("Mounting and Booting...");
}

void screen_set_wifi(AdapterConfig *ac)
{
    char s[128];
    
    screen_clear();
    bar_clear(false);
    screen_gotoxy(_screen_cols == 40 ? 10 : 30,0);
    screen_puts("WELCOME TO FUJINET!");
    screen_gotoxy(_screen_cols == 40 ? 10 : 30,24);
    screen_puts("SCANNING NETWORKS...");
    sprintf(s,"MAC Address: %02X:%02X:%02X:%02X:%02X:%02X",ac->macAddress[0],ac->macAddress[1],ac->macAddress[2],ac->macAddress[3],ac->macAddress[4],ac->macAddress[5]);
    screen_gotoxy(_screen_cols == 40 ? 10 : 30,2);
    screen_puts(s);
}

void screen_set_wifi_display_ssid(char n, SSIDInfo *s)
{
    unsigned char x = (_screen_cols == 40 ? 35 : 72);
    
    screen_gotoxy(2,n+NETWORKS_START_Y);
    screen_puts((char *)s->ssid);

    screen_gotoxy(x,n+NETWORKS_START_Y);
    
    if (s->rssi > -40)
    {
        screen_putc(0xFE);
        screen_putc(0xFE);
        screen_putc(0xFE);
    }
    else if (s->rssi > -60)
    {
        screen_putc(0xFE);
        screen_putc(0xFE);
    }
    else
    {
        screen_putc(0xFE);
    }
}

void screen_set_wifi_select_network(unsigned char nn)
{
    screen_clear_line(23);
    screen_clear_line(24);
    screen_gotoxy(5,23);
    _screen_color=0x09;
    screen_putc(0x18);
    _screen_color=0x07;
    screen_putc('/');
    _screen_color=0x09;
    screen_putc(0x19);
    _screen_color=0x07;
    screen_puts(" Choose Network  ");
    _screen_color=0x09;
    screen_putc('H');
    _screen_color=0x07;
    screen_puts("idden SSID       ");
    _screen_color=0x09;
    screen_putc('R');
    _screen_color=0x07;
    screen_puts("escan Networks  ");
    _screen_color=0x09;
    screen_puts("\x11\xC4\xD9");
    _screen_color=0x07;
    screen_puts(" Select Network");
    bar_set(2,1,nn,0);
}

void screen_set_wifi_custom(void)
{
    unsigned char x = (_screen_cols == 40 ? 15 : 35);

    screen_clear_line(22);
    screen_clear_line(23);
    screen_clear_line(24);
    screen_gotoxy(x,22);
    screen_puts("Enter Name  of Hidden Network");
}

void screen_set_wifi_password(void)
{
    screen_clear_line(22);
    screen_clear_line(23);
    screen_clear_line(24);

    screen_gotoxy(0,22);
    screen_puts("   Enter Network Password;  Press \x11\xC4\xD9   ");
}

/*
 * Display the 'info' screen
 */
void screen_show_info(int printerEnabled, AdapterConfig *ac)
{
    char s[128];
    unsigned char x = (_screen_cols == 40 ? 0 : 15);
    unsigned char y = 3
        ;

    screen_gotoxy(x,y++);

    screen_puts("\xC9\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBB");

    screen_gotoxy(x,y++);
    screen_puts("\xBA     ");
    _screen_color=0x0f;
    screen_puts("F U J I N E T    C O N F I G");
    _screen_color=0x07;
    screen_puts("     \xBA");

    screen_gotoxy(x,y++);
    screen_puts("\xCC\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xB9");

    screen_gotoxy(x,y++);
    sprintf(s,"\xBA%12s%-26s\xBA","SSID: ",ac->ssid);
    screen_puts(s);

    screen_gotoxy(x,y++);
    sprintf(s,"\xBA%12s%-26s\xBA","Hostname: ",ac->hostname);
    screen_puts(s);

    screen_gotoxy(x,y++);
    screen_puts("\xCC\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xB9");

    screen_gotoxy(x,y++);
    sprintf(s,"\xBA%12s%3u.%3u.%3u.%3u%11s\xBA","IP Address: ",ac->localIP[0],ac->localIP[1],ac->localIP[2],ac->localIP[3],"");
    screen_puts(s);

    screen_gotoxy(x,y++);
    sprintf(s,"\xBA%12s%3u.%3u.%3u.%3u%11s\xBA","Gateway: ",ac->gateway[0],ac->gateway[1],ac->gateway[2],ac->gateway[3],"");
    screen_puts(s);

    screen_gotoxy(x,y++);
    sprintf(s,"\xBA%12s%3u.%3u.%3u.%3u%11s\xBA","DNS: ",ac->dnsIP[0],ac->dnsIP[1],ac->dnsIP[2],ac->dnsIP[3],"");
    screen_puts(s);

    screen_gotoxy(x,y++);
    sprintf(s,"\xBA%12s%3u.%3u.%3u.%3u%11s\xBA","Netmask: ",ac->netmask[0],ac->netmask[1],ac->netmask[2],ac->netmask[3],"");
    screen_puts(s);

    screen_gotoxy(x,y++);
    screen_puts("\xCC\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xB9");

    screen_gotoxy(x,y++);
    sprintf(s,"\xBA%12s%02X:%02X:%02X:%02X:%02X:%02X%9s\xBA","MAC: ",ac->macAddress[0],ac->macAddress[1],ac->macAddress[2],ac->macAddress[3],ac->macAddress[4],ac->macAddress[5],"");
    screen_puts(s);

    screen_gotoxy(x,y++);
    sprintf(s,"\xBA%12s%02X:%02X:%02X:%02X:%02X:%02X%9s\xBA","BSSID: ",ac->bssid[0],ac->bssid[1],ac->bssid[2],ac->bssid[3],ac->bssid[4],ac->bssid[5],"");
    screen_puts(s);

    screen_gotoxy(x,y++);
    screen_puts("\xCC\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xB9");

    screen_gotoxy(x,y++);
    sprintf(s,"\xBA%12s%-26s\xBA","Version: ",ac->fn_version);
    screen_puts(s);

    screen_gotoxy(x,y++);
    screen_puts("\xCC\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xB9");

    screen_gotoxy(x,y++);
    screen_puts("\xBA      ");
    _screen_color=0x09;
    screen_putc('C');
    _screen_color=0x07;
    screen_puts(" Reconnect ");
    _screen_color=0x09;
    screen_putc('S');
    _screen_color=0x07;
    screen_puts(" Change SSID       \xBA");

    screen_gotoxy(x,y++);
    screen_puts("\xBA      ");
    _screen_color=0x09;
    screen_puts("Any other key  to return.");
    _screen_color=0x07;
    screen_puts("       \xBA");
    _screen_color=0x07;

    screen_gotoxy(x,y++);
    screen_puts("\xC8\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBC");
}

void screen_select_slot(const char *e)
{
    char s[256];
    
    struct _additl_info
    {
        unsigned char year;
        unsigned char month;
        unsigned char day;
        unsigned char hour;
        unsigned char min;
        unsigned char sec;
        unsigned long size;
        unsigned char isdir;
        unsigned char trunc;
        unsigned char type;
        unsigned char *filename;
    } *i = (struct _additl_info *)e;
  
    screen_clear();

    screen_hline(0);
    screen_gotoxy(_screen_cols == 40 ? 20 : 60,0);
    screen_puts("PLACE IN DEVICE SLOT:");

    screen_hosts_and_devices_device_slots(1,&deviceSlots[0],&deviceEnabled[0]);

    screen_hline(11);
    screen_gotoxy(_screen_cols == 40 ? 28 : 68,11);
    screen_puts("FILE DETAILS");

    screen_gotoxy(0,12);
    sprintf(s,"%8s 20%02u-%02u-%02u %02u:%02u:%02u\n","MTIME:",i->year,i->month,i->day,i->hour,i->min,i->sec);
    screen_puts(s);
    
    screen_gotoxy(0,13);
    sprintf(s,"%8s %lu K\n","SIZE:",i->size >> 10); // Quickly divide by 1024
    screen_puts(s);

    screen_gotoxy(0,14);
    screen_puts(&e[13]);

    screen_gotoxy(0,22);
    _screen_color = 0x09;
    screen_putc(0x18);
    _screen_color = 0x07;
    screen_putc('/');
    _screen_color = 0x09;
    screen_putc(0x19);
    _screen_color = 0x07;

    screen_puts(" Select Slot ");

    _screen_color = 0x09;
    screen_puts("\x11\xC4\xD9");
    _screen_color = 0x07;
    screen_puts(" Mount Read-Only ");

    _screen_color = 0x09;
    screen_putc('W');
    _screen_color = 0x07;
    screen_puts(" Mount Read-Write");

    _screen_color = 0x09;
    screen_puts("ESC");
    _screen_color = 0x07;
    screen_puts(" Abort");
    
    bar_set(1,1,NUM_DEVICE_SLOTS,0);
}

void screen_select_slot_mode(void)
{
    screen_puts("screen_select_slot_mode()\r\n");
}

void screen_select_slot_choose(void)
{
    screen_puts("screen_select_slot_choose()\r\n");
}

void screen_select_slot_eject(unsigned char ds)
{
    screen_puts("screen_select_slot_eject()\r\n");
}

void screen_select_slot_build_eos_directory(void)
{
    screen_puts("screen_select_slot_build_eos_directory()\r\n");
}

void screen_select_slot_build_eos_directory_label(void)
{
    screen_puts("screen_select_slot_build_eos_directory_label()\r\n");
}

void screen_select_slot_build_eos_directory_creating(void)
{
    screen_puts("screen_select_slot_build_eos_directory_creating()\r\n");
}

void screen_select_file(void)
{
    screen_puts("screen_select_file()\r\n");
}

void screen_select_file_display(char *p, char *f)
{
    screen_puts("screen_select_file_display()\r\n");
}

void screen_select_file_display_long_filename(const char *e)
{
    screen_puts("screen_select_file_display_long_filename()\r\n");
}

void screen_select_file_clear_long_filename(void)
{
    screen_puts("screen_select_file_clear_long_filename()\r\n");
}

void screen_select_file_filter(void)
{
    screen_puts("screen_select_file_filter()\r\n");
}

void screen_select_file_next(void)
{
    screen_puts("screen_select_file_next()\r\n");
}

void screen_select_file_prev(void)
{
    screen_puts("screen_select_file_prev()\r\n");
}

void screen_select_file_display_entry(unsigned char y, const char *e, unsigned entryType)
{
    screen_puts("screen_select_file_display_entry()\r\n");
}

void screen_select_file_choose(char visibleEntries)
{
    screen_puts("screen_select_file_choose()\r\n");
}

void screen_select_file_new_type(void)
{
    screen_puts("screen_select_file_new_type()\r\n");
}

void screen_select_file_new_size(unsigned char k)
{
    screen_puts("screen_select_file_new_size()\r\n");
}

void screen_select_file_new_custom(void)
{
    screen_puts("screen_select_file_new_custom()\r\n");
}

void screen_select_file_new_name(void)
{
    screen_puts("screen_select_file_new_names()\r\n");
}

void screen_select_file_new_creating(void)
{
    screen_puts("screen_select_file_new_creating()\r\n");
}

void screen_error(const char *msg)
{
    screen_error(msg);
}

void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d, bool *e)
{
    screen_clear();

    screen_hline(1);
    screen_gotoxy(_screen_cols-12,1);
    screen_puts("HOST SLOTS");

    screen_hline(10);
    screen_gotoxy(_screen_cols-13,10);
    screen_puts("DRIVE SLOTS");

    screen_hosts_and_devices_host_slots(&hostSlots[0]);
    screen_hosts_and_devices_device_slots(DEVICES_START_Y, &deviceSlots[0], NULL);

}

// Show the keys that are applicable when we are on the Hosts portion of the screen.
void screen_hosts_and_devices_hosts()
{
    screen_clear_line(23);
    screen_clear_line(24);

    screen_gotoxy(0,23);
    _screen_color = 0x09;
    screen_puts("1-8");
    _screen_color = 0x07;
    screen_puts(" Slot ");

    _screen_color = 0x09;
    screen_puts("E");
    _screen_color = 0x07;
    screen_puts("dit ");

    _screen_color = 0x09;
    screen_puts("\x11\xC4\xD9");
    _screen_color = 0x07;
    screen_puts(" Browse ");

    _screen_color = 0x09;
    screen_puts("L");
    _screen_color = 0x07;
    screen_puts("obby ");

    if (_screen_cols==40)
        screen_gotoxy(2,24);

    _screen_color = 0x09;
    screen_puts("C");
    _screen_color = 0x07;
    screen_puts("onfig ");

    _screen_color = 0x09;
    screen_puts("TAB");
    _screen_color = 0x07;
    screen_puts(" Drive Slots ");

    _screen_color = 0x09;
    screen_puts("ESC");
    _screen_color = 0x07;
    screen_puts(" Boot ");

    bar_set(2,1,8,selected_host_slot);
}

// Show the keys that are applicable when we are on the Devices portion of the screen.
void screen_hosts_and_devices_devices()
{
    screen_puts("screen_hosts_and_devices_devices()\r\n");
}

void screen_hosts_and_devices_host_slots(HostSlot *h)
{
    unsigned char slotNum=0;

    for (slotNum=0;slotNum<NUM_HOST_SLOTS; slotNum++)
    {
        screen_gotoxy(2,slotNum+HOSTS_START_Y);
        screen_putc(slotNum + '1');
        screen_putc(0x20);
        screen_putc(0x20);

        if (hostSlots[slotNum][0])
            screen_puts(hostSlots[slotNum]);
        else
            screen_puts(text_empty);
    }
}

void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *dslot, bool *e)
{
    unsigned char slotNum;
    unsigned char dinfo[6];

    for (slotNum = 0; slotNum < NUM_DEVICE_SLOTS; slotNum++)
    {
        dinfo[1] = 0x20;
        dinfo[2] = slotNum + 'A';
        dinfo[3] = 0x20;
        dinfo[4] = 0x20;
        dinfo[5] = 0x00;

        if (deviceSlots[slotNum].file[0])
        {
            dinfo[0] = deviceSlots[slotNum].hostSlot+'1';
            dinfo[3] = (deviceSlots[slotNum].mode == 0x02 ? 'W' : 'R');
        }
        else
        {
            dinfo[0] = 0x20;
            dinfo[3] = 0x20;
        }

        screen_gotoxy(0,slotNum+y);
        screen_puts(dinfo);

        screen_puts(deviceSlots[slotNum].file[0] != 0x00 ? (char *)deviceSlots[slotNum].file : text_empty);        
    }
}

void screen_hosts_and_devices_devices_clear_all(void)
{
    screen_puts("screen_hosts_and_devices_devices_clear_all()\r\n");
}

void screen_hosts_and_devices_clear_host_slot(int i)
{
    screen_puts("screen_hosts_and_devices_clear_host_slot()\r\n");
}

void screen_hosts_and_devices_edit_host_slot(int i)
{
    screen_puts("screen_hosts_and_devices_edit_host_slot()\r\n");
}

void screen_hosts_and_devices_eject(unsigned char ds)
{
  screen_hosts_and_devices_devices();
}

void screen_hosts_and_devices_host_slot_empty(int hs)
{
    screen_puts("screen_hosts_and_devices_host_slot_empty()\r\n");
}

void screen_hosts_and_devices_long_filename(const char *f)
{
    screen_puts("screen_hosts_and_devices_long_filename()\r\n");
}

void screen_init(void)
{
    union REGS r;

    /* Get Relevant current video mode attributes. */
    r.h.ah = 0x0F;

    int86(0x10,&r,&r);

    _screen_mode = r.h.al;
    _screen_cols = r.h.ah;
}

void screen_destination_host_slot(char *h, char *p)
{
    screen_puts("screen_destination_host_slot()\r\n");
}

void screen_destination_host_slot_choose(void)
{
    screen_puts("screen_destination_host_slot_choose()\r\n");
}

void screen_perform_copy(char *sh, char *p, char *dh, char *dp)
{
    screen_puts("screen_perform_copy()\r\n");
}

void screen_connect_wifi(NetConfig *nc)
{
    screen_puts("screen_connect_wifi()\r\n");
}

#endif
