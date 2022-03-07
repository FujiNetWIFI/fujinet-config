#ifdef BUILD_APPLE2
/** 
 * Input routines
 */

#include <conio.h>
#include <stdbool.h>
#include "input.h"
#include "../set_wifi.h"
#include "bar.h"
#include "screen.h"


#define KEY_RETURN       0x0D
#define KEY_ESCAPE       0x1B
#define KEY_SPACE        0x20
#define KEY_1            0x31
#define KEY_2            0x32
#define KEY_3            0x33
#define KEY_4            0x34
#define KEY_5            0x35
#define KEY_6            0x36
#define KEY_7            0x37
#define KEY_8            0x38
#define KEY_DELETE       127
#define KEY_UP_ARROW     11
#define KEY_DOWN_ARROW   10
#define KEY_LEFT_ARROW   8
#define KEY_RIGHT_ARROW  21

#define STATUS_BAR 21 // defined in screen.c

extern unsigned char copy_host_slot;
extern bool copy_mode;

/**
 * Get input from keyboard/joystick
 * @return keycode (or synthesized keycode if joystick)
 */
unsigned char input()
{
  return cgetc();
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

/**
 * Get line of input into c
 * @param x X position
 * @param y Y position
 * @param o start at character offset
 * @param c target buffer
 * @param l Length
 * @param password echoes characters.
 */
void input_line(unsigned char x, unsigned char y, unsigned char o, char *c, unsigned char len, bool password)
{
  char i = 0; // index into array and y-coordinate
  char a;
  gotoxy(x,y);
  cursor(1); // turn on cursor - does not have effect on Apple IIc
  while(1)
  {
    a = cgetc();
    switch (a)
    {
    case KEY_LEFT_ARROW:
    case KEY_DELETE:
      if (i>0)
      {
        c[--i + o] = 0;
        gotox(x + i);
        cputc(' ');
        gotox(x + i);
      }
      break;
    case KEY_RETURN:
      c[o + i] = 0;
      cursor(0); // turn off cursor
      return; // done
      break;
    default:
      if (i < len)
      {
        gotox(x + i);
        screen_putlcc(a);
        c[o + i++] = a;
      }
    break;
    }
  }
}

DHSubState input_destination_host_slot_choose(void) 
{
    unsigned char k=input();

  switch(k)
    {
    case KEY_RETURN:
      if (hostSlots[bar_get()][0] != 0x00)
	{
	  copy_host_slot=bar_get();
	  copy_mode=true;
	  state=SELECT_FILE;
	  return DH_DONE;
	}
      else
	return DH_CHOOSE;
    case KEY_ESCAPE:
      state=HOSTS_AND_DEVICES;
      return DH_ABORT;
    case KEY_1:
    case KEY_2:
    case KEY_3:
    case KEY_4:
    case KEY_5:
    case KEY_6:
    case KEY_7:
    case KEY_8:
      bar_jump(k-0x31);
      return DH_CHOOSE;
    case KEY_UP_ARROW:
      bar_up();
      return DH_CHOOSE;
    case KEY_DOWN_ARROW:
      bar_down();
      return DH_CHOOSE;
    default:
      return DH_CHOOSE;
    }
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
  char c;
  c =cgetc();
  switch (c)
  {
  case 'c':
  case 'C':
    state = SET_WIFI;
    return SI_DONE;
  case 'r':
  case 'R':
    state = CONNECT_WIFI;
    return SI_DONE;
  default:
    break;
  }
  return SI_SHOWINFO;
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
  input_line(2, STATUS_BAR + 1, 0, c, 32, false);
}

void input_line_set_wifi_password(char *c)
{
  input_line(2, STATUS_BAR + 1, 0, c, 64, false);
}

WSSubState input_set_wifi_select(void)
{
  // from adam
  unsigned char k = input();
  switch(k)
    {
    case KEY_RETURN:
      set_wifi_set_ssid(bar_get());
      return WS_PASSWORD;
    case 0x84:
      return WS_CUSTOM;
    case 'R':
    case 'r':
      return WS_SCAN;
    case 'S':
    case 's':
      state=HOSTS_AND_DEVICES;
      return WS_DONE;
    case KEY_UP_ARROW: // up arrow
    case KEY_LEFT_ARROW:
      bar_up();
      return WS_SELECT;
    case KEY_DOWN_ARROW: // down arrow
    case KEY_RIGHT_ARROW:
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
