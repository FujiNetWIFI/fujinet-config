#ifdef BUILD_C64
/** 
 * Input routines
 */

#include <conio.h>
#include <stdbool.h>
#include "input.h"

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

void input_line_set_wifi_custom(char *c)
{
}

void input_line_set_wifi_password(char *c)
{
}

void input_line_hosts_and_devices_host_slot(unsigned char i, unsigned char o, char *c)
{
}

void input_line_filter(char *c)
{
}
#endif /* BUILD_C64 */
