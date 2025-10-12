/**
 * Set new WiFi connection
 */

#include "set_wifi.h"
#include "die.h"
#include "screen.h"
#include "input.h"
#include "globals.h"
#include "compat.h"
#include "system.h"

WSSubState ws_subState;
NetConfig nc;
uint8_t numNetworks;
AdapterConfigExtended adapterConfigExt;
static SSIDInfo ssidInfo;

void set_wifi_set_ssid(uint_fast8_t i)
{
  fuji_get_scan_result((uint8_t) i, &ssidInfo);
  memcpy(nc.ssid, ssidInfo.ssid, 32);
}

void set_wifi_select(void)
{
  unsigned char k=0;
  
  screen_set_wifi_select_network(numNetworks);
  
  while(ws_subState==WS_SELECT)
    ws_subState=input_set_wifi_select();
}

void set_wifi_custom(void)
{
  screen_set_wifi_custom();
  input_line_set_wifi_custom(nc.ssid);
  ws_subState=WS_PASSWORD;
}

void set_wifi_password(void)
{
  screen_set_wifi_password();
  input_line_set_wifi_password(nc.password);
  ws_subState=WS_DONE;
}

void set_wifi_scan(void)
{
  char i;
  unsigned char valid_networks = 0;

  fuji_get_adapter_config_extended(&adapterConfigExt);
  screen_set_wifi_extended(&adapterConfigExt);

  fuji_scan_for_networks(&numNetworks);

  if (numNetworks > MAX_WIFI_NETWORKS)
	  numNetworks = MAX_WIFI_NETWORKS;

  if (fuji_error())
  {
	  screen_error("COULD NOT WS_SCAN NETWORKS");
	  die(); // to do retry or something instead
  }

  for (i = 0; i < numNetworks; i++)
  {
    fuji_get_scan_result(i, &ssidInfo);
    if (ssidInfo.ssid != NULL & strlen(ssidInfo.ssid) != 0)
    {
      screen_set_wifi_display_ssid(i, &ssidInfo);
      valid_networks++;
    }
    else
    {
      break;
    }
  }

  numNetworks = valid_networks;

  ws_subState = WS_SELECT;
}

void set_wifi_done(void)
{
#ifdef _CMOC_VERSION_  
  locate(0,14);
#endif
  int result = fuji_set_ssid(&nc);
  // always (good or bad) go to connect_wifi.
  // I had used result to only show this when we have good return, but
  // the connect_wifi shows error messages for us and jumps back to set wifi
  // anyway when there's an error, so it's the correct choice either way.
  state = CONNECT_WIFI;
}

void set_wifi(void)
{
  ws_subState = WS_SCAN;
  while (state == SET_WIFI)
  {
    switch (ws_subState)
    {
    case WS_SCAN:
      set_wifi_scan();
      break;
    case WS_SELECT:
      set_wifi_select();
      break;
    case WS_CUSTOM:
      set_wifi_custom();
      break;
    case WS_PASSWORD:
      set_wifi_password();
      break;
    case WS_DONE:
      set_wifi_done();
      break;
    }
  }
}
