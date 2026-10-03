#ifdef _CMOC_VERSION_

/**
 * Wi-Fi setup, connect and adapter info screens.
 */

#include "coco_screen.h"
#include "../globals.h"
#include "../system.h"

extern NetConfig nc;

static void center_puts(byte y, const char *s)
{
  screen_puts((SCREEN_COLS - (byte)strlen(s)) / 2, y, s);
}

static void clear_scanning(void)
{
  screen_puts(LIST_X, NETWORKS_Y + MAX_WIFI_NETWORKS / 2, "                                      ");
}

void screen_set_wifi_extended(AdapterConfigExtended *ac)
{
  char s[24];

  screen_frame("FujiNet Config");
  screen_box(0, 1, SCREEN_COLS, 4, NULL);
  center_puts(2, "Welcome to FujiNet");
  sprintf(s, "MAC: %02X:%02X:%02X:%02X:%02X:%02X", ac->macAddress[0], ac->macAddress[1], ac->macAddress[2],
          ac->macAddress[3], ac->macAddress[4], ac->macAddress[5]);
  center_puts(3, s);
  screen_box(0, NETWORKS_BOX_Y, SCREEN_COLS, MAX_WIFI_NETWORKS + 3, "AVAILABLE NETWORKS");
  center_puts(NETWORKS_Y + MAX_WIFI_NETWORKS / 2, "Scanning for networks...");
  screen_message_target(NETWORKS_Y + MAX_WIFI_NETWORKS / 2, true, 120);
}

void screen_set_wifi_display_ssid(char n, SSIDInfo *s)
{
  char meter[4] = "   ";
  char ds[LIST_W + 1];

  if (n == 0)
    clear_scanning();

  memset(ds, ' ', LIST_W);
  ds[LIST_W] = 0;
  strncpy(ds, s->ssid, 32);
  if (s->rssi > -50)
    strcpy(meter, "***");
  else if (s->rssi > -70)
    strcpy(meter, "** ");
  else
    strcpy(meter, "*  ");

  screen_puts(LIST_X, NETWORKS_Y + n, ds);
  screen_puts(LIST_X + LIST_W - 4, NETWORKS_Y + n, meter);
}

void screen_set_wifi_select_network(unsigned char nn)
{
  screen_menu_clear();
  if (nn)
  {
    screen_move(MENU_X, MENU_Y1);
    screen_print_menu("UP/DOWN", " Move  ");
    screen_print_menu("ENTER", " Select");
    screen_move(MENU_X, MENU_Y2);
  }
  else
  {
    screen_move(MENU_X, MENU_Y1);
  }
  screen_print_menu("S", "kip  ");
  screen_print_menu("H", "idden SSID  ");
  screen_print_menu("R", "escan");
  if (nn)
  {
    bar_set(NETWORKS_Y, 1, nn, 0);
  }
  else
  {
    clear_scanning();
    center_puts(NETWORKS_Y + MAX_WIFI_NETWORKS / 2 - 1, "No networks found.");
    center_puts(NETWORKS_Y + MAX_WIFI_NETWORKS / 2 + 1, "Rescan, or enter a hidden SSID.");
  }
}

void screen_set_wifi_custom(void)
{
  screen_frame("FujiNet Config");
  screen_box(0, PROMPT_BOX_Y, SCREEN_COLS, 7, "HIDDEN NETWORK");
  screen_puts(MENU_X, PROMPT_BOX_Y + 2, "Network name:");
  screen_move(MENU_X, MENU_Y1);
  screen_print_menu("ENTER", " Done");
}

void screen_set_wifi_password(void)
{
  screen_frame("FujiNet Config");
  screen_box(0, PROMPT_BOX_Y, SCREEN_COLS, 7, "NETWORK PASSWORD");
  screen_puts(MENU_X, PROMPT_BOX_Y + 2, "Network:");
  screen_puts(MENU_X + 10, PROMPT_BOX_Y + 2, nc.ssid);
  screen_puts(MENU_X, PROMPT_FIELD_Y, "Password:");
  screen_move(MENU_X, MENU_Y1);
  screen_print_menu("ENTER", " Connect");
}

void screen_connect_wifi(NetConfig *nc)
{
  screen_frame("FujiNet Config");
  screen_box(0, PROMPT_BOX_Y - 1, SCREEN_COLS, 9, "CONNECTING");
  center_puts(PROMPT_BOX_Y + 1, "Connecting to network:");
  center_puts(PROMPT_BOX_Y + 3, nc->ssid);
  screen_message_target(PROMPT_BOX_Y + 5, true, 0);
  screen_move(MENU_X, MENU_Y1);
  screen_print_menu("BREAK", " Abort");
}

#define INFO_ROWS    9
#define INFO_LABEL_W 9
#define INFO_VALUE_W 28

void screen_show_info_extended(bool printerEnabled, AdapterConfigExtended *ac)
{
  static const char *labels[INFO_ROWS] =
    { "SSID:", "Hostname:", "IP:", "Netmask:", "DNS:", "MAC:", "BSSID:", "FN Ver:", "Config:" };
  static char vals[INFO_ROWS][INFO_VALUE_W + 1];
  byte i, w, maxw = 0, x0;

  strncpy(vals[0], ac->ssid, INFO_VALUE_W);
  strncpy(vals[1], ac->hostname, INFO_VALUE_W);
  sprintf(vals[2], "%u.%u.%u.%u", ac->localIP[0], ac->localIP[1], ac->localIP[2], ac->localIP[3]);
  sprintf(vals[3], "%u.%u.%u.%u", ac->netmask[0], ac->netmask[1], ac->netmask[2], ac->netmask[3]);
  sprintf(vals[4], "%u.%u.%u.%u", ac->dnsIP[0], ac->dnsIP[1], ac->dnsIP[2], ac->dnsIP[3]);
  sprintf(vals[5], "%02X:%02X:%02X:%02X:%02X:%02X", ac->macAddress[0], ac->macAddress[1], ac->macAddress[2],
          ac->macAddress[3], ac->macAddress[4], ac->macAddress[5]);
  sprintf(vals[6], "%02X:%02X:%02X:%02X:%02X:%02X", ac->bssid[0], ac->bssid[1], ac->bssid[2],
          ac->bssid[3], ac->bssid[4], ac->bssid[5]);
  strncpy(vals[7], ac->fn_version, INFO_VALUE_W);
  strncpy(vals[8], GIT_VERSION, INFO_VALUE_W);

  for (i = 0; i < INFO_ROWS; i++)
  {
    vals[i][INFO_VALUE_W] = 0;
    w = (byte)strlen(vals[i]);
    if (w > maxw)
      maxw = w;
  }
  x0 = 1 + (SCREEN_COLS - 2 - (INFO_LABEL_W + 1 + maxw)) / 2;

  screen_frame("FujiNet Config");
  screen_box(0, 4, SCREEN_COLS, INFO_ROWS + 4, "FUJINET CONFIGURATION");
  for (i = 0; i < INFO_ROWS; i++)
  {
    screen_puts(x0 + INFO_LABEL_W - (byte)strlen(labels[i]), 6 + i, labels[i]);
    screen_puts(x0 + INFO_LABEL_W + 1, 6 + i, vals[i]);
  }

  screen_move((SCREEN_COLS - 22) / 2, MENU_Y1);
  screen_print_menu("C", "hange SSID  ");
  screen_print_menu("R", "econnect");
  screen_puts((SCREEN_COLS - 38) / 2, MENU_Y2, "Any other key returns to hosts/devices");
}

#endif
