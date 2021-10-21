/**
 * 05-scan
 */

#include <arch/z80.h>
#include <smartkeys.h>
#include "fuji_typedefs.h"
#include "state.h"

Context context;

char tmp[256];

State configured(Context *context)
{
  return SET_WIFI;
}

void main(void)
{
  smartkeys_set_mode();
  context.net_connected = false;
  context.state = configured(&context);

  state(&context);
}
