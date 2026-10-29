/**
 * Show adapter info
 */

#include "show_info.h"
#include "typedefs.h"
#include "screen.h"
#include "input.h"
#include "constants.h"
#include "globals.h"
#include "connect_wifi.h"
#include <string.h>

SISubState si_subState;

void show_info(void)
{
  si_subState=SI_SHOWINFO;
  
  fuji_get_adapter_config_extended(&adapterConfigExt);
  screen_show_info_extended(fuji_get_device_enabled_status(PRINTER), &adapterConfigExt);

  while (si_subState==SI_SHOWINFO)
    si_subState=input_show_info();

  // nc is not populated if wifi was already connected at startup, use SSID from adapter config
  if (state == CONNECT_WIFI)
    memcpy(nc.ssid, adapterConfigExt.ssid, SSID_MAXLEN);

  si_subState = SI_SHOWINFO;
}
