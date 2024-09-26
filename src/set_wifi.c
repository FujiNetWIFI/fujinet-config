/**
 * FujiNet for #Adam configuration program
 *
 * Set new WiFi connection
 */

#ifdef _CMOC_VERSION_
#include <cmoc.h>
#include "coco/io.h"
#include "coco/screen.h"
#include "coco/bar.h"
#include "coco/input.h"
#include "coco/globals.h"
#else
#include <string.h>
#endif /* _CMOC_VERSION_ */

#include "set_wifi.h"
#include "die.h"
#include "fuji_typedefs.h"

#ifdef BUILD_ADAM
#include "adam/io.h"
#include "adam/screen.h"
#include "adam/bar.h"
#include "adam/input.h"
#include "adam/globals.h"
#endif /* BUILD_ADAM */

#ifdef BUILD_APPLE2
#include "apple2/io.h"
#include "apple2/screen.h"
#include "apple2/bar.h"
#include "apple2/input.h"
#include "apple2/globals.h"
#endif /* BUILD_APPLE2 */

#ifdef BUILD_ATARI
#include "atari/io.h"
#include "atari/screen.h"
#include "atari/bar.h"
#include "atari/input.h"
#include "atari/globals.h"
#endif /* BUILD_ATARI */

#ifdef BUILD_C64
#include "c64/io.h"
#include "c64/screen.h"
#include "c64/bar.h"
#include "c64/input.h"
#include "c64/globals.h"
#endif /* BUILD_APPLE2 */

#ifdef BUILD_PC8801
#include "pc8801/io.h"
#include "pc8801/screen.h"
#include "pc8801/bar.h"
#include "pc8801/input.h"
#include "pc8801/globals.h"
#endif /* BUILD_PC8801 */

#ifdef BUILD_PC6001
#include "pc6001/io.h"
#include "pc6001/screen.h"
#include "pc6001/bar.h"
#include "pc6001/input.h"
#include "pc6001/globals.h"
#endif /* BUILD_PC6001 */

#ifdef BUILD_PMD85
#include "pmd85/io.h"
#include "pmd85/screen.h"
#include "pmd85/bar.h"
#include "pmd85/input.h"
#include "pmd85/globals.h"
#endif /* BUILD_PMD85 */

#ifdef BUILD_RC2014
#include "rc2014/io.h"
#include "rc2014/screen.h"
#include "rc2014/bar.h"
#include "rc2014/input.h"
#include "rc2014/globals.h"
#endif /* BUILD_RC2014 */

WSSubState ws_subState;

NetConfig nc;

unsigned char numNetworks;

#ifdef _CMOC_VERSION_
void set_wifi_set_ssid(int i)
#else
void set_wifi_set_ssid(unsigned char i)
#endif
{
  SSIDInfo *s = io_get_scan_result(i);
  memcpy(nc.ssid,s->ssid,32);
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
  screen_set_wifi(io_get_adapter_config());


  numNetworks = io_scan_for_networks();

  if (numNetworks > 16)
    numNetworks = 16;

  if (io_error())
    {
      screen_error("COULD NOT WS_SCAN NETWORKS");
      die(); // to do retry or something instead
    }

  for (i=0;i<numNetworks;i++)
    {
      screen_set_wifi_display_ssid(i,io_get_scan_result(i));
    }

  ws_subState=WS_SELECT;
}

void set_wifi_done(void)
{
  int result = io_set_ssid(&nc);
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
