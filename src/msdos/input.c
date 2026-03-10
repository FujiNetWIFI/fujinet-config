#ifdef BUILD_MSDOS

#include <dos.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "../input.h"
#include "cursor.h"
#include "io.h"
#include "screen.h"
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
    static union REGS r;

    r.h.ah = 0x01;
    int86(0x16,(union REGS *)&r,(union REGS *)&r);

    return r.x.ax;
}

unsigned char cgetc(void)
{
    static union REGS r;

    r.h.ah = 0x00;
    int86(0x16,(union REGS *)&r,(union REGS *)&r);

    if (r.h.al != 0)
        return r.h.al;

    // Extended key: map scan code to single-byte constant
    switch (r.h.ah)
    {
    case 0x48: return KEY_UP_ARROW;
    case 0x50: return KEY_DOWN_ARROW;
    case 0x4B: return KEY_LEFT_ARROW;
    case 0x4D: return KEY_RIGHT_ARROW;
    case 0x47: return KEY_HOME;
    case 0x4F: return KEY_END;
    case 0x53: return KEY_DELETE;
    case 0x49: return KEY_PAGE_UP;
    case 0x51: return KEY_PAGE_DOWN;
    default:   return 0;
    }
}

unsigned char input()
{
    if (entry_timer>0)
        entry_timer--;

    if (kbhit())
    {
        return cgetc();
    }

    return 0;
}

unsigned char input_ucase()
{
    return 0;
}

void gotoxy(unsigned char x, unsigned char y)
{
    static union REGS r;

    r.h.ah = 0x02;
    r.h.bh = 0;
    r.h.dh = y;
    r.h.dl = x;
    int86(0x10,(union REGS *)&r,(union REGS *)&r);
}

void input_line(unsigned char x, unsigned char y, unsigned char o, char *c, unsigned char l, bool password)
{
    unsigned char pos=o;
    unsigned char key=0;
    
    c += o;
    x += o;

    cursor(true);
    gotoxy(x,y);

    while (key = cgetc())
    {
        unsigned int vo;

        if (key == KEY_RETURN)
        {
            break;
        }
        else if (key == KEY_ESCAPE)
        {
            break;
        }
        else if (key == KEY_BACKSPACE)
        {
            if (pos > 0)
            {
                pos--;
                x--;
                c--;
                *c = 0x00;
                vo = (unsigned int)(y * screen_cols + x) * 2;
                video[vo]     = ' ';
                video[vo + 1] = ATTRIBUTE_NORMAL;
                gotoxy(x, y);
            }
        }
        else if (key > 0x1F && key < 0x7F) // Printable characters
        {
            if (pos < l)
            {
                *c = key;
                vo = (unsigned int)(y * screen_cols + x) * 2;
                video[vo]     = password ? '*' : key;
                video[vo + 1] = ATTRIBUTE_NORMAL;
                c++;
                pos++;
                x++;
                gotoxy(x, y);
            }
        }
    }

    cursor(false);
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
    memset((void *)stars, 0, 65);
    memset((void *)stars, '*', l);
    if (l > 64) l = 64;
    stars[l] = '\0';
    screen_puts(0, 21, ATTRIBUTE_NORMAL, (const char *)stars);
    input_line(0, 21, 0, c, 64, true);
}

void input_line_hosts_and_devices_host_slot(uint_fast8_t i, uint_fast8_t o, char *c)
{
    input_line(5, i + HOSTS_START_Y, o, c, 32, false);
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
    memset((void *)temp, 0, sizeof(temp));
    input_line(34, 21, 0, (char *)temp, sizeof(temp), false);

    // TODO: make an enum so these are easier to understand
    switch (temp[0])
    {
    case '1':
        return 360;
    case '2':
        return 720;
    case '3':
        return 1440;
    case KEY_ESCAPE:
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
    memset((void *)tmp_str, 0, sizeof(tmp_str));
    input_line(11, 20, 0, (char *)tmp_str, sizeof(tmp_str), false);
    custom_numSectors = atoi((const char *)tmp_str);
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
  case KEY_UP_ARROW:
    bar_up();
    selected_network = bar_get();
    return WS_SELECT;
  case KEY_DOWN_ARROW:
    bar_down();
    selected_network = bar_get();
    return WS_SELECT;
  case KEY_ESCAPE:
    return WS_SCAN;
  case 'S':
  case 's':
    state = HOSTS_AND_DEVICES;
    return WS_DONE;
  case KEY_CTRL_RBRACKET:
    selected_network = bar_get();
    if (selected_network < numNetworks)
    {
      set_wifi_set_ssid(bar_get());
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
    
    if (k == KEY_HOME) // TK-II HOME
        k = '1';
    else if (k == KEY_END) // TK-II END
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
        selected_host_slot = bar_get();
        return HD_HOSTS;
    case KEY_UP_ARROW:
        bar_up();
        selected_host_slot = bar_get();
        return HD_HOSTS;
    case KEY_DOWN_ARROW:bar_down();
        selected_host_slot = bar_get();
        return HD_HOSTS;
    case 'e':
    case 'E':
        hosts_and_devices_edit_host_slot(bar_get());
        return HD_HOSTS;
    case KEY_TAB:
        return HD_DEVICES;
    case 'C':
    case 'c':
        state = SHOW_INFO;
        return HD_DONE;
    case 'L':
    case 'l':
        // boot lobby.
        memset((void *)temp, 0, sizeof(temp));
        screen_puts(0,24,ATTRIBUTE_BOLD, "Boot Lobby Y/N? ");
        input_line(16,24,0,(char *)temp,2, false);
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
    case KEY_RETURN:
        selected_host_slot = bar_get();
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
    case KEY_ESCAPE:
      return HD_DONE;
    default:
        return HD_HOSTS;
    }
}

HDSubState input_hosts_and_devices_devices(void)
{
    // Down in the devices section.
    int k = input();
    char temp[20];
        
    if (k == KEY_HOME) // Home
        k = '1';
    else if (k == KEY_END) // End
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
        selected_device_slot = bar_get();
        return HD_DEVICES;
    case KEY_DELETE:
        return HD_CLEAR_ALL_DEVICES;
    case KEY_UP_ARROW:
        bar_up();
        selected_device_slot = bar_get();
        return HD_DEVICES;
    case KEY_DOWN_ARROW:
        bar_down();
        selected_device_slot = bar_get();
        return HD_DEVICES;
    case 'e':
    case 'E':
        // Eject
        hosts_and_devices_eject(bar_get());
        return HD_DEVICES;
    case KEY_TAB:
        return HD_HOSTS;
    case 'r':
    case 'R':
        // set device mode to read
        selected_device_slot = bar_get();
        hosts_and_devices_devices_set_mode(MODE_READ);
        screen_hosts_and_devices_device_slots(DEVICES_START_Y, &deviceSlots[0], NULL);
        return HD_DEVICES;
    case 'W':
    case 'w':
        // set device mode to write
        selected_device_slot = bar_get();
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
        memset((void *)temp, 0, sizeof(temp));
        screen_puts(0,24,ATTRIBUTE_NORMAL,"Boot Lobby Y/N? ");
        input_line(16,24,0,(char *)temp,2, false);
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
    case KEY_ESCAPE:
        return HD_DONE;
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
    case KEY_HOME:
        bar_set(FILES_START_Y,1,_visibleEntries,0);
        return SF_CHOOSE;
    case KEY_END:
        bar_set(FILES_START_Y, 1, _visibleEntries, _visibleEntries-1);
        return SF_CHOOSE;
    case KEY_PAGE_UP:
        if (pos > 0)
            return SF_PREV_PAGE;
        return SF_CHOOSE;
    case KEY_PAGE_DOWN:
        if ((ENTRIES_PER_PAGE == _visibleEntries) && (dir_eof == false))
            return SF_NEXT_PAGE;
        return SF_CHOOSE;
    case KEY_UP_ARROW:
        entry_timer=ENTRY_TIMER_DUR;
        if ((bar_get() == 0) && (pos > 0))
            return SF_PREV_PAGE;

        bar_up();
        return SF_CHOOSE;
    case KEY_DOWN_ARROW:
        // down
        entry_timer=ENTRY_TIMER_DUR;
        if ((bar_get() == _visibleEntries - 1) && (dir_eof == false))
            return SF_NEXT_PAGE;

        bar_down();
        return SF_CHOOSE;
    case KEY_RETURN:
        pos += bar_get();
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
    case KEY_BACKSPACE:
        return SF_DEVANCE_FOLDER;
    case KEY_LEFT_ARROW:
        if ( strlen(path) == 1 && pos == 0 ) // at root, first page — go back to hosts/devices
        {
            state = HOSTS_AND_DEVICES;
            return SF_DONE;
        }
        if ( pos > 0 )
            return SF_PREV_PAGE;
        else
            return SF_DEVANCE_FOLDER;
        return SF_CHOOSE;
    case KEY_RIGHT_ARROW:
        if ((ENTRIES_PER_PAGE == _visibleEntries ) && (dir_eof == false))
            return SF_NEXT_PAGE;
        return SF_CHOOSE;
    case KEY_ESCAPE:
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
            pos += bar_get();
            select_file_set_source_filename();
            copy_host_slot = selected_host_slot;
            return SF_COPY;
        }
    case '!':
    case 'B':
    case 'b':
        //mount_and_boot();
    default:
        return SF_CHOOSE;
    }
}

SSSubState input_select_slot_choose(void)
{
    int k=input();

    if (k == KEY_HOME)  // TK-II HOME
        k = '1';
    else if (k == KEY_END) // TK-II END
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
        selected_device_slot = bar_get();
        return SS_CHOOSE;
    case KEY_UP_ARROW:
        bar_up();
        selected_device_slot = bar_get();
        return SS_CHOOSE;
    case KEY_DOWN_ARROW:
        bar_down();
        selected_device_slot = bar_get();
        return SS_CHOOSE;
    case 'E':
    case 'e':
        // Eject
        mounting = true;
        hosts_and_devices_eject(selected_device_slot);
        mounting = false;
        return SS_CHOOSE;
    case KEY_ESCAPE:
        state = SELECT_FILE;
        backToFiles = true;
        return SS_DONE;
    case KEY_RETURN: // For Atari I think we need to ask for file mode after this, it's not in the main select_slot.c code.
        selected_device_slot = bar_get();
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
        case KEY_ESCAPE:
            return 0;
            break;
        case 'W':
        case 'w':
            mode[0] = 2;
            return 1;
            break;
        case KEY_RETURN:
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

    if (k == KEY_HOME) // HOME
        k = '1';
    else if (k == KEY_END) // END
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
        selected_host_slot = bar_get();
        return DH_CHOOSE;
    case KEY_UP_ARROW:
        bar_up();
        selected_host_slot = bar_get();
        return DH_CHOOSE;
    case KEY_DOWN_ARROW:
        bar_down();
        selected_host_slot = bar_get();
        return DH_CHOOSE;
    case KEY_RETURN:
        selected_host_slot = bar_get();
        copy_mode = true;
        return DH_DONE;
    case KEY_ESCAPE:
        state = HOSTS_AND_DEVICES;
        return DH_ABORT;
    default:
        return DH_CHOOSE;
    }
}


#endif /* BUILD_MSDOS */
