#ifdef _CMOC_VERSION_

/**
 * Wi-Fi setup and adapter info input.
 */

#include "coco_input.h"
#include "../input.h"
#include "../globals.h"
#include "../key_codes.h"
#include "../set_wifi.h"

void input_line_set_wifi_custom(char *c)
{
  input_line(MENU_X, PROMPT_FIELD_Y, 0, c, 32, false);
}

void input_line_set_wifi_password(char *c)
{
  input_line(MENU_X + 10, PROMPT_FIELD_Y, 0, c, 64, true);
}

static void redraw_networks(void)
{
  SSIDInfo s;
  byte i;
  byte row = (byte)bar_get();

  screen_clear();
  screen_set_wifi_extended(&adapterConfigExt);
  for (i = 0; i < numNetworks; i++)
  {
    fuji_get_scan_result(i, &s);
    screen_set_wifi_display_ssid(i, &s);
  }
  screen_set_wifi_select_network(numNetworks);
  if (numNetworks)
    bar_jump(row);
}

WSSubState input_set_wifi_select(void)
{
  char k = waitkey_joystick();

  switch (k)
  {
  case KEY_ENTER:
    if (numNetworks == 0)
      return WS_SELECT;
    set_wifi_set_ssid(bar_get());
    return WS_PASSWORD;
  case 'H':
  case 'h':
    return WS_CUSTOM;
  case 'R':
  case 'r':
    return WS_SCAN;
  case 'S':
  case 's':
    state = HOSTS_AND_DEVICES;
    return WS_DONE;
  case KEY_UP_ARROW:
    bar_up();
    break;
  case KEY_DOWN_ARROW:
    bar_down();
    break;
  case KEY_REDRAW:
    redraw_networks();
    break;
  default:
    return WS_SELECT;
  }

  return WS_SELECT;
}

/*
 *  'C' - Change SSID
 *  'R' - Reconnect Wifi
 *  Space - change colors
 *  Any other key - return to main hosts and devices screen
 */
SISubState input_show_info(void)
{
  char c = waitkey_joystick();

  switch (c)
  {
  case 'c':
  case 'C':
    state = SET_WIFI;
    return SI_DONE;
  case 'r':
  case 'R':
    state = CONNECT_WIFI;
    return SI_DONE;
  case KEY_REDRAW:
    screen_clear();
    state = SHOW_INFO;
    return SI_DONE;
  default:
    state = HOSTS_AND_DEVICES;
    return SI_DONE;
  }

  return SI_SHOWINFO;
}

#endif
