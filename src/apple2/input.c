#ifdef BUILD_APPLE2
/**
 * Input routines
 */

#ifdef __ORCAC__
#include <coniogs.h>
#include <apple2gs.h>
#else
#include <conio.h>
#include <apple2.h>
#include <peekpoke.h>
#endif
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "input.h"
#include "bar.h"
#include "screen.h"
#include "mount_and_boot.h"

#include "../set_wifi.h"
#include "../die.h"
#include "../hosts_and_devices.h"
#include "../select_file.h"
#include "../select_slot.h"

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
#define KEY_DELETE       127
#define KEY_UP_ARROW     11
#define KEY_DOWN_ARROW   10
#define KEY_LEFT_ARROW   8
#define KEY_RIGHT_ARROW  21

#define STATUS_BAR 21 // defined in screen.c

#define UNUSED(x) (void)(x);

extern unsigned char copy_host_slot;
extern bool copy_mode;
extern bool long_entry_displayed;

extern unsigned char io_create_type;

/**
 * Get input from keyboard/joystick
 * @return keycode (or synthesized keycode if joystick)
 */
unsigned char input(void)
{
  return cgetc();
}

unsigned char input_ucase(void)
{
  unsigned char c = input();
  if ((c>='a') && (c<='z')) c&=~32;
  return c;
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
  ostype = get_ostype() & 0xF0;

  i = o; // index into array and y-coordinate
  // x += o;

  gotoy(y);

  while(1)
  {
    gotox(x + i);
    cputc('_');
    gotox(x + i);
    a = cgetc();
    if (ostype == APPLE_IIIEM)   // check for Apple3 lowercase
      if (!(PEEK(0xc008)&0x02) && (PEEK(0xc008)&0x08)) // test for shift or alpha lock key not pressed
        if ((a > 63) && (a < 96))
          a += 32;
    switch (a)
    {
    case KEY_ESCAPE:
      if (ostype == APPLE_II)
      {
        if (uc == 0)
          uc = 32;
        else
          uc = 0;
      }
      break;
    case KEY_LEFT_ARROW:
    case KEY_DELETE:
      if (i>0)
      {
        c[--i] = 0;
        cputc(' ');
        gotox(x + i);
        cputc(' ');
        gotox(x + i);
      }
      break;
    case KEY_RIGHT_ARROW:
      break;
    case KEY_RETURN:
      cputc(' ');
      c[i] = 0;
      return; // done
      break;
    default:
      if (i < len)
      {
        if (a>64 && a<91)
          a += uc;
        gotox(x + i);
	if (password)
	  screen_putlcc('*');
	else
	  screen_putlcc(a);
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
      strcpy((char *)selected_host_name, (char *)hostSlots[selected_host_slot]);
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
      bar_jump(k-KEY_1);
      return DH_CHOOSE;
    case KEY_UP_ARROW:
    case 'i':
    case 'I':
      bar_up();
      return DH_CHOOSE;
    case KEY_DOWN_ARROW:
    case 'm':
    case 'M':
      bar_down();
      return DH_CHOOSE;
    default:
      return DH_CHOOSE;
    }
}

SFSubState input_select_file_choose(void)
{
  unsigned entryType;
  unsigned char k = cgetc();

  switch (k)
  {
  case KEY_RIGHT_ARROW:
  case 'k':
  case 'K':
  case KEY_RETURN:
    pos += bar_get();
    entryType = select_file_entry_type();
    if (entryType == ENTRY_TYPE_FOLDER)
      return SF_ADVANCE_FOLDER;
    else if (entryType == ENTRY_TYPE_LINK)
      return SF_LINK;
    else
    {
      strncpy(source_path, path, 224);
      old_pos = pos;
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
  case 'N':
  case 'n':
    return SF_NEW;
  case 'C':
  case 'c':
    if (copy_mode == true)
    {
      return SF_DONE;
    }
    else
    {
      pos += bar_get();
      select_file_set_source_filename();
      copy_host_slot = selected_host_slot;
      return SF_COPY;
    }
  case KEY_UP_ARROW:
  case 'i':
  case 'I':
    if ((bar_get() == 0) && (pos > 0))
      return SF_PREV_PAGE;
    else
    {
      long_entry_displayed = false;
      bar_up();
      select_display_long_filename();
      return SF_CHOOSE;
    }
  case KEY_LEFT_ARROW:
  case 'j':
  case 'J':
    if ( strlen(path) == 1 && pos <= 0 ) // We're at the root of the filesystem, and we're on the first page - go back to hosts/devices screen.
    {
      state = HOSTS_AND_DEVICES;
      return SF_DONE;
    }
    if (pos > 0)
      return SF_PREV_PAGE;
    else
      return SF_DEVANCE_FOLDER;
    return SF_CHOOSE;
  case KEY_DOWN_ARROW:
  case 'm':
  case 'M':
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
  switch (cgetc())
    {
    case 'P':
    case 'p':
      io_create_type=0;
      return 1;
    case '2':
      io_create_type=1;
      return 2;
    default:
      io_create_type=0;
      return 0;
    }
}

unsigned long input_select_file_new_size(unsigned char t)
{
  UNUSED(t); // Type not used.

  switch (cgetc())
    {
    case '1':
      return 280;
    case '8':
      return 1600;
    case '3':
      return 65535UL;
    case 'C':
    case 'c':
      return 1; // CUSTOM
    }
}

unsigned long input_select_file_new_custom(void)
{
  char c[12];
  input_line(0,22,0,c,32,false);
  return atol(c);
}

void input_select_file_new_name(char *c)
{
  input_line(0,22,0,c,255,false);
}

SSSubState input_select_slot_choose(void)
{
  // cprintf(" [1-4] SELECT SLOT\r\n [RETURN] INSERT INTO SLOT\r\n [ESC] TO ABORT.");
  unsigned char k;

  k=cgetc();

  switch(k)
    {
    case KEY_ESCAPE:
      state = SELECT_FILE;
      backToFiles = true;
      return SS_DONE;
    case KEY_1:
    case KEY_2:
    case KEY_3:
    case KEY_4:
    case KEY_5:
    case KEY_6:
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
      selected_device_slot=bar_get();
      mode = MODE_READ;
      return SS_DONE;
      // Ask for mode.
      screen_select_slot_mode();
      k = input_select_slot_mode(&mode);

      if (!k)
      {
        state = SELECT_FILE;
        backToFiles = true;
      }
      return SS_DONE;
    case 'W':
    case 'w':
      selected_device_slot=bar_get();
      mode = MODE_WRITE;
      return SS_DONE;
    case KEY_UP_ARROW:
    case 'i':
    case 'I':
    case KEY_LEFT_ARROW:
    case 'j':
    case 'J':
      bar_up();
      return SS_CHOOSE;
    case KEY_DOWN_ARROW:
    case 'm':
    case 'M':
    case KEY_RIGHT_ARROW:
    case 'k':
    case 'K':
      bar_down();
      return SS_CHOOSE;
    default:
      return SS_CHOOSE;
    }
}

unsigned char input_select_slot_mode(char *mode)
{
  unsigned char k = 0;

  while (k == 0)
  {
    k = input_ucase();

    if (k == KEY_ESCAPE)
    {
      return 0;
    }

    switch(k)
    {
      case 'W':
      case 'w':
        mode[0] = 2;
        break;
      default:
        mode[0] = 1;
    }
  }
  return 1;
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
    state = HOSTS_AND_DEVICES;
    return SI_DONE;
  }
  return SI_SHOWINFO;
}

HDSubState input_hosts_and_devices_hosts(void)
{
  unsigned char k=cgetc();

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
    bar_jump(k-KEY_1);
    return HD_HOSTS;
  case KEY_TAB:
  case 'T':
    bar_clear(false);
    // Switching view, reset selected
    selected_device_slot = 0;
    selected_host_slot = 0;
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
  case 'S':
  case 's':
    state = SHOW_DEVICES;
    return HD_DONE;
  case KEY_UP_ARROW:
  case 'i':
  case 'I':
  case KEY_LEFT_ARROW:
  case 'j':
  case 'J':
    bar_up();
    selected_host_slot = bar_get();
    return HD_HOSTS;
  case 'l':
  case 'L':
    mount_and_boot_lobby();
    return HD_HOSTS;
  case KEY_DOWN_ARROW:
  case 'm':
  case 'M':
  case KEY_RIGHT_ARROW:
  case 'k':
  case 'K':
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
    case KEY_5:
    case KEY_6:
      bar_jump(k-KEY_1);
      selected_device_slot=bar_get();
      hosts_and_devices_long_filename();
      return HD_DEVICES;
    case KEY_TAB:
    case 'T':
      bar_clear(false);
      // Switching view, reset selected
      selected_device_slot = 0;
      selected_host_slot = 0;
      return HD_HOSTS;
    case 'E':
    case 'e':
      hosts_and_devices_eject(bar_get());
      return HD_DEVICES;
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
    case KEY_UP_ARROW:
    case 'i':
    case 'I':
    case KEY_LEFT_ARROW:
    case 'j':
    case 'J':
      bar_up();
      selected_device_slot=bar_get();
      hosts_and_devices_long_filename();
      return HD_DEVICES;
    case 'l':
    case 'L':
      mount_and_boot_lobby();
      return HD_HOSTS;
    case KEY_DOWN_ARROW:
    case 'm':
    case 'M':
    case KEY_RIGHT_ARROW:
    case 'k':
    case 'K':
      bar_down();
      selected_device_slot=bar_get();
      hosts_and_devices_long_filename();
      return HD_DEVICES;
    case KEY_ESCAPE: // ESC
      return HD_DONE;
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
  input_line(2, STATUS_BAR + 1, 0, c, 64, true);
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
    case 'H':
    case 'h':
      return WS_CUSTOM;
    case 'R':
    case 'r':
      return WS_SCAN;
    case 'S':
    case 's':
      state=HOSTS_AND_DEVICES;
      return WS_DONE;
    case KEY_UP_ARROW:
    case 'i':
    case 'I':
    case KEY_LEFT_ARROW:
    case 'j':
    case 'J':
      bar_up();
      return WS_SELECT;
    case KEY_DOWN_ARROW:
    case 'm':
    case 'M':
    case KEY_RIGHT_ARROW:
    case 'k':
    case 'K':
      bar_down();
      return WS_SELECT;
    default:
      return WS_SELECT;
    }
}

void input_line_hosts_and_devices_host_slot(unsigned char i, unsigned char o, char *c)
{
    bar_clear(false);
    input_line(2,i+1,o,c,32,false);
}

void input_line_filter(char *c)
{
  input_line(0,23,0,c,32,false);
}
#endif /* BUILD_APPLE2 */
