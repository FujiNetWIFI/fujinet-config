#ifdef __WATCOMC__

#include <dos.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"
#include "io.h"
#include "bar.h"
#include "screen.h"
#include "globals.h"
#include "mount_and_boot.h"
#include "../hosts_and_devices.h"
#include "../select_file.h"
#include "../set_wifi.h"

unsigned char selected_network;
extern bool copy_mode;
extern unsigned char copy_host_slot;
unsigned short custom_numSectors;
unsigned short custom_sectorSize;
extern char fn[256];
bool mounting = false;
extern unsigned short entry_timer;

bool is_special_key(int kbcode)
{
    return !(kbcode & 0xFF);
}

int kbhit(void)
{
    union REGS r;

    r.h.ah = 0x01;
    int86(0x16,&r,&r);

    return r.x.ax;
}

int cgetc(void)
{
    union REGS r;
    int kbcode=0;

    r.h.ah = 0x00;
    int86(0x16,&r,&r);
    kbcode = r.x.ax;
    
    // Return full scancode, if arrow key
    if (is_special_key(kbcode))
        return r.x.ax;
   
    // Otherwise, Return ASCII value
    return r.h.al;
}

unsigned char input()
{
    if (entry_timer>0)
        entry_timer--;

    if (kbhit())
    {
        return cgetc();
    }
    /* else */
    /*     return input_handle_joystick(); */

    return 0;
}

unsigned char input_ucase()
{
    unsigned char c = input();
    if ((c >= 'a') && (c <= 'z'))
        c &= ~32;
    return c;
}

void gotoxy(unsigned char x, unsigned char y)
{
    union REGS r;

    r.h.ah = 0x02;
    r.h.bh = 0;
    r.h.dh = y;
    r.h.dl = x;
    int86(0x10,&r,&r);
}

void input_line(unsigned char x, unsigned char y, unsigned char o, char *c, unsigned char l, bool password)
{
    unsigned char pos=o;
    unsigned char key=0;
    
    c += o;
    x += o;

    gotoxy(x,y);
    
    while (key = cgetc())
    {
        if (key == 0x0D)
	{
            break;
	}
        else if (key == 0x08)
	{
            if (pos > 0)
	    {
                pos--;
                x--;
                c--;
                *c=0x00;
                screen_putc(x,y,ATTRIBUTE_NORMAL,0x08);
                screen_putc(x,y,ATTRIBUTE_NORMAL,0x20);
                screen_putc(x,y,ATTRIBUTE_NORMAL,0x80);
	    }
	}
        else if (key > 0x1F && key < 0x7F) // Printable characters
	{
            if (pos < l)
	    {
                pos++;
                x++;
                *c=key;
                c++;
                screen_putc(x,y,ATTRIBUTE_NORMAL,password ? 0x8B : key);
	    }
	}
    }
}

void input_line_set_wifi_custom(char *c)
{
    bar_set(20,1,1,0);
    memset(c, 0, 32);
    input_line(2, 20, 0, c, 32, false);
}

void input_line_set_wifi_password(char *c)
{
    char stars[65];
    int l = strlen(c);
    memset(stars, 0, 65);
    memset(stars, '*', l);
    if (l > 64) l = 64;
    stars[l] = '\0';
    screen_puts(0, 21, ATTRIBUTE_NORMAL, stars);
    input_line(0, 21, 0, c, 64, true);
}

void input_line_hosts_and_devices_host_slot(unsigned char i, unsigned char o, char *c)
{
    input_line(5, i + HOSTS_START_Y, 0, c, 32, false);
}

void input_line_filter(char *c)
{
    input_line(5, 2, 0, c, 32, false);
}

unsigned char input_select_file_new_type(void)
{
    // Not used on Atari
    return 1;
}

unsigned long input_select_file_new_size(unsigned char t)
{
    char temp[8];
    memset(temp, 0, sizeof(temp));
    input_line(34, 21, 0, temp, sizeof(temp), false);

    // TODO: make an enum so these are easier to understand
    switch (temp[0])
    {
    case '1':
        return 360;
    case '2':
        return 720;
    case '3':
        return 1440;
    case 0x1b:
        return 0;
    }

    return 0;
}

unsigned long input_select_file_new_custom(void)
{
    char tmp_str[8];
    custom_sectorSize = 512;
    custom_numSectors = 0;

    // Number of Sectors
    memset(tmp_str, 0, sizeof(tmp_str));
    input_line(11, 20, 0, tmp_str, sizeof(tmp_str), false);
    custom_numSectors = atoi(tmp_str);
    return 999;
}

void input_select_file_new_name(char *c)
{
    // TODO: Find out actual max length we shoud allow here. Input variable is [128] but do we allow filenames that large?
    input_line(0, 21, 0, c, 128, false);
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
  int k = input();

  switch (k)
  {
  case 0x4800: // Up Arrow
    // up
    if (bar_get() > NETWORKS_START_Y)
    {
      bar_up();
    }
    selected_network = bar_get() - NETWORKS_START_Y;
    return WS_SELECT;
  case 0x5000: // Down Arrow
    if (bar_get() < NETWORKS_START_Y + numNetworks)
    {
      bar_down();
    }
    selected_network = bar_get() - NETWORKS_START_Y;
    return WS_SELECT;
  case 0x1b:
    return WS_SCAN;
  case 'S':
    state = HOSTS_AND_DEVICES;
    return WS_DONE;
  case 0x1d:
    selected_network = bar_get() - NETWORKS_START_Y;
    if (selected_network < numNetworks)
    {
      set_wifi_set_ssid(bar_get() - NETWORKS_START_Y);
    }
    else
    {
      return WS_CUSTOM;
    }
    return WS_PASSWORD;

  default:
    return WS_SELECT;
  }

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
    // Up in the hosts section.
    int k = input();
    char temp[20];
    
    if (k == 0x4700) // TK-II HOME
        k = '1';
    else if (k == 0x4F00) // TK-II END
        k = '8';
    
    switch (k)
    {
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
        bar_set(HOSTS_START_Y,1,NUM_HOST_SLOTS,k-'1');
        selected_host_slot = bar_get() - HOSTS_START_Y;
        return HD_HOSTS;
    case 0x4800: // TK-II UP
        // up
        if (bar_get() > HOSTS_START_Y)
        {
            bar_up();
        }
        selected_host_slot = bar_get() - HOSTS_START_Y;
        return HD_HOSTS;
    case 0x5000:
        // down
        if (bar_get() < HOSTS_END_Y)
        {
            bar_down();
        }
        selected_host_slot = bar_get() - HOSTS_START_Y;
        return HD_HOSTS;
    case 'e':
    case 'E':
        hosts_and_devices_edit_host_slot(bar_get() - HOSTS_START_Y);
        return HD_HOSTS;
    case 0x09:
        return HD_DEVICES;
    case 'C':
    case 'c':
        state = SHOW_INFO;
        return HD_DONE;
    case 'L':
    case 'l':
        // boot lobby.
        memset(temp, 0, sizeof(temp));
        screen_puts(0,24,ATTRIBUTE_BOLD, "Boot Lobby Y/N? ");
        input_line(16,24,0,temp,2, false);
        screen_clear_line(24);
        switch (temp[0])
        {
        case 'Y':
        case 'y':
            //mount_and_boot_lobby();
            return HD_DONE;
      default: // Anything but Y/y take to mean "no"
          return HD_HOSTS;
        }
        return HD_HOSTS;
    case 0x0d:
        selected_host_slot = bar_get() - HOSTS_START_Y;
        if ( !wifiEnabled && strcmp(hostSlots[selected_host_slot],"SD") != 0) // Don't go in a TNFS host if wifi is disabled.
        {
            return HD_HOSTS;
        }
        if (hostSlots[selected_host_slot][0] != 0)
        {
            strcpy(selected_host_name, hostSlots[selected_host_slot]);
            state = SELECT_FILE;
            return HD_DONE;
        }
        else
        {
            return HD_HOSTS;
        }
    case '!':
    case 'B': // Taken from original config. What is that checking for?
        //mount_and_boot();
    default:
        return HD_HOSTS;
    }
}

HDSubState input_hosts_and_devices_devices(void)
{
    // Down in the devices section.
    int k = input();
    char temp[20];
        
    if (k == 0x4700) // Home
        k = '1';
    else if (k == 0x4F00) // End
        k = '8';
    
    switch (k)
    {
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
        bar_set(DEVICES_START_Y,1,NUM_DEVICE_SLOTS,k-'1');
        selected_device_slot = bar_get() - DEVICES_START_Y;
        return HD_DEVICES;
    case 0x5300: // Del
        return HD_CLEAR_ALL_DEVICES;
    case 0x4800: // Up arrow
        // up
        if (bar_get() > DEVICES_START_Y)
        {
            bar_up();
        }
        selected_device_slot = bar_get() - DEVICES_START_Y;
    return HD_DEVICES;
    case 0x5000: // Down Arrow
        // down
        if (bar_get() < DEVICES_END_Y)
        {
            bar_down();
        }
        selected_device_slot = bar_get() - DEVICES_START_Y;
        return HD_DEVICES;
    case 'e':
    case 'E':
        // Eject
    hosts_and_devices_eject(bar_get() - DEVICES_START_Y);
    return HD_DEVICES;
    case 0x7F:
        return HD_HOSTS;
    case 'r':
    case 'R':
        // set device mode to read
        selected_device_slot = bar_get() - DEVICES_START_Y;
        hosts_and_devices_devices_set_mode(MODE_READ);
        screen_hosts_and_devices_device_slots(DEVICES_START_Y, &deviceSlots[0], NULL);
        return HD_DEVICES;
    case 'W':
    case 'w':
        // set device mode to write
        selected_device_slot = bar_get() - DEVICES_START_Y;
        hosts_and_devices_devices_set_mode(MODE_WRITE);
        screen_hosts_and_devices_device_slots(DEVICES_START_Y, &deviceSlots[0], NULL);
        return HD_DEVICES;
    case 'C':
    case 'c':
        state = SHOW_INFO;
        return HD_DONE;
    case 'L':
    case 'l':
        // boot lobby.
        // boot lobby.
        memset(temp, 0, sizeof(temp));
        screen_puts(0,24,ATTRIBUTE_NORMAL,"Boot Lobby Y/N? ");
        input_line(16,24,0,temp,2, false);
        screen_clear_line(24);
        switch (temp[0])
        {
        case 'Y':
        case 'y':
//            mount_and_boot_lobby();
            return HD_DONE;
        default: // Anything but Y/y take to mean "no"
            return HD_DEVICES;
        }
        return HD_DEVICES;
    case '!':
        //      mount_and_boot();
    default:
        return HD_DEVICES;
    }
}

SFSubState input_select_file_choose(void)
{
    int k = input();
    unsigned entryType;

    switch (k)
    {
    case 0x4700: // HOME
        bar_set(FILES_START_Y,1,_visibleEntries,0);
        return SF_CHOOSE;
    case 0x4F00: // END
        bar_set(FILES_START_Y, 1, _visibleEntries, _visibleEntries-1);
        return SF_CHOOSE;
    case 0x4800: // UP
        entry_timer=ENTRY_TIMER_DUR;
        if ((bar_get() == FILES_START_Y) && (pos > 0))
            return SF_PREV_PAGE;
        
        if (bar_get() > FILES_START_Y)
        {
            bar_up();
        }
        return SF_CHOOSE;
    case 0x5000: // TK-II DOWN
        // down
        entry_timer=ENTRY_TIMER_DUR;
        if ((bar_get() == _visibleEntries - 1 + FILES_START_Y) && (dir_eof == false))
            return SF_NEXT_PAGE;
        
        if (bar_get() < _visibleEntries - 1 + FILES_START_Y)
        {
            bar_down();
        }
        return SF_CHOOSE;
    case 0x0d:
        pos += bar_get() - FILES_START_Y;
        screen_select_file_clear_long_filename();
        entryType = select_file_entry_type();
        
        if (entryType == ENTRY_TYPE_FOLDER)
            return SF_ADVANCE_FOLDER;
        else if (entryType == ENTRY_TYPE_LINK)
            return SF_LINK;
        else
        {
            strncpy(source_path, path, 224);
            old_pos = pos;
            return SF_DONE;
        }
    case 0x08:
        return SF_DEVANCE_FOLDER;
        
    case 0x4B00: // TK-II LEFT
        if ( strlen(path) == 1 && pos == 0 ) // We're at the root of the filesystem, and we're on the first page - go back to hosts/devices screen.
        {
            state = HOSTS_AND_DEVICES;
            return SF_DONE;
        }
        if ( pos > 0 )
            return SF_PREV_PAGE;
        else
            return SF_DEVANCE_FOLDER;
        return SF_CHOOSE;
    case 0x4D00:
        if ((ENTRIES_PER_PAGE == _visibleEntries ) && (dir_eof == false))
            return SF_NEXT_PAGE;
        return SF_CHOOSE;
    case 0x1B:
        state = HOSTS_AND_DEVICES;
        return SF_DONE;
    case 'F':
    case 'f':
        return SF_FILTER;
    case 'N':
    case 'n':
        return SF_NEW;
    case 'C':
    case 'c':
        if (copy_mode == true)
        {
            return SF_DONE;
        }
        else
        {
            pos += bar_get() - FILES_START_Y;
            select_file_set_source_filename();
            copy_host_slot = selected_host_slot;
            return SF_COPY;
        }
    case '!':
    case 'B':
        //mount_and_boot();
    default:
        return SF_CHOOSE;
    }
}

SSSubState input_select_slot_choose(void)
{
    int k=input();

    if (k == 0x4700)  // TK-II HOME
        k = '1';
    else if (k == 0x4F00) // TK-II END
        k = '8';
    
    switch (k)
    {
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
        bar_set(DEVICES_START_MOUNT_Y, 1, NUM_DEVICE_SLOTS, k - '1');
        selected_device_slot = bar_get() - DEVICES_START_MOUNT_Y;
        return SS_CHOOSE;
    case 0x4800: // UP
        if (bar_get() > DEVICES_START_MOUNT_Y)
        {
            bar_up();
        }
        selected_device_slot = bar_get() - DEVICES_START_MOUNT_Y;
        return SS_CHOOSE;
    case 0x5000: // DOWN
        // down
        if (bar_get() < DEVICES_END_MOUNT_Y)
        {
            bar_down();
        }
        selected_device_slot = bar_get() - DEVICES_START_MOUNT_Y;
        return SS_CHOOSE;
    case 'E':
    case 'e':
        // Eject
        mounting = true;
        hosts_and_devices_eject(selected_device_slot);
        mounting = false;
        return SS_CHOOSE;
    case 0x1B:
        state = SELECT_FILE;
        backToFiles = true;
        return SS_DONE;
    case 0x0D: // For Atari I think we need to ask for file mode after this, it's not in the main select_slot.c code.
        selected_device_slot = bar_get() - DEVICES_START_MOUNT_Y;
        // Ask for mode.
        screen_select_slot_mode();
        k = input_select_slot_mode(&mode);
        
        if (!k)
        {
            state = SELECT_FILE;
            backToFiles = true;
        }
        return SS_DONE;
    default:
        return SS_CHOOSE;
    }
}

unsigned char input_select_slot_mode(char *mode)
{
    unsigned char k = input();

    while (1)
    {
        switch (k)
        {
        case 0x1b:
            return 0;
            break;
        case 'W':
        case 'w':
            mode[0] = 2;
            return 1;
            break;
        case 0x0D:
        case 'R':
        case 'r':
            mode[0] = 1;
            return 1;
            break;
        default:
            break;
        }
    }
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
    while(1)
    {
        unsigned char k = input();

        switch (k)
        {
        case 'C':
        case 'c':
            state = CONNECT_WIFI;
            return SI_DONE;
        case 'S':
        case 's':
            state = SET_WIFI;
            return SI_DONE;
        default:
            state = HOSTS_AND_DEVICES;
            return SI_DONE;
        }
    }
}

DHSubState input_destination_host_slot_choose(void)
{
    int k = input();

    if (k == 0x4700) // HOME
        k = '1';
    else if (k == 0x4F00) // END
        k = '8';
    
    switch (k)
    {
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
        bar_set(HOSTS_START_Y, 1, NUM_HOST_SLOTS, k - '1');
        selected_host_slot = bar_get() - HOSTS_START_Y;
        return DH_CHOOSE;
    case 0x4800: // UP
        // up
        if (bar_get() > HOSTS_START_Y)
        {
            bar_up();
        }
        selected_host_slot = bar_get() - HOSTS_START_Y;
        return DH_CHOOSE;
    case 0x5000: // TK-II DOWN
        // down
        if (bar_get() < HOSTS_END_Y)
        {
            bar_down();
        }
        selected_host_slot = bar_get() - HOSTS_START_Y;
        return DH_CHOOSE;
    case 0x0D:
        selected_host_slot = bar_get() - HOSTS_START_Y;
        copy_mode = true;
        return DH_DONE;
    case 0x1B:
        state = HOSTS_AND_DEVICES;
        return DH_ABORT;
    default:
        return DH_CHOOSE;
    }
}


#endif /* __WATCOMC__ */
