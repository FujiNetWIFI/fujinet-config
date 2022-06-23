#ifdef BUILD_ATARI

/**
 * Input routines
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <atari.h>
#include <conio.h>
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

unsigned char input()
{
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
    return 0x9B;
  else
    return 0;
}

void input_line(unsigned char x, unsigned char y, unsigned char o, char *c, unsigned char l, bool password)
{
}

void input_line_set_wifi_custom(char *c)
{
  bar_show(20);
  _screen_input(2, 20, c, 32);
}

void input_line_set_wifi_password(char *c)
{
  //bar_show(19);
  _screen_input(0, 21, c, 32);
}

void input_line_hosts_and_devices_host_slot(unsigned char i, unsigned char o, char *c)
{
  _screen_input(5, i + HOSTS_START_Y, c, 32);
}

void input_line_filter(char *c)
{
  _screen_input(5, 2, c, 32);
}

unsigned char input_select_file_new_type(void)
{
  // Not used on Atari
  return 1;
}

unsigned long input_select_file_new_size(unsigned char t)
{
  char temp[2];
  memset(temp, 0, 2);
  _screen_input(34, 21, &temp, 2);

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
  case 'C':
    return 1;
  }
}

unsigned long input_select_file_new_custom(void)
{
}

void input_select_file_new_name(char *c)
{
  // TODO: Find out actual max length we shoud allow here. Input variable is [128] but do we allow filenames that large?
  _screen_input(0, 21, c, 128);
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

  sprintf(temp, "y=%d, nn=%d, sn=%d", bar_get(), numNetworks, selected_network);
  screen_debug(temp);

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

  sprintf(temp, "y=%d", bar_get());
  screen_debug(temp);

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
  case KCODE_RETURN:
    selected_host_slot = bar_get() - HOSTS_START_Y;
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
    // Do we need a new substate here MOUNT_AND_BOOT for Atari?
  }

  k = input_ucase();

  sprintf(temp, "y=%d", bar_get());
  screen_debug(temp);

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
    screen_clear_line(11);
    for (i = 0; i < NUM_DEVICE_SLOTS; i++)
    {
      if (i % 2)
      {
        screen_puts(4, 11, "EJECTING ALL");
      }
      else
      {
        screen_clear_line(11);
      }
      hosts_and_devices_eject(i);
    }
    screen_clear_line(11);
    screen_puts(4, 11, "DRIVE SLOTS");
    return HD_DEVICES;
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
    return HD_DEVICES;
  case 'W':
    // set device mode to write
    return HD_DEVICES;
  case 'C':
    state = SHOW_INFO;
    return HD_DONE;
  default:
    return HD_DEVICES;
  }
}

SFSubState input_select_file_choose(void)
{
  unsigned char k;
  unsigned char y;
  unsigned char temp[30];

  if (input_handle_console_keys() == 0x03)
  {
    // Do we need a new substate here MOUNT_AND_BOOT for Atari?
  }

  k = input_ucase();

  sprintf(temp, "y=%d,ve=%d,pos=%d,shs=%d", bar_get(), _visibleEntries, pos, selected_host_slot);
  screen_debug(temp);

  switch (k)
  {
  case 0x1C:
  case '-':
    // up
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
    if ((bar_get() == _visibleEntries - 1 + FILES_START_Y) && (dir_eof == false))
      return SF_NEXT_PAGE;

    if (bar_get() < _visibleEntries - 1 + FILES_START_Y)
    {
      bar_down();
    }
    return SF_CHOOSE;
  case KCODE_RETURN:
    pos = bar_get() - FILES_START_Y;
    if (select_file_is_folder())
      return SF_ADVANCE_FOLDER;
    else
    {
      state = SELECT_SLOT;
      return SF_DONE;
    }
  // Have to try these on real atari, see if they work.
  case CH_CURS_LEFT:
    if ((bar_get() == FILES_START_Y) && (pos > 0))
      return SF_PREV_PAGE;
  case CH_CURS_RIGHT:
    if ((bar_get() == _visibleEntries - 1 + FILES_START_Y) && (dir_eof == false))
      return SF_NEXT_PAGE;
  case KCODE_ESCAPE:
    state = HOSTS_AND_DEVICES;
    return SF_DONE;
  case 'F':
    return SF_FILTER;
  case 'N':
    return SF_NEW;
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
    // Do we need a new substate here MOUNT_AND_BOOT for Atari?
  }

  k = input_ucase();

  sprintf(temp, "y=%d,pos=%d,sds=%d", bar_get(), pos, selected_device_slot);
  screen_debug(temp);

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
    hosts_and_devices_eject(selected_device_slot);
    return SS_CHOOSE;
  case KCODE_ESCAPE:
    state = SELECT_FILE;
    return SS_DONE;
  case KCODE_RETURN: // For Atari I think we need to ask for file mode after this, it's not in the main select_slot.c code.
    selected_device_slot = bar_get() - DEVICES_START_MOUNT_Y;
    // Ask for mode.
    screen_select_slot_mode();
    k = input_select_slot_mode(&mode);

    if (!k)
    {
      state = SELECT_FILE;
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
}

#endif
