/**
 * 05-scan
 */

#include <arch/z80.h>
#include <smartkeys.h>
#include "fuji_typedefs.h"
#include "state.h"
#include "fuji_adam.h"

Context context;

char tmp[256];

State configured(Context *context)
{
  NetConfig n;

  fuji_adamnet_get_ssid(&n);
  
  return n.ssid[0] == '\0' ? SET_WIFI : CONNECT_WIFI;
}

void main(void)
{
  context_setup(&context);
  smartkeys_set_mode();
  context.net_connected = false;
  context.state = configured(&context);

  state(&context);
}
