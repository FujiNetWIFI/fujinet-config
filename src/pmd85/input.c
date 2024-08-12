#ifdef BUILD_PMD85
/** 
 * Input routines
 */

#include <conio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "input.h"
#include "cursor.h"
#include "globals.h"
#include "bar.h"
#include "screen.h"
#include "colors.h"
#include "../set_wifi.h"
#include "../die.h"
#include "../hosts_and_devices.h"
#include "../select_file.h"
#include "../select_slot.h"

#define KEY_EOL          0x0A
#define KEY_RETURN       KEY_EOL
#define KEY_SPACE        0x20
#define KEY_K0           0x1B
#define KEY_K1           0x80
#define KEY_K2           0x81
#define KEY_K3           0x82
#define KEY_K4           0x83
#define KEY_K5           0x84
#define KEY_K6           0x85
#define KEY_K7           0x86
#define KEY_K8           0x87
#define KEY_K9           0x88
#define KEY_K10          0x89
#define KEY_K11          0x8A
#define KEY_ESCAPE       KEY_K0
#define KEY_CD           0x07
#define KEY_TAB          KEY_CD
#define KEY_DELETE       0x7F
#define KEY_1            0x31
#define KEY_2            0x32
#define KEY_3            0x33
#define KEY_4            0x34
#define KEY_5            0x35
#define KEY_6            0x36
#define KEY_7            0x37
#define KEY_8            0x38
#define KEY_9            0x39
#define KEY_0            0x30
#define KEY_HOME         0x0B
#define KEY_END          0x0D
#define KEY_UP_ARROW     KEY_HOME
#define KEY_DOWN_ARROW   KEY_END
#define KEY_LEFT_ARROW   0x0C
#define KEY_RIGHT_ARROW  0x09
#define KEY_RCL          0x8D
#define KEY_CLR          0x8E

extern unsigned short entry_timer;
extern bool long_entry_displayed;
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
  unsigned char c = cgetc();
  if ((c>='a') && (c<='z')) c&=~32;
  return c;
}

static void input_clear_bottom(void)
{
}

void input_line(unsigned char x, unsigned char y, unsigned char o, char *c, unsigned char len, bool password)
{
  char i;
  char a;

  i = o; // index into array and y-coordinate
  // x += o;

  cputs("\x1Bq"); // reverse off
  textcolor(EDITLINE_COLOR);

  while(1)
  {
    cursor_pos(x + i, y); // update cursor position
    cursor(1);            // turn cursor on
    gotoxy(x + i, y);     // update text position

    a = input();
    switch (a)
    {
    case KEY_LEFT_ARROW:
    case KEY_DELETE:
      if (i>0)
      {
        c[--i] = '\0';
        gotoxy(x + i, y);
        cputc(' ');
      }
      break;
    case KEY_RIGHT_ARROW:
      break;
    case KEY_RETURN:
      c[i] = '\0';
      cursor(0); // turn off cursor
      return; // done
    default:
      if (i < len && a > 0x1F && a < 0x7F)
      {
        // hide cursor
        cursor(0);
        // draw character on cursor position
        if (password)
          cputc('*');
        else
          cputc(a);
        c[i++] = a;
      }
    break;
    }
  }
}

WSSubState input_set_wifi_select(void)
{
  unsigned char k=input();

  switch(k)
  {
    case KEY_RETURN:
      set_wifi_set_ssid(bar_get());
      return WS_PASSWORD;
    case 'H':
    case 'h':
      return WS_CUSTOM;
    case 'R':
    case 'r':
      return WS_SCAN;
    case KEY_ESCAPE:
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

void input_line_set_wifi_custom(char *c)
{
  input_line(SCR_X0, STATUS_BAR + 1, 0, c, 31, false); // should be 32, but we are given small buffer
}

void input_line_set_wifi_password(char *c)
{
  input_line(SCR_X0, STATUS_BAR + 1, 0, c, 40, true); // should be 64, but we do not handle so long text on screen ;-)
}

HDSubState input_hosts_and_devices_hosts(void)
{
  unsigned char k = input();

  switch (k)
  {
  case KEY_UP_ARROW:
  case KEY_LEFT_ARROW:
    bar_up();
    selected_host_slot = bar_get();
    return HD_HOSTS;
  case KEY_DOWN_ARROW:
  case KEY_RIGHT_ARROW:
    bar_down();
    selected_host_slot = bar_get();
    return HD_HOSTS;
  case KEY_1:
  case KEY_2:
  case KEY_3:
  case KEY_4:
  case KEY_5:
  case KEY_6:
  case KEY_7:
  case KEY_8:
    bar_jump(k-KEY_1);
    return HD_HOSTS;
  case KEY_TAB:
    bar_clear(false);
    return HD_DEVICES;
  case KEY_RETURN:
    selected_host_slot = bar_get();
    if (hostSlots[selected_host_slot][0] != 0)
    {
      strcpy((char *)selected_host_name, (char *)hostSlots[selected_host_slot]);
      state = SELECT_FILE;
      return HD_DONE;
    }
    else
      return HD_HOSTS;
  case KEY_K10:
    return HD_DONE;
  case 'C':
  case 'c':
    state = SHOW_INFO;
    return HD_DONE;
  case 'E':
  case 'e':
    bar_clear(false);
    hosts_and_devices_edit_host_slot(selected_host_slot);
    return HD_HOSTS;
  default:
    return HD_HOSTS;
  }
}

HDSubState input_hosts_and_devices_devices(void)
{
  unsigned char k=input();
  switch(k)
    {
    case KEY_UP_ARROW:
    case KEY_LEFT_ARROW:
      bar_up();
      selected_device_slot=bar_get();
      // hosts_and_devices_long_filename();
      return HD_DEVICES;
    case KEY_DOWN_ARROW:
    case KEY_RIGHT_ARROW:
      bar_down();
      selected_device_slot=bar_get();
      // hosts_and_devices_long_filename();
      return HD_DEVICES;
    // Memory Module 'M' -> '4'
    case 'M':
    case 'm':
        k = KEY_4;
        // fall throw
    // Tape 'T' -> '3'
    case 'T':
    case 't':
      if (k != KEY_4)
        k = KEY_3;
        // fall throw
    case KEY_1:
    case KEY_2:
    case KEY_3:
    case KEY_4:
      bar_jump(k-KEY_1);
      selected_device_slot=bar_get();
      //hosts_and_devices_long_filename();
      return HD_DEVICES;
    case KEY_TAB:
      bar_clear(false);
      return HD_HOSTS;
    case 'E':
    case 'e':
      hosts_and_devices_eject(bar_get());
      return HD_DEVICES;
    // TODO: R is already used by ROM Module
    case 'R':
    case 'r':
      selected_device_slot=bar_get();
      hosts_and_devices_devices_set_mode(MODE_READ);
      return HD_DEVICES;
      break;
    case 'W':
    case 'w':
      selected_device_slot=bar_get();
      hosts_and_devices_devices_set_mode(MODE_WRITE);
      return HD_DEVICES;
      break;
    // case KEY_CLEAR:
    //   return HD_CLEAR_ALL_DEVICES;
    case 'C':
    case 'c':
      state = SHOW_INFO;
      return HD_DONE;
    case KEY_K10:
      return HD_DONE;
    default:
      return HD_DEVICES;
    }
}

void input_line_hosts_and_devices_host_slot(unsigned char i, unsigned char o, char *c)
{
  // reverse pixels, set editline color
  screen_set_region(SCR_X0 + 1, SCR_Y0 + 1 + i, 39, 1);
  screen_fill_region(EDITLINE_PATTERN_ON);

  input_line(SCR_X0 + 3, SCR_Y0 + 1 + i, o, c, 31, false); // should be 32

  // reverse pixels, set list color
  screen_set_region(SCR_X0 + 1, SCR_Y0 + 1 + i, 39, 1);
  screen_fill_region(EDITLINE_PATTERN_OFF);
}

void input_line_filter(char *c)
{
  input_line(SCR_X0, STATUS_BAR + 1, 0, c, 31, false); // should be 32
}

SFSubState input_select_file_choose(void)
{
	unsigned entryType;
	unsigned char k = input();

	switch (k)
	{
	case KEY_UP_ARROW:
	case KEY_LEFT_ARROW:
		if ((bar_get() == 0) && (pos > 0))
			return SF_PREV_PAGE;
    long_entry_displayed = false;
    bar_up();
    select_display_long_filename();
    break;
	case KEY_DOWN_ARROW:
	case KEY_RIGHT_ARROW:
		if ((bar_get() == 14) && (dir_eof == false))
			return SF_NEXT_PAGE;
    long_entry_displayed = false;
    bar_down();
    select_display_long_filename();
    break;
	case KEY_RETURN:
		pos += bar_get();
		entryType = select_file_entry_type();
		if (entryType == ENTRY_TYPE_FOLDER)
			return SF_ADVANCE_FOLDER;
		else if (entryType == ENTRY_TYPE_LINK)
			return SF_LINK;
    // strncpy(source_path, path, 224); // makes mess :-(
    // old_pos = pos;
    return SF_DONE;
	case KEY_ESCAPE:
		copy_mode = false;
		state = HOSTS_AND_DEVICES;
		return SF_DONE;
	case KEY_RCL:
		return strcmp(path, "/") == 0 ? SF_CHOOSE : SF_DEVANCE_FOLDER;
	case 'F':
	case 'f':
		return SF_FILTER;
	// case 'N':
	// case 'n':
	// 	return SF_NEW;
	// case 'C':
	// case 'c':
	// 	pos += bar_get();
	// 	select_file_set_source_filename();
	// 	return SF_COPY;
	case ',':
	case '<':
		if (pos > 0)
			return SF_PREV_PAGE;
    break;
	case '.':
	case '>':
		if (dir_eof == false)
			return SF_NEXT_PAGE;
    break;
	}
  return SF_CHOOSE;
}

unsigned char input_select_file_new_type(void)
{
  return 0;
}

unsigned long input_select_file_new_size(unsigned char t)
{
  return 0;
}

unsigned long input_select_file_new_custom(void)
{
  return 0;
}

void input_select_file_new_name(char *c)
{
  input_line(SCR_X0, SCR_Y0 + 19, 0, c, 40, false); // should be 255(?), but we do not handle multiline strings yet
}

SSSubState input_select_slot_choose(void)
{
  unsigned char k;

  k=input();

  switch(k)
    {
    case KEY_ESCAPE:
      state = SELECT_FILE;
      backToFiles = true;
      // state = HOSTS_AND_DEVICES;
      return SS_DONE;
    case KEY_UP_ARROW:
    case KEY_LEFT_ARROW:
      bar_up();
      return SS_CHOOSE;
    case KEY_DOWN_ARROW:
    case KEY_RIGHT_ARROW:
      bar_down();
      return SS_CHOOSE;
    // Memory Module 'M' -> '4'
    case 'M':
    case 'm':
        k = KEY_4;
        // fall throw
    // Tape 'T' -> '3'
    case 'T':
    case 't':
      if (k != KEY_4)
        k = KEY_3;
        // fall throw
    case KEY_1:
    case KEY_2:
    case KEY_3:
    case KEY_4:
      bar_jump(k-KEY_1);
	  return SS_CHOOSE;
    // case KEY_SMART_IV:
    case 'E':
    case 'e':
      select_slot_eject(bar_get());
      return SS_CHOOSE;
    case 'R':
    case 'r':
    case KEY_RETURN:
      mode = MODE_READ;
      selected_device_slot=bar_get();
      return SS_DONE;
    case 'W':
    case 'w':
      mode = MODE_WRITE;
      selected_device_slot=bar_get();
      return SS_DONE;
    default:
      return SS_CHOOSE;
    }
}

unsigned char input_select_slot_mode(char *mode)
{
  // unsigned char k = 0;

  // while (k == 0)
  // {
  //   k = input_ucase();

  //   if (k == KEY_ESCAPE)
  //   {
  //     return 0;
  //   }

  //   switch(k)
  //   {
  //     case 'W':
  //     case 'w':
  //       mode[0] = 2;
  //       break;
  //     default:
  //       mode[0] = 1;
  //   }
  // }
  return 1;
}

DHSubState input_destination_host_slot_choose(void)
{
  return DH_CHOOSE;
}

SISubState input_show_info(void)
{
	char c;
	c = input();
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
		state = HOSTS_AND_DEVICES;
		return SI_DONE;
	}
	return SI_SHOWINFO;
}

#endif /* BUILD_PMD85 */
