#ifdef BUILD_RC2014
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
#include "../set_wifi.h"
#include "../die.h"
#include "../hosts_and_devices.h"
#include "../select_file.h"
#include "../select_slot.h"
#include "../fuji_typedefs.h"
#include "console.h"
#include "io.h"

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
#define KEY_TAB          9
#define KEY_DELETE       0x08   // 127
#define KEY_UP_ARROW     5     // ctrl-e
#define KEY_DOWN_ARROW   24    // ctrl-x
#define KEY_LEFT_ARROW   19    // ctrl-s
#define KEY_RIGHT_ARROW  4     // ctrl-d

#define STATUS_BAR 21 // defined in screen.c

extern unsigned char copy_host_slot;
extern bool copy_mode;
extern bool long_entry_displayed;
/**
 * Get input from keyboard/joystick
 * @return keycode (or synthesized keycode if joystick)
 */
unsigned char input()
{
  return console_rx();// cgetc();
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
  char i;
  char a;
  char uc;
  char ostype;

  uc = 0;

  i = o; // index into array and y-coordinate
  // x += o;

  cursor_pos(0, y);
  cputc(':');
  while(1)
  {
    cursor_pos(x + i, y);
    cursor(1); // turn on cursor
    cursor_pos(x + i, y);
    a = input(); //cgetc();

    switch (a)
    {
    case KEY_ESCAPE:
      break;
    case KEY_LEFT_ARROW:
    case KEY_DELETE:
      if (i>0)
      {
        c[--i] = 0;
        cputc(' ');
        cursor_pos(x + i, y);
        cputc(' ');
        cursor_pos(x + i, y);
      }
      break;
    case KEY_RIGHT_ARROW:
      break;
    case KEY_RETURN:
      cputc('#');
      c[i] = 0;
      cursor(0); // turn off cursor
      return; // done
      break;
    default:
      if (i < len)
      {
        if (a>64 && a<91)
          a += uc;
        cursor_pos(x + i, y);
        cputc(a);
        c[i++] = a;
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
      selected_host_slot=bar_get();
      copy_mode=true;
      return DH_DONE;
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
  unsigned entryType;
  unsigned char k = input();

  switch (k)
  {
  case KEY_RETURN:
    pos += bar_get();
    entryType = select_file_entry_type();
    if (entryType == ENTRY_TYPE_FOLDER)
      return SF_ADVANCE_FOLDER;
    else if (entryType == ENTRY_TYPE_LINK)
      return SF_LINK;
    else
    {
      return SF_DONE;
    }
  case KEY_ESCAPE:
    copy_mode = false;
    state = HOSTS_AND_DEVICES;
    return SF_DONE;
  // case KEY_DELETE:
  //   pos = 0;
  //   dir_eof = quick_boot = false;
  //   return SF_DISPLAY;
  case KEY_DELETE:
    return strcmp(path, "/") == 0 ? SF_CHOOSE : SF_DEVANCE_FOLDER;
  case 'F':
  case 'f':
    return SF_FILTER;
  // case KEY_RETURN: // KEY_SMART_VI:
  //   if (copy_mode == false)
  //   {
  //     quick_boot = true;
  //     pos += bar_get();
  //     state = SELECT_SLOT; // should not change here? ... gets picked in SF_DONE state
  //   }
  //   return SF_DONE;
  // case KEY_INSERT:
  //   return SF_NEW;
  case 'C':
  case 'c':
    pos += bar_get();
    select_file_set_source_filename();
    return SF_COPY;
  case KEY_UP_ARROW:
  case KEY_LEFT_ARROW:
    if ((bar_get() == 0) && (pos > 0))
      return SF_PREV_PAGE;
    else
    {
      long_entry_displayed = false;
      bar_up();
      select_display_long_filename();
      return SF_CHOOSE;
    }
  case KEY_DOWN_ARROW:
  case KEY_RIGHT_ARROW:
    if ((bar_get() == 14) && (dir_eof == false))
      return SF_NEXT_PAGE;
    else
    {
      long_entry_displayed = false;
      bar_down();
      select_display_long_filename();
      return SF_CHOOSE;
    }
    break;
  case ',':
  case '<':
    if (pos > 0)
      return SF_PREV_PAGE;
  case '.':
  case '>':
    if (dir_eof == false)
      return SF_NEXT_PAGE;
  default:
    return SF_CHOOSE;
  }
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
  // cprintf(" [1-4] SELECT SLOT\r\n [RETURN] INSERT INTO SLOT\r\n [ESC] TO ABORT.");
   unsigned char k;

   k=input();

  switch(k)
  {
  case KEY_ESCAPE:
    state=HOSTS_AND_DEVICES;
    return SS_ABORT;
  case KEY_1:
  case KEY_2:
  case KEY_3:
  case KEY_4:
    bar_jump(k-0x31);
    return SS_CHOOSE;
  // case KEY_SMART_IV:
  case 'E':
  case 'e':
    select_slot_eject(bar_get());
    return SS_CHOOSE;
  case 'R':
  case 'r':
  case KEY_RETURN:
    selected_device_slot=bar_get();
    mode=0; // ?? read/write mode?
    return SS_DONE;
  case 'W':
  case 'w':
    selected_device_slot=bar_get();
    mode=2;
    return SS_DONE;
  case KEY_UP_ARROW:
  case KEY_LEFT_ARROW:
    bar_up();
    return SS_CHOOSE;
  case KEY_DOWN_ARROW:
  case KEY_RIGHT_ARROW:
    bar_down();
    return SS_CHOOSE;
  default:
    return SS_CHOOSE;
  }
}

SISubState input_show_info(void)
{
  char c;
  c =input();
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

HDSubState input_hosts_and_devices_hosts(void)
{
  unsigned char k=input();

  switch (k)
  {
  case KEY_1:
  case KEY_2:
  case KEY_3:
  case KEY_4:
  case KEY_5:
  case KEY_6:
  case KEY_7:
  case KEY_8:
    bar_jump(k - KEY_1);
    return HD_HOSTS;
  case KEY_TAB:
    bar_clear(false);
    return HD_DEVICES;
  case KEY_RETURN:
    selected_host_slot = bar_get();
    if (hostSlots[selected_host_slot][0] != 0)
    {
      strcpy(selected_host_name, hostSlots[selected_host_slot]);
      state = SELECT_FILE;
      return HD_DONE;
    }
    else
      return HD_HOSTS;
  case KEY_ESCAPE: // ESC
    return HD_DONE;
  case 'C':
  case 'c':
    state = SHOW_INFO;
    return HD_DONE;
  case 'E':
  case 'e':
    // smartkeys_sound_play(SOUND_POSITIVE_CHIME);
    hosts_and_devices_edit_host_slot(bar_get());
    bar_clear(true);
    bar_jump(selected_host_slot);
    k = 0;
    return HD_HOSTS;
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
  default:
    return HD_HOSTS;
  }
}

HDSubState input_hosts_and_devices_devices(void)
{
  unsigned char k=input();
  switch(k)
  {
  case KEY_1:
  case KEY_2:
  case KEY_3:
  case KEY_4:
    bar_jump(k-KEY_1);
    selected_device_slot=bar_get();
    hosts_and_devices_long_filename();
    return HD_DEVICES;
  case KEY_TAB:
    bar_clear(false);
    return HD_HOSTS;
  case 'E':
  case 'e':
    hosts_and_devices_eject(bar_get());
    return HD_DEVICES;
  case 'R':
  case 'r':
    selected_device_slot=bar_get();
    hosts_and_devices_devices_set_mode(0);
    return HD_DEVICES;
  case 'W':
  case 'w':
    selected_device_slot=bar_get();
    hosts_and_devices_devices_set_mode(2);
    return HD_DEVICES;
  case KEY_UP_ARROW:
  case KEY_LEFT_ARROW:
    bar_up();
    selected_device_slot=bar_get();
    hosts_and_devices_long_filename();
    return HD_DEVICES;
  case KEY_DOWN_ARROW:
  case KEY_RIGHT_ARROW:
    bar_down();
    selected_device_slot=bar_get();
    hosts_and_devices_long_filename();
    return HD_DEVICES;
  default:
    return HD_DEVICES;
  }
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
  bar_clear(false);
  input_line(4,i+1,o,c,32,false);
}

void input_line_filter(char *c)
{
  input_line(0,23,0,c,32,false);
}

#endif /* BUILD_RC2014 */
