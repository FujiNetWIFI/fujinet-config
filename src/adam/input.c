#ifdef BUILD_ADAM
/**
 * Input routines
 */

#include <video/tms99x8.h>
#include <eos.h>
#include <smartkeys.h>
#include <conio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "input.h"
#include "cursor.h"
#include "globals.h"
#include "bar.h"
#include "../set_wifi.h"
#include "../die.h"
#include "../hosts_and_devices.h"
#include "../select_file.h"
#include "../select_slot.h"
#include "io.h"

static GameControllerData cont;
static unsigned char key=0;
static unsigned char joystick=0;
static unsigned char joystick_copy=0;
static unsigned char button=0;
static unsigned char button_copy=0;
static unsigned char keypad=0;
static unsigned char keypad_copy=0;
static unsigned char repeat=0;

extern unsigned short entry_timer;
extern bool long_entry_displayed;
extern unsigned char copy_host_slot;
extern bool copy_mode;

/**
 * ADAM keyboard mapping
 */
#define KEY_BACKSPACE    0x08
#define KEY_TAB          0x09
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
#define KEY_HOME         0x80
#define KEY_SMART_I      0x81
#define KEY_SMART_II     0x82
#define KEY_SMART_III    0x83
#define KEY_SMART_IV     0x84
#define KEY_SMART_V      0x85
#define KEY_SMART_VI     0x86
#define KEY_WILD_CARD    0x90
#define KEY_UNDO         0x91
#define KEY_MOVE         0x9A
#define KEY_GET          0x93
#define KEY_INSERT       0x94
#define KEY_PRINT        0x95
#define KEY_CLEAR        0x96
#define KEY_DELETE       0x97
#define KEY_COPY         0x92
#define KEY_STORE        0x9B
#define KEY_S_INSERT     0x9C
#define KEY_S_PRINT      0x9D
#define KEY_S_CLEAR      0x9E
#define KEY_S_DELETE     0x9F
#define KEY_UP_ARROW     0xA0
#define KEY_DOWN_ARROW   0xA2
#define KEY_C_UP_ARROW   0xA4
#define KEY_C_DOWN_ARROW 0xA6

/**
 * Get input from keyboard/joystick
 * @return keycode (or synthesized keycode if joystick)
 */
unsigned char input()
{
  key = eos_end_read_keyboard();
  if (entry_timer>0)
    entry_timer--;

  if (key > 1)
    {
      eos_start_read_keyboard();
      if (key != 0x0D && key != 0x1B)
	smartkeys_sound_play(SOUND_KEY_PRESS);
      return key;
    }
  else
    {
      eos_read_game_controller(0x03,&cont);
      joystick = cont.joystick1 | cont.joystick2;
      button = cont.joystick1_button_left | cont.joystick1_button_right | cont.joystick2_button_left | cont.joystick2_button_right;
      keypad = cont.joystick1_keypad;

      if ((button > 0) && (button != button_copy))
	{
	  key=0x0d; // same as RETURN
	}
      else if ((keypad != 0x0F) && (keypad != keypad_copy))
	{
	  switch (keypad)
	    {
	    case 1: // Slot 1
	      key=KEY_1;
	      break;
	    case 2: // Slot 2
	      key=KEY_2;
	      break;
	    case 3: // Slot 3
	      key=KEY_3;
	      break;
	    case 4: // Slot 4
	      key=KEY_4;
	      break;
	    case 5: // Slot 5
	      key=KEY_5;
	      break;
	    case 6: // Slot 6
	      key=KEY_6;
	      break;
	    case 7: // Slot 7
	      key=KEY_7;
	      break;
	    case 8: // Slot 8
	      key=KEY_8;
	      break;
	    case 0x0a: // *
	      key=KEY_SMART_VI;
	      break;
	    }
	}
      else if ((joystick > 0) && (joystick == joystick_copy))
	repeat++;
      else
	repeat=0;

      if (repeat > 16)
	{
	  repeat=0;
	  switch(joystick)
	    {
	    case 1: // UP
	      key=KEY_UP_ARROW;
	      break;
	    case 4: // DOWN
	      key=KEY_DOWN_ARROW;
	      break;
	    }
	}

      joystick_copy = joystick;
      button_copy = button;
      keypad_copy = keypad;
    }

  return key;
}

unsigned char input_ucase(void)
{
  unsigned char c = input();
  if ((c>='a') && (c<='z')) c&=~32;
  return c;
}

static void input_clear_bottom(void)
{
  vdp_vfill(0x1200,0x00,768);
}

void input_line(unsigned char x, unsigned char y, unsigned char o, char *c, unsigned char len, bool password)
{
  unsigned char pos=o;

  c += o;
  x += o;

  cursor(true);
  input_clear_bottom();

  gotoxy(x,y);
  cursor_pos(x,y);

  while (key = eos_read_keyboard())
    {
      smartkeys_sound_play(SOUND_KEY_PRESS);
      if (key == KEY_RETURN)
	{
	  cursor(false);
	  break;
	}
      else if (key == KEY_BACKSPACE)
	{
	  if (pos > 0)
	    {
	      pos--;
	      x--;
	      c--;
	      *c=0x00;
	      putchar(KEY_BACKSPACE);
	      putchar(KEY_SPACE);
	      putchar(KEY_BACKSPACE);
	      cursor_pos(x,y);
	    }
	}
      else if (key > 0x1F && key < 0x7F) // Printable characters
	{
	  if (pos < len)
	    {
	      pos++;
	      x++;
	      *c=key;
	      c++;
	      putchar(password ? 0x8B : key);
	      cursor_pos(x,y);
	    }
	}
    }
  eos_end_read_keyboard();
  eos_start_read_keyboard();
}

WSSubState input_set_wifi_select(void)
{
  unsigned char k=input();
  switch(k)
    {
    case 0x0D:
      set_wifi_set_ssid(bar_get());
      return WS_PASSWORD;
    case 0x84:
      return WS_CUSTOM;
    case 0x85:
      return WS_SCAN;
    case 0x86:
      state=HOSTS_AND_DEVICES;
      return WS_DONE;
    case 0xA0:
      bar_up();
      return WS_SELECT;
    case 0xA2:
      bar_down();
      return WS_SELECT;
    default:
      return WS_SELECT;
    }
}

void input_line_set_wifi_custom(char *c)
{
  input_line(0,19,0,c,32,false);
}

void input_line_set_wifi_password(char *c)
{
  input_line(0,19,0,c,64,true);
}

HDSubState input_hosts_and_devices_hosts(void)
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
    case KEY_7:
    case KEY_8:
      bar_jump(k-KEY_1);
      return HD_HOSTS;
    case KEY_TAB:
      bar_clear(false);
      return HD_DEVICES;
    case KEY_RETURN:
      selected_host_slot=bar_get();
      if (hostSlots[selected_host_slot][0] != 0)
      {
        strcpy(selected_host_name,hostSlots[selected_host_slot]);
        state=SELECT_FILE;
        smartkeys_sound_play(SOUND_CONFIRM);
        return HD_DONE;
      }
      else
        return HD_HOSTS;
    case KEY_SMART_IV:
      state=SHOW_INFO;
      return HD_DONE;
    case KEY_SMART_V:
      smartkeys_sound_play(SOUND_POSITIVE_CHIME);
      hosts_and_devices_edit_host_slot(bar_get());
      bar_clear(false);
      bar_jump(selected_host_slot);
      k=0;
      return HD_HOSTS;
    case KEY_SMART_VI:
      smartkeys_sound_play(SOUND_CONFIRM);
      return HD_DONE;
    case KEY_UP_ARROW:
      bar_up();
      selected_host_slot=bar_get();
      return HD_HOSTS;
    case KEY_DOWN_ARROW:
      bar_down();
      selected_host_slot=bar_get();
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
    case KEY_SMART_IV:
      hosts_and_devices_eject(bar_get());
      return HD_DEVICES;
    case KEY_SMART_V:
      selected_device_slot=bar_get();
      hosts_and_devices_devices_enable_toggle(selected_device_slot);
      return HD_DEVICES;
      break;
    case KEY_SMART_VI:
      smartkeys_sound_play(SOUND_CONFIRM);
      return HD_DONE;
      break;
    case KEY_CLEAR:
      return HD_CLEAR_ALL_DEVICES;
    case KEY_UP_ARROW:
      bar_up();
      selected_device_slot=bar_get();
      hosts_and_devices_long_filename();
      return HD_DEVICES;
    case KEY_DOWN_ARROW:
      bar_down();
      selected_device_slot=bar_get();
      hosts_and_devices_long_filename();
      return HD_DEVICES;
    default:
      return HD_DEVICES;
    }
}

void input_line_hosts_and_devices_host_slot(unsigned char i, unsigned char o, char *c)
{
  input_line(1,i+1,o,c,32,false);
}

void input_line_filter(char *c)
{
  input_line(0,19,0,c,32,false);
}

SFSubState input_select_file_choose(void)
{
  unsigned entryType;
  unsigned char k=input();

  if (entry_timer>0)
    entry_timer--;

  switch(k)
    {
    case KEY_RETURN:
      pos+=bar_get();
      entryType = select_file_entry_type();
      if (entryType == ENTRY_TYPE_FOLDER)
        return SF_ADVANCE_FOLDER;
      else if (entryType == ENTRY_TYPE_LINK)
        return SF_LINK;
      else
        return SF_DONE;
    case KEY_ESCAPE:
      copy_mode=false;
      state=HOSTS_AND_DEVICES;
      return SF_DONE;
    case KEY_HOME:
      pos=0;
      dir_eof=quick_boot=false;
      return SF_DISPLAY;
    case KEY_SMART_IV:
      return strcmp(path,"/") == 0 ? SF_CHOOSE : SF_DEVANCE_FOLDER;
    case KEY_SMART_V:
      return SF_FILTER;
    case KEY_SMART_VI:
      if (copy_mode == false)
      {
        quick_boot=true;
        pos+=bar_get();
        state=SELECT_SLOT;
      }
      smartkeys_sound_play(SOUND_CONFIRM);
      return SF_DONE;
    case KEY_INSERT:
      return SF_NEW;
    case KEY_COPY:
      pos += bar_get();
      select_file_set_source_filename();
      smartkeys_sound_play(SOUND_CONFIRM);
      copy_host_slot=selected_host_slot;
      return SF_COPY;
    case KEY_UP_ARROW:
      if ((bar_get() == 0) && (pos > 0))
	return SF_PREV_PAGE;
      else
	{
	  long_entry_displayed=false;
	  entry_timer=ENTRY_TIMER_DUR;
	  bar_up();
	  select_display_long_filename();
	  return SF_CHOOSE;
	}
    case KEY_DOWN_ARROW:
      if ((bar_get() == 14) && (dir_eof==false))
	return SF_NEXT_PAGE;
      else
	{
	  long_entry_displayed=false;
	  entry_timer=ENTRY_TIMER_DUR;
	  bar_down();
	  select_display_long_filename();
	  return SF_CHOOSE;
	}
      break;
    case KEY_C_UP_ARROW:
      if (pos>0)
	return SF_PREV_PAGE;
    case KEY_C_DOWN_ARROW:
      if (dir_eof==false)
	return SF_NEXT_PAGE;
    default:
      return SF_CHOOSE;
    }
}

unsigned char input_select_file_new_type(void)
{
  switch(cgetc())
    {
    case KEY_SMART_V:
      return 1;
    case KEY_SMART_VI:
      return 2;
    default:
      return 0;
    }
}

unsigned long input_select_file_new_size(unsigned char t)
{
  switch (t)
    {
    case 1: // DDP
      switch(cgetc())
	{
	case KEY_SMART_III:
	  return 128;
	case KEY_SMART_IV:
	  return 256;
	case KEY_SMART_V:
	  return 320;
	case KEY_SMART_VI:
	  return 1; // CUSTOM
	}
      break;
    case 2: // DSK
      switch(cgetc())
	{
	case KEY_SMART_II:
	  return 160;
	case KEY_SMART_III:
	  return 320;
	case KEY_SMART_IV:
	  return 720;
	case KEY_SMART_V:
	  return 1440;
	case KEY_SMART_VI:
	  return 1; // CUSTOM
	}
      break;
    }
}

unsigned long input_select_file_new_custom(void)
{
  char c[12];
  input_line(0,19,0,c,32,false);
  return atol(c);
}

void input_select_file_new_name(char *c)
{
  input_line(0,19,0,c,255,false);
}

SSSubState input_select_slot_choose(void)
{
  unsigned char k=input();

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
    case KEY_SMART_IV:
      select_slot_eject(bar_get());
      return SS_CHOOSE;
    case KEY_RETURN:
    case KEY_SMART_V:
      selected_device_slot=bar_get();
      mode=0;
      strncpy(source_path, path, 224);
      old_pos = pos;
      return SS_DONE;
    case KEY_SMART_VI:
      selected_device_slot=bar_get();
      mode=2;
      return SS_DONE;
    case KEY_UP_ARROW:
      bar_up();
      return SS_CHOOSE;
    case KEY_DOWN_ARROW:
      bar_down();
      return SS_CHOOSE;
    default:
      return SS_CHOOSE;
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

SISubState input_show_info(void)
{
  unsigned char k=input();

  switch(k)
    {
    case 0x00:
      return SI_SHOWINFO;
    case KEY_RETURN:
    case KEY_ESCAPE:
    case KEY_SPACE:
      state=HOSTS_AND_DEVICES;
      return SI_DONE;
    case KEY_SMART_IV:
      k=io_get_device_enabled_status(2);
      if (k==true)
	io_disable_device(2);
      else
	io_enable_device(2);
      k=0;
      return SI_DONE;
    case KEY_SMART_V:
      state=SET_WIFI;
      return SI_DONE;
    case KEY_SMART_VI:
      state=CONNECT_WIFI;
      return SI_DONE;
    default:
      return SI_SHOWINFO;
    }
}

bool input_select_slot_build_eos_directory(void)
{
  return (cgetc() == KEY_SMART_V);
}

void input_select_slot_build_eos_directory_label(char *c)
{
  input_line(0,19,0,c,12,false);
}

#endif /* BUILD_ADAM */
