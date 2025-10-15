/**
 * FujiNet for ADAM Configuration Tool
 *
 */

#include "check_wifi.h"
#include "connect_wifi.h"
#include "set_wifi.h"
#include "hosts_and_devices.h"
#include "select_file.h"
#include "select_slot.h"
#include "perform_copy.h"
#include "show_info.h"
#include "destination_host_slot.h"
#include "screen.h"
#include "typedefs.h"
#include "system.h"

State state;
bool backToFiles=false;
bool backFromCopy=false;

void setup(void)
{
  screen_init();
}

void done(void)
{
  // reboot here
  fuji_set_boot_config(0); // disable config
  screen_end();            // restore screen/keyboard state if needed
  system_boot();           // and reboot.
}

void run(void)
{
	while (true)
	{
		switch (state)
		{
		case CHECK_WIFI:
			check_wifi();
			break;
		case CONNECT_WIFI:
			connect_wifi();
			break;
		case SET_WIFI:
			set_wifi();
			break;
		case HOSTS_AND_DEVICES:
			hosts_and_devices();
			break;
		case SELECT_FILE:
			select_file();
			break;
		case SELECT_SLOT:
			select_slot();
			break;
		case DESTINATION_HOST_SLOT:
			destination_host_slot();
			break;
		case PERFORM_COPY:
			perform_copy();
			break;
		case SHOW_INFO:
			show_info();
			break;
		#ifdef BUILD_APPLE2
		case SHOW_DEVICES:
			system_list_devs();
			break;
		#endif
		case DONE:
			done();
			#ifdef BUILD_A2CDA
				return;
			#endif
			break;
		}
  }
}

int main(void)
{
	setup();
	state = CHECK_WIFI;
	run();
	return 0;
}
