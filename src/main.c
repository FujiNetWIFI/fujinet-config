/**
 * #FujiNet for ADAM Configuration Tool
 *
 */

#include <stdlib.h>
#include "typedefs.h"
#include "check_wifi.h"
#include "connect_wifi.h"
#include "set_wifi.h"
#include "hosts_and_devices.h"
#include "select_file.h"
#include "select_slot.h"
#include "perform_copy.h"
#include "show_info.h"
#include "destination_host_slot.h"

#ifdef BUILD_ADAM
#include "adam/io.h"
#include "adam/screen.h"
#endif /* BUILD_ADAM */

#ifdef BUILD_APPLE2
#include "apple2/io.h"
#include "apple2/screen.h"
#include "apple2/sp.h"
// #include <conio.h> // for dev
#include <stdint.h>
#endif /* BUILD_APPLE2 */

#ifdef BUILD_ATARI
#include "atari/io.h"
#include "atari/screen.h"
#endif /* BUILD_ATARI */

#ifdef BUILD_C64
#include "c64/io.h"
#include "c64/screen.h"
#endif /* BUILD_C64 */

#ifdef BUILD_PC8801
#include "pc8801/io.h"
#include "pc8801/screen.h"
#endif /* BUILD_PC8801 */

#ifdef BUILD_PC6001
#include "pc6001/io.h"
#include "pc6001/screen.h"
#endif /* BUILD_PC6001 */

State state=HOSTS_AND_DEVICES;

void setup(void)
{
  io_init();
  screen_init();
}

void done(void)
{
  // reboot here
  io_set_boot_config(0); // disable config
  io_boot();             // and reboot.
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
			io_list_devs();
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

void main(void)
{
	setup();
	state = CHECK_WIFI;
	run();
}

#ifdef BUILD_A2CDA
void Start(void)
{
	main();
}

void ShutDown(void)
{
}
#endif /* BUILD_A2CDA */
