/**
 * Connect to existing WiFi connection
 */
#include "connect_wifi.h"
#include "pause.h"
#include "typedefs.h"
#include "globals.h"
#include "compat.h"
#include "screen.h"
#include "input.h"

void connect_wifi(void)
{
	unsigned char retries = 20;
	NetConfig nc;
	unsigned char s;

        fuji_get_ssid(&nc);

	state = SET_WIFI;

	screen_connect_wifi(&nc);

	while (retries > 0)
	{
	  // TODO: Write COCO specific version that just checks for a key, as input() blocks! grrr. -thom
#ifndef _CMOC_VERSION_
	  // check for esc key and abort
		if (input() == KEY_ABORT)
		{
			screen_error("CONNECTION ABORTED");
			pause(150);
			state=SET_WIFI;
			return;
		}
#endif /* _CMOC_VERSION_ */
		
		fuji_get_wifi_status(&s);

		switch (s)
		{
		case 1:
			screen_error("NO SSID AVAILABLE");
			pause(150);
			return;
		case 3:
			screen_error("CONNECTION SUCCESS!");
			state = HOSTS_AND_DEVICES;
#ifdef BUILD_ADAM
                        screen_should_be_cleared=true;
#endif
			pause(60);
			return;
		case 4:
			screen_error("CONNECT FAILED");
			pause(150);
			return;
		case 5:
			screen_error("CONNECTION LOST");
			pause(150);
			return;
		default:
			screen_error("PLEASE WAIT...");
 			pause(150);
			retries--;
			break;
		}
	}
	screen_error("UNABLE TO CONNECT");
	pause(150);
	state = SET_WIFI;
}
