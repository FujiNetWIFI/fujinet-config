#ifdef _CMOC_VERSION_

/**
 * Input routines
 */

#include <cmoc.h>
#include <coco.h>
#include "input.h"
#include "io.h"
#include "bar.h"
#include "screen.h"
#include "globals.h"
#include "mount_and_boot.h"
#include "../hosts_and_devices.h"
#include "../select_file.h"
#include "../set_wifi.h"

unsigned char selected_network;
extern bool copy_mode;
extern unsigned char copy_host_slot;
unsigned short custom_numSectors;
unsigned short custom_sectorSize;
extern char fn[256];
bool mounting = false;
extern unsigned short entry_timer;
extern HDSubState hd_subState;

/**
 * @brief this routine is needed because waitkey() and readline()
 *        always emit uppercase. argh!
 */
char input()
{
  char shift=false;
  char k;
  
  while (true)
    {
      k=inkey();
      
      if (isKeyPressed(KEY_PROBE_SHIFT,KEY_BIT_SHIFT))
	{
	  shift=0x00;
	}
      else
	{
	  if (k>'@' && k<'[')
	    shift=0x20;
	}

      if (k)
	return k+shift;
    }  
}

unsigned char input_ucase()
{
  return 0;
}

unsigned char input_handle_joystick(void)
{
  return 0;
}

// Old cursor position
char ox=-1;
char oy=-1;
char oc=0;

void input_cursor(char x, char y)
{
  if (ox > -1)
    {
      // Replace character at old position
      screen_put(ox,oy,oc);
    }

  ox=x;
  oy=y;
  oc=screen_get(x,y);
  screen_put(x,y,0xAF); // Blue cursor
}

void input_line(unsigned char x, unsigned char y, char *c, unsigned char l, bool password)
{
  int o = strlen(c);
  char k = 0;
  char *b = c;
  
  // Print existing string
  locate(x,y);
  printf("%s",c);

  // Place string pointer at end of string
  while (*c)
    {
      putchar(*c);
      c++;
      x++;
      if (x>31)
	{
	  y++;
	  x=0;
	}
    }

  input_cursor(x,y);
  
  while (k!=0x0D) // ENTER
    {
      k=input();

      switch (k)
	{
	case 0x08:
	  putchar(0x08);
	  if (c>b)
	    {
	      c--;
	      x--;
	      *c = 0;
	    }
	  break;
	case 0x09:
	  if (*c)
	    {
	      c++;
	      x++;
	    }
	  break;
	case 0x0d: // Ignore it.
	  break;
	default:
	  if (password)
	    {
	      putchar('*');
	    }
	  else
	    {
	      putchar(k);
	    }
	  
	  input_cursor(x,y);
	  x++;

	  if (x > 31)
	    {
	      x=0;
	      y++;
	    }
	  *c = k;
	  c++;  
	}
    }
}

void input_line_set_wifi_custom(char *c)
{
  input_line(0,15,c,32,false);
}

void input_line_set_wifi_password(char *c)
{
  input_line(0,15,c,64,true);
}

void input_line_hosts_and_devices_host_slot(int i, unsigned int o, char *c)
{
  bar_clear(true);
  input_line(1,(unsigned char)i+1,c,32,false);
}

void input_line_filter(char *c)
{
  while (true)
    printf("%c",inkey());
}

unsigned char input_select_file_new_type(void)
{
  return 1;
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
}

bool input_select_slot_build_eos_directory(void)
{
  return false;
}

void input_select_slot_build_eos_directory_label(char *c)
{
}

WSSubState input_set_wifi_select(void)
{
  char k = waitkey(true);

  switch(k)
    {
    case 0x0d:
      set_wifi_set_ssid(bar_get());
      return WS_PASSWORD;
    case 'H':
    case 'h':
      return WS_CUSTOM;
    case 'R':
    case 'r':
      return WS_SCAN;
    case 0x5E:
      bar_up();
      break;
    case 0x0A:
      bar_down();
      break;
    default:
      return WS_SELECT;
    }

  return WS_SELECT;
}

/**
 * Handle global console keys.
 */
unsigned char input_handle_console_keys(void)
{
  return 0;
}

HDSubState input_hosts_and_devices_hosts(void)
{
  locate(31,14);
  
  char k=waitkey(true);

  switch(k)
    {
    case 0x03:
      locate(0,10);
      exit(0);
      break;
    case 0x5E: // up
      bar_up();
      break;
    case 0x0A: // down
      bar_down();
      break;
    case 0x0d: // enter
      selected_host_slot=(unsigned char)bar_get();
      if (hostSlots[selected_host_slot][0] != 0)
	{
	  strcpy((char *)selected_host_name, (char *)hostSlots[selected_host_slot]);
	  state = SELECT_FILE;
	  return HD_DONE;
	}
      else
	return HD_HOSTS;
    case 0x08:
    case 0x09:
      return HD_DEVICES;
    case 'C':
      state = SHOW_INFO;
      return HD_DONE;
    case 'E':
      hosts_and_devices_edit_host_slot(bar_get());
      bar_clear(true);
      bar_jump(selected_host_slot);
      k = 0;
      return HD_HOSTS;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
      break;
    }
  return HD_HOSTS;
}

HDSubState input_hosts_and_devices_devices(void)
{
  locate(31,15);
  
  char k=waitkey(true);

  switch(k)
    {
    case 0x03:
      locate(0,10);
      exit(0);
      break;
    case 0x5E: // up
      bar_up();
      break;
    case 0x0A: // down
      bar_down();
      break;
    case 0x08:
    case 0x09:
      return HD_HOSTS;
    case 0x31:
    case 0x32:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37:
    case 0x38:
      bar_jump(k-'1');
      break;
    }
  return HD_DEVICES;
}

SFSubState input_select_file_choose(void)
{
  return SF_CHOOSE;
}

SSSubState input_select_slot_choose(void)
{
  return SS_CHOOSE;
}

unsigned char input_select_slot_mode(char *mode)
{
  return 1;
}

/*
 *  Handle inupt for the "show info" screen.
 *
 *  'C' - Reconnect Wifi
 *  'S' - Change SSID
 *  Any other key - return to main hosts and devices screen
 *
 */
SISubState input_show_info(void)
{
  char c = waitkey(true);
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

DHSubState input_destination_host_slot_choose(void)
{
  return DH_CHOOSE;
}

void set_device_slot_mode(unsigned char slot, unsigned char mode)
{
}

#endif /* _CMOC_VERSION_ */
