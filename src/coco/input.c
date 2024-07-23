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
#include "../select_slot.h"

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
  screen_put(x,y,0xAF); // Blue cursor
}

void input_line(unsigned char x, unsigned char y, char *c, unsigned char l, bool password)
{
  int o = strlen(c);
  char k = 0;
  char *b = c;
  
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
      locate(x,y);
      input_cursor(x,y);
      k=(char)waitkey (false);

      switch (k)
	{
	case 0x08:
	  if (c>b)
	    {
	      putchar(0x08);
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
	  
	  if (x > 31)
	    {
	      x=0;
	      y++;
	    }
	  else
	    x++;
	  
	  *c = k;
	  c++;
	}

      input_cursor(x,y);
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
	input_line(0,15,c,31,false);
}

unsigned char input_select_file_new_type(void)
{
    return 1; // We always wanna make a DSK
}

unsigned long input_select_file_new_size(unsigned char t)
{
    char c[16];

    memset(c,0,sizeof(c));

    input_line(0,15,c,16,false);

    return (long)atol(c);
}

unsigned long input_select_file_new_custom(void)
{
  return 0;
}

void input_select_file_new_name(char *c)
{
    input_line(0,15,c,255,false);
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
      return HD_DONE;
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
    case 'C':
    case 'c':
      state = SHOW_INFO;
      return HD_DONE;
    case 'E':
    case 'e':
      hosts_and_devices_eject((byte)bar_get());
      break;
    case 'R':
    case 'r':
      selected_device_slot=(byte)bar_get();
      hosts_and_devices_devices_set_mode(0);
      return HD_DEVICES;
    case 'W':
    case 'w':
      selected_device_slot=(byte)bar_get();
      hosts_and_devices_devices_set_mode(2);
      return HD_DEVICES;
    case 0x03:
      return HD_DONE;
    case 0x5E: // up
      bar_up();
      break;
    case 0x0A: // down
      bar_down();
      break;
    case 0x0c: // CLEAR
      return HD_CLEAR_ALL_DEVICES;
    case 0x08:
    case 0x09:
      return HD_HOSTS;
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x33:
      bar_jump(k-'0');
      break;
    /* default: */
    /*   locate(0,10); */
    /*   printf("%02x",k); */
    /*   break; */
    }
  return HD_DEVICES;
}

SFSubState input_select_file_choose(void)
{
  char k;
  unsigned entryType=0;
  
  locate(31,15);

  while (true)
    {
      k = waitkey(true);

      switch(k)
	{
	case 'F':
	case 'f':
	  return SF_FILTER;
    case 'N':
    case 'n':
      return SF_NEW;
	case 0x03: // BREAK
	  state = HOSTS_AND_DEVICES;
	  return SF_DONE;
	case 0x08: // LEFT
	  return strcmp(path,"/") == 0 ? SF_CHOOSE : SF_DEVANCE_FOLDER;
	case 0x09: // RIGHT
	case 0x0d: // ENTER
	  pos+=bar_get();
	  entryType = select_file_entry_type();
	  if (entryType == ENTRY_TYPE_FOLDER)
	    return SF_ADVANCE_FOLDER;
	  else if (entryType == ENTRY_TYPE_LINK)
	    return SF_LINK;
	  else
	    return SF_DONE;
	case 0x5E: // up arrow
	  if ((bar_get() == 0) && (pos > 0))
	    return SF_PREV_PAGE;
	  else
	    {
	      /* long_entry_displayed=false; */
	      entry_timer=ENTRY_TIMER_DUR;
	      bar_up();
	      select_display_long_filename();
	      return SF_CHOOSE;
	    }
	case 0x5F: // shifted up arrow
	  if (pos > 0)
		return SF_PREV_PAGE;
	  break;
	case 0x0A: // down arrow
	  if ((bar_get() == 9) && (dir_eof==false))
		return SF_NEXT_PAGE;
	  else
	    {
	      /* long_entry_displayed=false; */
	      entry_timer=ENTRY_TIMER_DUR;
	      bar_down();
	      select_display_long_filename();
	      return SF_CHOOSE;
	    }
	  break;
	case 0x5B: // shifted down arrow
	  if (dir_eof == false)
		return SF_NEXT_PAGE;
	  break;
	default:
	  return SF_CHOOSE;
	}
    }
  
  return SF_CHOOSE;
}

SSSubState input_select_slot_choose(void)
{
  locate (31,14);
  char c = waitkey(true);
  switch(c)
    {
    case 0x03: // BREAK
      state=HOSTS_AND_DEVICES;
      return SS_ABORT;
    case 'E':
      select_slot_eject((char)bar_get());
      break;
    case 0x0d: // ENTER
    case 'R':
      mode=0;
      selected_device_slot=(char)bar_get();
      strncpy(source_path,path,224);
      old_pos = pos;
      return SS_DONE;
    case 'W':
      mode=2;
      selected_device_slot=(char)bar_get();
      return SS_DONE;
    case 0x5E:
      bar_up();
      return SS_CHOOSE;
    case 0x0A:
      bar_down();
      return SS_CHOOSE;      
    default:
      return SS_CHOOSE;
    }
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
