#ifdef BUILD_APPLE2
/** 
 * Input routines
 */

#include <conio.h>
#include <stdbool.h>
#include "input.h"
#include "../set_wifi.h"
#include "bar.h"

/**
 * Get input from keyboard/joystick
 * @return keycode (or synthesized keycode if joystick)
 */
unsigned char input()
{
  return 0;
}

unsigned char input_ucase(void)
{
  unsigned char c = input();
  if ((c>='a') && (c<='z')) c&=~32;
  return c;
}

static void input_clear_bottom(void)
{
}

void input_line(unsigned char x, unsigned char y, unsigned char o, char *c, unsigned char len, bool password)
{
}

DHSubState input_destination_host_slot_choose(void) 
{
  // TODO: Implement
}

SFSubState input_select_file_choose(void) 
{
  // TODO: implement
}

unsigned char input_select_file_new_type(void) 
{
  // TODO: Implement
}

unsigned long input_select_file_new_size(unsigned char t) 
{
  // TODO: implement
}

unsigned long input_select_file_new_custom(void) 
{
  // TODO: implement
}

void input_select_file_new_name(char *c) 
{
  // TODO: Implement
}

SSSubState input_select_slot_choose(void)
{
  // TODO: implement
}

SISubState input_show_info(void)
{
  // TODO: implement
}

HDSubState input_hosts_and_devices_hosts(void)
{
  // TODO: implement
}

HDSubState input_hosts_and_devices_devices(void)
{
  // TODO: Implement
}

void input_line_set_wifi_custom(char *c)
{
}

void input_line_set_wifi_password(char *c)
{
}

WSSubState input_set_wifi_select(void)
{
  // from adam
  unsigned char k = cgetc();
  switch(k)
    {
    case 0x0D:
      set_wifi_set_ssid(bar_get());
      return WS_PASSWORD;
    case 0x84:
      return WS_CUSTOM;
    case 'R':
      return WS_SCAN;
    case 'S':
      state=HOSTS_AND_DEVICES;
      return WS_DONE;
    case 11: // up arrow
      bar_up();
      return WS_SELECT;
    case 10: // down arrow
      bar_down();
      return WS_SELECT;
    default:
      return WS_SELECT;
    }
}

void input_line_hosts_and_devices_host_slot(unsigned char i, unsigned char o, char *c)
{
}

void input_line_filter(char *c)
{
}
#endif /* BUILD_APPLE2 */
