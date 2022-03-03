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
#include <conio.h> // for dev
#include <stdint.h>
#endif /* BUILD_APPLE2 */

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
extern DeviceSlot deviceSlots[8];

void setup(void)
{
  io_init();
  screen_init();
}

void done(void)
{
}

void run(void)
{
  while (state != DONE)
    {
      switch(state)
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
	case DONE:
	  done();
	  break;
	}
    }
}

void test()
{
	uint8_t i, j;
	uint8_t err, num;

	clrscr();
	cputs("FujiNet Getting Started\n\r");

	err = sp_status(0x00, 0x00); // get number of devices
	num = sp_payload[0];
	num++;
	for (i = 1; i < num; i++)
	{
		cprintf("UNIT #%d NAME: ", i);
		err = sp_status(i, 0x03);
		for (j = 0; j < sp_payload[4]; j++)
			cputc(sp_payload[5 + j]);
		cputs("\r\n");
	}
}

void main(void)
{
	setup();
#ifdef BUILD_APPLE2
	test();
	cgetc();
#else
	run();
#endif

}
