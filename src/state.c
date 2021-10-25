/**
 * #FUJINET Configuration
 *
 * State Machine
 */

#include <stdbool.h>
#include <string.h>
#include "state.h"
#include "set_wifi.h"
#include "connect_wifi.h"
#include "diskulator_hosts.h"
#include "diskulator_info.h"

/**
 * Set up initial context
 */
void context_setup(Context* context)
{
  memset(context,0,sizeof(Context));
  context->net_connected = false;
  context->copySubState = DISABLED;
}

/**
 * Common code for every state
 */
void state_common(void)
{
}

/**
 * State machine. (Never exits.)
 */
void state(Context *context)
{
  while(true)
    {
      state_common();
      
      switch(context->state)
	{
	case SET_WIFI:
	  context->state = set_wifi(context);
	  break;
	case CONNECT_WIFI:
	  context->state = connect_wifi(context);
	  break;
	case DISKULATOR_HOSTS:
	  context->state = diskulator_hosts(context);
	  break;
	/* case DISKULATOR_SELECT: */
	/*   context->state = diskulator_select(context); */
	/*   break; */
	/* case DISKULATOR_SLOT: */
	/*   context->state = diskulator_slot(context); */
	/*   break; */
	case DISKULATOR_INFO:
	  context->state = diskulator_info();
	  break;
	/* case DISKULATOR_COPY: */
	/*   context->state = diskulator_copy(context); */
	/*   break; */
	/* case MOUNT_AND_BOOT: */
	/*   context->state = mount_and_boot(context); */
	/*   break; */
	}
    }
}
