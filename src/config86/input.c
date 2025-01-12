#ifdef BUILD_CONFIG86

/**
 * Input routines
 */
#include <stdio.h>
#include <dos.h>
#include "input.h"
#include "io.h"
#include "bar.h"
#include "screen.h"
#include "globals.h"
#include "mount_and_boot.h"
#include "../hosts_and_devices.h"
#include "../select_file.h"
#include "../set_wifi.h"
#include "../select_slot.h"

unsigned char selected_network;
extern bool copy_mode;
extern unsigned char copy_host_slot;
unsigned short custom_numSectors;
unsigned short custom_sectorSize;
extern char fn[256];
bool mounting = false;
extern unsigned short entry_timer;
extern HDSubState hd_subState;

extern unsigned char _screen_color;

int input()
{
    union REGS r;

    r.h.ah = 0x00;
    int86(0x16,&r,&r);

    return r.x.ax;
}

void input_line(unsigned char x, unsigned char y, char *c, unsigned char l, bool password)
{
    union REGS r;
    int k=0, pos=0;

    while(k!=0x0d)
    {
        screen_gotoxy(x,y);
        k=input() & 0xFF;

        if (!k)
            continue; /* Ignore extended keys */

        r.h.ah = 0x09;
        r.h.bh = 0;
        r.h.bl = _screen_color;
        r.x.cx = 1;
        
        if (pos && k==0x08)
        {
            pos--;
            x--;
            r.h.al = 0x20;
        }
        else
        {
            pos++;
            x++;
            if (password)
                r.h.al = '*';
            else
                r.h.al = k;
        }

        int86(0x10,&r,0);
    }
}

void input_line_set_wifi_custom(char *c)
{
  input_line(0,15,c,32,false);
}

void input_line_set_wifi_password(char *c)
{
  input_line(0,15,c,64,true);
}

void input_line_hosts_and_devices_host_slot(int i, unsigned int o, char *c)
{
  bar_clear(true);
  input_line(1,(unsigned char)i+1,c,32,false);
}

void input_line_filter(char *c)
{
    input_line(0,15,c,31,false);
}

unsigned char input_select_file_new_type(void)
{
    return 1; // We always wanna make a DSK
}

unsigned long input_select_file_new_size(unsigned char t)
{
    return 0UL;
}

unsigned long input_select_file_new_custom(void)
{
  return 0;
}

void input_select_file_new_name(char *c)
{
    input_line(0,15,c,255,false);
}

bool input_select_slot_build_eos_directory(void)
{
  return false;
}

void input_select_slot_build_eos_directory_label(char *c)
{
}

WSSubState input_set_wifi_select(void)
{
  return WS_SELECT;
}

/**
 * Handle global console keys.
 */
unsigned char input_handle_console_keys(void)
{
  return 0;
}

HDSubState input_hosts_and_devices_hosts(void)
{
    int k=input();

    switch(k)
    {
    case 0x0231:
    case 0x0332:
    case 0x0433:
    case 0x0534:
    case 0x0635:
    case 0x0736:
    case 0x0837:
    case 0x0938:
        bar_jump(k-'1');
        return HD_HOSTS;
    case 0x2E63:
    case 0x2E43:
        state = SHOW_INFO;
        return HD_DONE;
    case 0x4800:
        bar_up();
        selected_host_slot = bar_get();
        return HD_HOSTS;
    case 0x5000:
        bar_down();
        selected_host_slot = bar_get();
        return HD_HOSTS;
    }
    return HD_HOSTS;
}

HDSubState input_hosts_and_devices_devices(void)
{
  return HD_DEVICES;
}

SFSubState input_select_file_choose(void)
{  
  return SF_CHOOSE;
}

SSSubState input_select_slot_choose(void)
{
  return SS_CHOOSE;
}

unsigned char input_select_slot_mode(char *mode)
{
  return 1;
}

/*
 *  Handle inupt for the "show info" screen.
 *
 *  'C' - Reconnect Wifi
 *  'S' - Change SSID
 *  Any other key - return to main hosts and devices screen
 *
 */
SISubState input_show_info(void)
{
    int k=input();

    switch(k)
    {
    case 0x2E42:
    case 0x2E63:
        state = CONNECT_WIFI;
        return SI_DONE;
    case 0x1453:
    case 0x1473:
        state = CONNECT_WIFI;
        return SI_DONE;
    default:
        state = HOSTS_AND_DEVICES;
        return SI_DONE;
    }
    
    return SI_SHOWINFO;
}

DHSubState input_destination_host_slot_choose(void)
{
  return DH_CHOOSE;
}

void set_device_slot_mode(unsigned char slot, unsigned char mode)
{
}

#endif /* BUILD_CONFIG86 */
