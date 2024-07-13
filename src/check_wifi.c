/**
 * FujiNet for #Adam configuration program
 *
 * Check for Wifi Configuration and/or connection.
 */

#ifdef BUILD_ADAM
#include "adam/io.h"
#include "adam/globals.h"
#endif /* BUILD_ADAM */

#ifdef BUILD_APPLE2
#include "apple2/io.h"
#include "apple2/globals.h"
#endif /* BUILD_APPLE2 */

#ifdef BUILD_ATARI
#include "atari/io.h"
#include "atari/globals.h"
#endif /* BUILD_ATARI */

#ifdef BUILD_C64
#include "c64/io.h"
#include "c64/globals.h"
#endif /* BUILD_C64 */

#ifdef BUILD_PC8801
#include "pc8801/io.h"
#include "pc8801/globals.h"
#endif /* BUILD_PC8801 */

#ifdef BUILD_PC6001
#include "pc6001/io.h"
#include "pc6001/globals.h"
#endif /* BUILD_PC6001 */

#ifdef BUILD_PMD85
#include "pmd85/io.h"
#include "pmd85/globals.h"
#endif /* BUILD_PMD85 */

#ifdef BUILD_RC2014
#include "rc2014/io.h"
#include "rc2014/globals.h"
#endif /* BUILD_RC2014 */

#ifdef _CMOC_VERSION_
#include "coco/stdbool.h"
#include "coco/io.h"
#include "coco/globals.h"
#endif /* CMOC_VERSION */

#include "check_wifi.h"

void check_wifi(void)
{
  // Before checking actual connectivity, there's a command to check if wifi is completely disabled, check that first. If it's
  // disabled, don't go to the set/connect wifi screens.
  //
  if ( !io_get_wifi_enabled() )
  {
    state=HOSTS_AND_DEVICES;
  }
  else if (io_get_wifi_status() == 3)
    {
      state=HOSTS_AND_DEVICES;
    }
  else if (io_get_ssid()->ssid[0] == 0x00)
    {
      state=SET_WIFI;
    }
  else
    {
      state=CONNECT_WIFI;
    }
}
