#ifdef BUILD_ADAM
/** 
 * Input routines
 */

#include <msx.h>
#include <eos.h>
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

static GameControllerData cont;
static unsigned char key=0;
static unsigned char joystick=0;
static unsigned char joystick_copy=0;
static unsigned char button=0;
static unsigned char button_copy=0;
static unsigned char keypad=0;
static unsigned char keypad_copy=0;
static unsigned char repeat=0;

/**
 * Get input from keyboard/joystick
 * @return keycode (or synthesized keycode if joystick)
 */
unsigned char input()
{
  key = eos_end_read_keyboard();
  
  if (key > 1)
    {
      eos_start_read_keyboard();
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
	      key=0x31;
	      break;
	    case 2: // Slot 2
	      key=0x32;
	      break;
	    case 3: // Slot 3
	      key=0x33;
	      break;
	    case 4: // Slot 4
	      key=0x34;
	      break;
	    case 0x0a: // *
	      key=0x86;
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
	      key=0xA0;
	      break;
	    case 4: // DOWN
	      key=0xA2;
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
  msx_vfill(0x1200,0x00,768);
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
      if (key == 0x0d)
	{
	  cursor(false);
	  break;
	}
      else if (key == 0x08)
	{
	  if (pos > 0)
	    {
	      pos--;
	      x--;
	      c--;
	      *c=0x00;
	      putchar(0x08);
	      putchar(0x20);
	      putchar(0x08);
	      cursor_pos(x,y);
	    }
	}
      else if (key > 0x1F && key < 0x7F) 
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
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
      bar_jump(k-0x31);
      return HD_HOSTS;
    case 0x09: // TAB
      bar_clear(false);
      return HD_DEVICES;
    case 0x0d: // RETURN
      selected_host_slot=bar_get();
      if (hostSlots[selected_host_slot][0] != 0)
	{
	  strcpy(selected_host_name,hostSlots[selected_host_slot]);
	  state=SELECT_FILE;
	  return HD_DONE;
	}
      else
	return HD_HOSTS;
    case 0x1b: // ESC
      quit();
      break;
    case 0x84: // 
      state=SHOW_INFO;
      return HD_DONE;
    case 0x85:
      hosts_and_devices_edit_host_slot(bar_get());
      bar_clear(false);
      bar_jump(selected_host_slot);
      k=0;
      return HD_HOSTS;
    case 0x86:
      return HD_DONE;
    case 0xA0:
      bar_up();
      selected_host_slot=bar_get();
      return HD_HOSTS;
    case 0xA2:
      bar_down();
      selected_host_slot=bar_get();
      return HD_HOSTS;
    }
}

HDSubState input_hosts_and_devices_devices(void)
{
  unsigned char k=input();
  switch(k)
    {
    case '1':
    case '2':
    case '3':
    case '4':
      bar_jump(k-0x31);
      selected_device_slot=bar_get();
      hosts_and_devices_long_filename();
      return HD_DEVICES;
    case 0x09:
      bar_clear(false);
      return HD_HOSTS;
    case 0x84:
      hosts_and_devices_eject(bar_get());
      return HD_DEVICES;
    case 0xA0:
      bar_up();
      selected_device_slot=bar_get();
      hosts_and_devices_long_filename();
      return HD_DEVICES;
    case 0xA2:
      bar_down();
      selected_device_slot=bar_get();
      hosts_and_devices_long_filename();
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
  unsigned char k=input();
  
  switch(k)
    {
    case 0x0d:
      pos+=bar_get();
      return SF_DONE;
    case 0x1b:
      state=HOSTS_AND_DEVICES;
      return SF_DONE;
    case 0x80:
      pos=0;
      dir_eof=quick_boot=false;
      return SF_DISPLAY;
    case 0x84:
      return strcmp(path,"/") == 0 ? SF_CHOOSE : SF_DEVANCE_FOLDER;
    case 0x85:
      return SF_FILTER;
    case 0x86:
      quick_boot=true;
      pos+=bar_get();
      state=SELECT_SLOT;
      return SF_DONE;
    case 0x94:
      return SF_NEW;
    case 0xA0:
      if ((bar_get() == 0) && (pos > 0))
	return SF_PREV_PAGE;
      else
	{
	  bar_up();
	  return SF_CHOOSE;
	}
    case 0xA2:
      if ((bar_get() == 14) && (dir_eof==false))
	return SF_NEXT_PAGE;
      else
	{
	  bar_down();
	  return SF_CHOOSE;
	}
      break;
    case 0xA4:
      if (pos>0)
	return SF_PREV_PAGE;
    case 0xA6:
      if (dir_eof==false)
	return SF_NEXT_PAGE;
    }
}

unsigned char input_select_file_new_type(void)
{
  switch(cgetc())
    {
    case 0x85:
      return 1;
    case 0x86:
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
	case 0x83:
	  return 128;
	case 0x84:
	  return 256;
	case 0x85:
	  return 320;
	case 0x86:
	  return 1; // CUSTOM
	}
      break;
    case 2: // DSK
      switch(cgetc())
	{
	case 0x83:
	  return 160;
	case 0x84:
	  return 320;
	case 0x85:
	  return 8192;
	case 0x86:
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
    case 0x1B:
      state=HOSTS_AND_DEVICES;
      return SS_ABORT;
    case '1':
    case '2':
    case '3':
    case '4':
      bar_jump(k-0x31);
      return SS_CHOOSE;
    case 0x84:
      select_slot_eject(bar_get());
      return SS_CHOOSE;
    case 0x0d:
    case 0x85:
      selected_device_slot=bar_get();
      mode=0;
      return SF_DONE;
    case 0x86:
      selected_device_slot=bar_get();
      mode=2;
      return SF_DONE;
    case 0xA0:
      bar_up();
      return SS_CHOOSE;
    case 0xA2:
      bar_down();
      return SS_CHOOSE;
    }
}

SISubState input_show_info(void)
{
  unsigned char k=input();

  switch(k)
    {
    case 0x00:
      return SI_SHOWINFO;
    case 0x0D:
    case 0x1B:
    case 0x20:
      state=HOSTS_AND_DEVICES;
      return SI_DONE;
    case 0x85:
      state=SET_WIFI;
      return SI_DONE;
    case 0x86:
      state=CONNECT_WIFI;
      return SI_DONE;
    }
}

bool input_select_slot_build_eos_directory(void)
{
  return (cgetc() == 0x85);
}

void input_select_slot_build_eos_directory_label(char *c)
{
  input_line(0,19,0,c,12,false);
}

#endif /* BUILD_ADAM */
