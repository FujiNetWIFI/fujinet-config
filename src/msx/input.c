/**
 * Input routines
 */

#include <conio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../input.h"
#include "../globals.h"
#include "../mount_and_boot.h"
#include "../screen.h"
#include "../set_wifi.h"
#include "../hosts_and_devices.h"
#include "../select_file.h"
#include "../select_slot.h"
#include "key_codes.h"
#include "cursor.h"

extern unsigned short entry_timer;
extern bool long_entry_displayed;
extern unsigned char copy_host_slot;
extern bool copy_mode;
extern bool screen_should_be_cleared;

/**
 * Get input from keyboard/joystick
 * @return keycode (or synthesized keycode if joystick)
 */
unsigned char input()
{
  char c = cgetc();
  // gotoxy(0,0); cprintf("%02X",c);
  return c;

  // return cgetc();
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
  // textcolor(EDITLINE_COLOR);

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
  // input_line(SCR_X0, STATUS_BAR + 1, 0, c, 31, false); // should be 32, but we are given small buffer
}

void input_line_set_wifi_password(char *c)
{
  // input_line(SCR_X0, STATUS_BAR + 1, 0, c, 40, true); // should be 64, but we do not handle so long text on screen ;-)
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
  case 'B':
  case 'b':
    screen_should_be_cleared = true;
    mount_and_boot();
    return HD_HOSTS;
  case KEY_TAB:
  case 'D':
  case 'd':
    bar_clear(false);
    return HD_DEVICES;
  case KEY_RETURN:
    selected_host_slot = bar_get();
    if (hostSlots[selected_host_slot][0] != 0)
    {
      screen_should_be_cleared = true;
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
    screen_should_be_cleared = true;
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
    case KEY_1:
    case KEY_2:
    case KEY_3:
    case KEY_4:
      bar_jump(k-KEY_1);
      selected_device_slot=bar_get();
      //hosts_and_devices_long_filename();
      return HD_DEVICES;
    case 'B':
    case 'b':
      mount_and_boot();
      return HD_DEVICES;
    case KEY_TAB:
    case 'H':
    case 'h':
      bar_clear(false);
      // gotoxy(0,0); cprintf("Hosts!");
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
      screen_should_be_cleared = true;
      state = SHOW_INFO;
      return HD_DONE;
    case KEY_K10:
      return HD_DONE;
    default:
      return HD_DEVICES;
    }
}

void input_line_hosts_and_devices_host_slot(uint_fast8_t i, uint_fast8_t o, char *c)
{
  // reverse pixels, set editline color
  // screen_set_region(SCR_X0 + 1, SCR_Y0 + 1 + i, 39, 1);
  // screen_fill_region(EDITLINE_PATTERN_ON);

  // input_line(SCR_X0 + 3, SCR_Y0 + 1 + i, o, c, 31, false); // should be 32

  // reverse pixels, set list color
  // screen_set_region(SCR_X0 + 1, SCR_Y0 + 1 + i, 39, 1);
  // screen_fill_region(EDITLINE_PATTERN_OFF);
}

void input_line_filter(char *c)
{
  // input_line(SCR_X0, STATUS_BAR + 1, 0, c, 31, false); // should be 32
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
  // input_line(SCR_X0, SCR_Y0 + 19, 0, c, 40, false); // should be 255(?), but we do not handle multiline strings yet
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
	  screen_should_be_cleared = true;
		state = SET_WIFI;
		return SI_DONE;
	case 'r':
	case 'R':
	  screen_should_be_cleared = true;
		state = CONNECT_WIFI;
		return SI_DONE;
	default:
	  screen_should_be_cleared = true;
		state = HOSTS_AND_DEVICES;
		return SI_DONE;
	}
	return SI_SHOWINFO;
}
