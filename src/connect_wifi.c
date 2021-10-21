/**
 * #FUJINET CONFIG
 * Connect Wifi Network
 */

#include "connect_wifi.h"
#include "fuji_typedefs.h"

/**
 * Setup
 */
void connect_wifi_setup(char* ssid)
{
}

/**
 * Wait for network connection
 */
State connect_wifi_wait_for_network(NetConfig* n, Context *context)
{
  State new_state = SET_WIFI;
  unsigned char wifiStatus=0;
  unsigned char retries=0;
  
  return new_state;
}

/**
 * Connect wifi State
 */
State connect_wifi(Context *context)
{
  NetConfig n;
  
  return connect_wifi_wait_for_network(&n,context);
}
