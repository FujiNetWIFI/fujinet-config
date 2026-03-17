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
	unsigned char s;

	state = SET_WIFI;

	screen_connect_wifi(&nc);

	while (retries > 0)
	{
		// check for esc key and abort
#ifdef _CMOC_VERSION_
		if (inkey() == KEY_BREAK)
#else
		if (input() == KEY_ABORT)
#endif /* _CMOC_VERSION_ */
		{
			screen_error("CONNECTION ABORTED");
			pause(150);
			state=SET_WIFI;
			return;
		}
		
		fuji_get_wifi_status(&s);

		switch (s)
		{
		case 1:
			screen_error("NO SSID AVAILABLE");
			pause(150);
			return;
		case 3:
			fuji_get_ssid(&nc);
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
			screen_error("PLEASE WAIT...(ESC TO ABORT)");
 			pause(150);
			retries--;
			break;
		}
	}
	screen_error("UNABLE TO CONNECT");
	pause(150);
	state = SET_WIFI;
}
