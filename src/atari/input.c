#ifdef BUILD_ATARI

/**
 * Input routines
 */

#include <stdbool.h>
#include <stdlib.h>
#include <atari.h>
#include <conio.h>
#include <string.h>
#include <peekpoke.h>
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

unsigned char input()
{
  if (entry_timer>0)
    entry_timer--;

  if (kbhit())
  {
    rtclr();
    return cgetc();
  }
  else
    return input_handle_joystick();
}

unsigned char input_ucase()
{
  unsigned char c = input();
  if ((c >= 'a') && (c <= 'z'))
    c &= ~32;
  return c;
}

unsigned char input_handle_joystick(void)
{
  if ((OS.stick0 != 0x0F) && (OS.rtclok[2] > 6))
  {
    rtclr();
    POKE(0x4d, 0); // Turn off ATRACT (screen saver) since it doesn't turn off via joystick movement like it does with a keypress.
    switch (OS.stick0)
    {
    case 14:
      if (OS.strig0 == 1)
        return 0x1C;
      else
        return '^'; // joy up and fire
    case 13:
      return 0x1D;
    case 11:
      if (OS.strig0 == 1)
        return '<';
      else
        return '!'; // joy left and fire
    case 7:
      return '>';
    }
  }
  else if (OS.strig0 == 0)
  {
    POKE(0x4d, 0); // Turn off ATRACT (screen saver) since it doesn't turn off via joystick movement like it does with a keypress.
    return 0x9B;
  }
  else
    return 0;
}

void input_line(unsigned char x, unsigned char y, unsigned char o, char *c, unsigned char l, bool password)
{
}

void input_line_set_wifi_custom(char *c)
{
  bar_show(20);
  memset(c, 0, 32);
  edit_line(2, 20, c, 32);
}

void input_line_set_wifi_password(char *c)
{
  // bar_show(19);
  screen_puts(0, 21, c);
  edit_line(0, 21, c, 64);
}

void input_line_hosts_and_devices_host_slot(unsigned char i, unsigned char o, char *c)
{
  edit_line(5, i + HOSTS_START_Y, c, 32);
}

void input_line_filter(char *c)
{
  edit_line(5, 2, c, 32);
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
  edit_line(34, 21, temp, sizeof(temp));

  // TODO: make an enum so these are easier to understand
  switch (temp[0])
  {
  case '1':
    return 90;
  case '2':
    return 130;
  case '3':
    return 180;
  case '4':
    return 360;
  case '5':
    return 720;
  case '6':
    return 1440;
  case '7':
    return 1; // For a custom file size, the called expects 1 to be returned.
  case KCODE_ESCAPE:
    return 0;
  }
}

unsigned long input_select_file_new_custom(void)
{
  // screen_puts(0, 20, "# Sectors?");
  // screen_puts(0, 21, "Sector Size (128/256/512)?");
  //  Code copied out of fujinet-config/diskulator_select.c/diskulator_select_new_disk()
  //
  char tmp_str[8];
  custom_sectorSize = 0;
  custom_numSectors = 0;

  // Number of Sectors
  memset(tmp_str, 0, sizeof(tmp_str));
  edit_line(11, 20, tmp_str, sizeof(tmp_str));
  custom_numSectors = atoi(tmp_str);

  // Sector Size
  memset(tmp_str, 0, sizeof(tmp_str));
  while (tmp_str[0] != '1' && tmp_str[0] != '2' && tmp_str[0] != '5')
  {
    edit_line(27, 21, tmp_str, sizeof(tmp_str));
  }

  switch (tmp_str[0])
  {
  case '1':
    custom_sectorSize = 128;
    break;
  case '2':
    custom_sectorSize = 256;
    break;
  case '5':
    custom_sectorSize = 512;
    break;
  default:
    return 0;
  }
  return 999;
}

void input_select_file_new_name(char *c)
{
  // TODO: Find out actual max length we shoud allow here. Input variable is [128] but do we allow filenames that large?
  edit_line(0, 21, c, 128);
}

bool input_select_slot_build_eos_directory(void)
{
}

void input_select_slot_build_eos_directory_label(char *c)
{
}

WSSubState input_set_wifi_select(void)
{
  unsigned char k;
  unsigned char temp[29];
  k = input_ucase();

  // sprintf(temp, "y=%d, nn=%d, sn=%d", bar_get(), numNetworks, selected_network);
  // screen_debug(temp);

  switch (k)
  {
  case 0x1C:
  case '-':
    // up
    if (bar_get() > NETWORKS_START_Y)
    {
      bar_up();
    }
    selected_network = bar_get() - NETWORKS_START_Y;
    return WS_SELECT;
  case 0x1D:
  case '=':
    // down
    if (bar_get() < NETWORKS_START_Y + numNetworks)
    {
      bar_down();
    }
    selected_network = bar_get() - NETWORKS_START_Y;
    return WS_SELECT;
  case KCODE_ESCAPE:
    return WS_SCAN;
  case 'S':
    state = HOSTS_AND_DEVICES;
    return WS_DONE;
  case KCODE_RETURN:
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
}

/**
 * Handle global console keys.
 */
unsigned char input_handle_console_keys(void)
{
  return GTIA_READ.consol;
}

HDSubState input_hosts_and_devices_hosts(void)
{
  // Up in the hosts section.
  unsigned char k;
  unsigned char y;
  char temp[20];

  if (input_handle_console_keys() == 0x03) // "Option" key
  {
    mount_and_boot();
  }

  k = input_ucase();

  // sprintf(temp, "y=%d,k=%02x", bar_get(), k);
  // screen_debug(temp);

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
    bar_show(HOSTS_START_Y + (k - '1'));
    selected_host_slot = bar_get() - HOSTS_START_Y;
    return HD_HOSTS;
  case 0x1C:
  case '-':
    // up
    if (bar_get() > HOSTS_START_Y)
    {
      bar_up();
    }
    selected_host_slot = bar_get() - HOSTS_START_Y;
    return HD_HOSTS;
  case 0x1D:
  case '=':
    // down
    if (bar_get() < HOSTS_END_Y)
    {
      bar_down();
    }
    selected_host_slot = bar_get() - HOSTS_START_Y;
    return HD_HOSTS;
  case 'E':
    hosts_and_devices_edit_host_slot(bar_get() - HOSTS_START_Y);
    return HD_HOSTS;
  case 0x7F:
    return HD_DEVICES;
  case 'C':
    state = SHOW_INFO;
    return HD_DONE;
  case 'L':
    // boot lobby.
    memset(temp, 0, sizeof(temp));
    screen_puts(0,24,"Boot Lobby Y/N? ");
    edit_line(16,24,temp,2);
    screen_clear_line(24);
    switch (temp[0])
    {
      case 'Y':
      case 'y':
        mount_and_boot_lobby();
        return HD_DONE;
      default: // Anything but Y/y take to mean "no"
        return HD_HOSTS;
      }
    return HD_HOSTS;
  case KCODE_RETURN:
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
    mount_and_boot();
  default:
    return HD_HOSTS;
  }
}

HDSubState input_hosts_and_devices_devices(void)
{
  // Down in the devices section.
  unsigned char k;
  unsigned char i;
  char temp[20];

  if (input_handle_console_keys() == 0x03)
  {
    mount_and_boot();
  }

  k = input_ucase();

  // sprintf(temp, "y=%d", bar_get());
  // screen_debug(temp);

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
    bar_show(DEVICES_START_Y + (k - '1'));
    selected_device_slot = bar_get() - DEVICES_START_Y;
    return HD_DEVICES;
  case CH_CLR: // Clear
    return HD_CLEAR_ALL_DEVICES;
  case 0x1C:
  case '-':
    // up
    if (bar_get() > DEVICES_START_Y)
    {
      bar_up();
    }
    selected_device_slot = bar_get() - DEVICES_START_Y;
    return HD_DEVICES;
  case 0x1D:
  case '=':
    // down
    if (bar_get() < DEVICES_END_Y)
    {
      bar_down();
    }
    selected_device_slot = bar_get() - DEVICES_START_Y;
    return HD_DEVICES;
  case 'E':
    // Eject
    hosts_and_devices_eject(bar_get() - DEVICES_START_Y);
    return HD_DEVICES;
  case 0x7F:
    return HD_HOSTS;
  case 'R':
    // set device mode to read
    selected_device_slot = bar_get() - DEVICES_START_Y;
    set_device_slot_mode(selected_device_slot, 1);
    screen_hosts_and_devices_device_slots(DEVICES_START_Y, &deviceSlots[0], "");
    return HD_DEVICES;
  case 'W':
    // set device mode to write
    selected_device_slot = bar_get() - DEVICES_START_Y;
    set_device_slot_mode(selected_device_slot, 2);
    screen_hosts_and_devices_device_slots(DEVICES_START_Y, &deviceSlots[0], "");
    return HD_DEVICES;
  case 'C':
    state = SHOW_INFO;
    return HD_DONE;
  case 'L':
    // boot lobby.
    // boot lobby.
    memset(temp, 0, sizeof(temp));
    screen_puts(0,24,"Boot Lobby Y/N? ");
    edit_line(16,24,temp,2);
    screen_clear_line(24);
    switch (temp[0])
    {
      case 'Y':
      case 'y':
        mount_and_boot_lobby();
        return HD_DONE;
      default: // Anything but Y/y take to mean "no"
        return HD_DEVICES;
      }
    return HD_DEVICES;
  case '!':
    mount_and_boot();
  default:
    return HD_DEVICES;
  }
}

SFSubState input_select_file_choose(void)
{
  unsigned char k;
  unsigned char y;
  unsigned char temp[30];
  unsigned entryType;

  if (input_handle_console_keys() == 0x03)
  {
    mount_and_boot();
  }

  k = input_ucase();

  //sprintf(temp, "y=%d,ve=%d,pos=%d,eof=%d", bar_get(), _visibleEntries, pos, dir_eof);
  //screen_debug(temp);

  switch (k)
  {
  case 0x1C:
  case '-':
    // up
    entry_timer=ENTRY_TIMER_DUR;
    if ((bar_get() == FILES_START_Y) && (pos > 0))
      return SF_PREV_PAGE;

    if (bar_get() > FILES_START_Y)
    {
      bar_up();
    }
    return SF_CHOOSE;
  case 0x1D:
  case '=':
    // down
    entry_timer=ENTRY_TIMER_DUR;
    if ((bar_get() == _visibleEntries - 1 + FILES_START_Y) && (dir_eof == false))
      return SF_NEXT_PAGE;

    if (bar_get() < _visibleEntries - 1 + FILES_START_Y)
    {
      bar_down();
    }
    return SF_CHOOSE;
  case KCODE_RETURN:
  case '*': // took from fujinet-config
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
  case KCODE_BACKSP:
    return SF_DEVANCE_FOLDER;

  case '<':
  case '+':
    if ( strlen(path) == 1 && pos <= 0 ) // We're at the root of the filesystem, and we're on the first page - go back to hosts/devices screen.
    {
      state = HOSTS_AND_DEVICES;
      return SF_DONE;
    }
    if ( pos > 0 )
      return SF_PREV_PAGE;
    else
      return SF_DEVANCE_FOLDER;
    return SF_CHOOSE;
  case '>':
    if ((ENTRIES_PER_PAGE == _visibleEntries ) && (dir_eof == false))
      return SF_NEXT_PAGE;
    return SF_CHOOSE;
  case KCODE_ESCAPE:
    state = HOSTS_AND_DEVICES;
    return SF_DONE;
  case 'F':
    return SF_FILTER;
  case 'N':
    return SF_NEW;
  case 'C':
    if (copy_mode == true)
    {
      return SF_DONE;
    }
    else
    {
      //pos = bar_get() - FILES_START_Y;
      pos += bar_get() - FILES_START_Y;
      select_file_set_source_filename();
      copy_host_slot = selected_host_slot;
      return SF_COPY;
    }
  case '!':
  case 'B':
    mount_and_boot();
  default:
    return SF_CHOOSE;
  }
}

SSSubState input_select_slot_choose(void)
{
  unsigned char k;
  unsigned char y;
  unsigned char temp[30];
  if (input_handle_console_keys() == 0x03)
  {
    mount_and_boot();
  }

  k = input_ucase();

  // sprintf(temp, "y=%d,pos=%d,sds=%d", bar_get(), pos, selected_device_slot);
  // screen_debug(temp);

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
    bar_show(DEVICES_START_MOUNT_Y + (k - '1'));
    selected_device_slot = bar_get() - DEVICES_START_MOUNT_Y;
    return SS_CHOOSE;
  case 0x1C:
  case '-':
    // up
    if (bar_get() > DEVICES_START_MOUNT_Y)
    {
      bar_up();
    }
    selected_device_slot = bar_get() - DEVICES_START_MOUNT_Y;
    return SS_CHOOSE;
  case 0x1D:
  case '=':
    // down
    if (bar_get() < DEVICES_END_MOUNT_Y)
    {
      bar_down();
    }
    selected_device_slot = bar_get() - DEVICES_START_MOUNT_Y;
    return SS_CHOOSE;
  case 'E':
    // Eject
    mounting = true;
    hosts_and_devices_eject(selected_device_slot);
    mounting = false;
    return SS_CHOOSE;
  case KCODE_ESCAPE:
    state = SELECT_FILE;
    backToFiles = true;
    return SS_DONE;
  case KCODE_RETURN: // For Atari I think we need to ask for file mode after this, it's not in the main select_slot.c code.
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
  unsigned char k = 0;

  while (k == 0)
  {
    k = input_ucase();

    if (k == KCODE_ESCAPE)
    {
      return 0;
    }

    if (k == 'W')
    {
      mode[0] = 2;
    }
    else
      mode[0] = 1;
  }
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
  unsigned char k = 0;

  while (k == 0)
  {
    k = input_ucase();
  }

  switch (k)
  {
  case 'C':
    state = CONNECT_WIFI;
    return SI_DONE;
  case 'S':
    state = SET_WIFI;
    return SI_DONE;
  default:
    state = HOSTS_AND_DEVICES;
    return SI_DONE;
  }
}

DHSubState input_destination_host_slot_choose(void)
{
  // Up in the hosts section.
  unsigned char k;
  char temp[20];

  k = input_ucase();

  // sprintf(temp, "y=%d", bar_get());
  // screen_debug(temp);

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
    bar_show(HOSTS_START_Y + (k - '1'));
    selected_host_slot = bar_get() - HOSTS_START_Y;
    return DH_CHOOSE;
  case 0x1C:
  case '-':
    // up
    if (bar_get() > HOSTS_START_Y)
    {
      bar_up();
    }
    selected_host_slot = bar_get() - HOSTS_START_Y;
    return DH_CHOOSE;
  case 0x1D:
  case '=':
    // down
    if (bar_get() < HOSTS_END_Y)
    {
      bar_down();
    }
    selected_host_slot = bar_get() - HOSTS_START_Y;
    return DH_CHOOSE;
  case KCODE_RETURN:
    selected_host_slot = bar_get() - HOSTS_START_Y;
    copy_mode = true;
    return DH_DONE;
  case KCODE_ESCAPE:
    state = HOSTS_AND_DEVICES;
    return DH_ABORT;
  default:
    return DH_CHOOSE;
  }
}

void set_device_slot_mode(unsigned char slot, unsigned char mode)
{
  unsigned char tmp_hostSlot;
  unsigned char tmp_file[FILE_MAXLEN];

  if ( deviceSlots[slot].hostSlot == 0xFF )
  {
    return;
  }

  tmp_hostSlot = deviceSlots[slot].hostSlot;
  memcpy(tmp_file, deviceSlots[slot].file, FILE_MAXLEN);
  io_get_filename_for_device_slot(slot, fn);

  io_umount_disk_image(slot);

  deviceSlots[slot].hostSlot = tmp_hostSlot;
  deviceSlots[slot].mode = mode;
  memcpy(deviceSlots[slot].file, tmp_file, FILE_MAXLEN);

  io_set_device_filename(slot, tmp_hostSlot, mode, fn);
  io_put_device_slots(&deviceSlots[0]);
  io_mount_disk_image(slot, mode);

  // If we couldn't mount read/write, then re-mount again as read-only
  /*
  in original config, this repeated same log (using same mode..)
  if ( io_error() )
  {
    io_umount_disk_image(slot);

  }
  */
}
#endif
