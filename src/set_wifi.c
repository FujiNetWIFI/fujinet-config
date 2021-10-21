/**
 * #FUJINET CONFIG
 * Set Wifi Network
 */

#include <msx.h>
#include <eos.h>
#include <stdlib.h>
#include <string.h>
#include <smartkeys.h>
#include <stdio.h>
#include <conio.h>
#include <stdbool.h>
#include "fuji_adam.h"
#include "set_wifi.h"
#include "fuji_typedefs.h"
#include "bar.h"
#include "input.h"

#define MAX_NETWORKS 16 // Max visible networks on screen

#define SSID_CUSTOM 0xFE
#define SSID_SKIP 0xFF

#define DISPLAY_ROWS 17
#define DISPLAY_LENGTH 60
#define ADDR_PATH_LINE MODE2_ATTR
#define ATTR_PATH_LINE 0xF5
#define ATTR_PATH_LINE_GREYED 0xF1
#define ADDR_FILE_LIST ADDR_PATH_LINE + 256
#define ATTR_FILE_LIST 0x1F
#define ATTR_FILE_LIST_GREYED 0x1E
#define ADDR_FILE_TYPE ADDR_FILE_LIST
#define ATTR_FILE_TYPE 0xF5
#define ATTR_FILE_TYPE_GREYED 0xF1
#define ATTR_BAR 0x13
#define ATTR_BAR_GREYED 0x1E
extern char tmp[256];

/**
 * Attrs
 */
void set_wifi_attrs(bool greyed)
{
  // Paint path line
  msx_vfill(ADDR_PATH_LINE,greyed == true ? ATTR_PATH_LINE_GREYED : ATTR_PATH_LINE,256);

  // Paint ssid list 
  msx_vfill(ADDR_FILE_LIST,greyed == true ? ATTR_FILE_LIST_GREYED : ATTR_FILE_LIST,4352);

  // Paint rssi column
  msx_vfill_v(ADDR_FILE_TYPE,greyed == true ? ATTR_FILE_TYPE_GREYED : ATTR_FILE_TYPE,136);
  msx_vfill_v(ADDR_FILE_TYPE+8,greyed == true ? ATTR_FILE_TYPE_GREYED : ATTR_FILE_TYPE,136);
  msx_vfill_v(ADDR_FILE_TYPE+16,greyed == true ? ATTR_FILE_TYPE_GREYED : ATTR_FILE_TYPE,136);

  msx_color(15,greyed == true ? 1 : 5,7);
  gotoxy(7,0); cprintf("WELCOME TO #FUJINET");

}

/**
 * Setup 
 */
void set_wifi_setup(void)
{
  smartkeys_set_mode();
  clrscr();
}

/**
 * Print SSID
 */
void set_wifi_print_ssid(SSIDInfo *s, unsigned char i)
{
  msx_color(1,15,7);
  smartkeys_puts(24, (i*8)+8, (char *)s->ssid);
}

/**
 * Print RSSI (strength)
 */
void set_wifi_print_rssi(SSIDInfo *s, unsigned char i)
{
  char out[4] = {0x20, 0x20, 0x20, 0x00};
  
  if (s->rssi > -40)
    {
      out[0] = 0x10;
      out[1] = 0x11;
      out[2] = 0x12;
    }
  else if (s->rssi > -60)
    {
      out[0] = 0x10;
      out[1] = 0x11;
    }
  else
    {
      out[0] = 0x10;
    }
  
  msx_color(15,5,7);

  smartkeys_puts(0, (i*8) + 8, out);
}

/**
 * Save selection
 * selectedNetwork = index from wifi scan.
 */
State set_wifi_save_network(unsigned char selectedNetwork, Context *context)
{
  SSIDInfo s;
  NetConfig n;

  if (selectedNetwork == 0xFF)
    return DISKULATOR_HOSTS;
  else if (selectedNetwork == 0xFE)
    {
      smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
      smartkeys_status("  ENTER NAME OF NETWORK.");
      input_line(n.ssid,false);
    }
  else
    {
      fuji_adamnet_scan_result(selectedNetwork,&s);
      memcpy(n.ssid,s.ssid,sizeof(n.ssid));
    }

  smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
  smartkeys_status("  ENTER PASSWORD FOR NETWORK\n  OR PRESS RETURN FOR NONE.");
  input_line(n.password,true);

  fuji_adamnet_set_ssid(true,&n);
  
  return CONNECT_WIFI;  
}

/**
 * Set wifi State
 */
State set_wifi(Context *context)
{
  AdapterConfig adapterConfig;
  State new_state = CONNECT_WIFI;
  unsigned char numNetworks=0;
  
  set_wifi_setup();
  
  smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
  smartkeys_status("  SCANNING FOR NETWORKS. PLEASE WAIT...");

  numNetworks = fuji_adamnet_do_scan();
  
  while (numNetworks == 0)
    {
      numNetworks = fuji_adamnet_do_scan();
    }

  smartkeys_display(NULL,NULL,NULL," CUSTOM","  NONE"," RE-SCAN");
  sprintf(tmp,"  %d NETWORKS FOUND.\n  SELECT A NETWORK.",numNetworks);
  smartkeys_status(tmp);

  set_wifi_attrs(false);
  
  for (unsigned char i=0;i<numNetworks;i++)
    {
      SSIDInfo s;

      fuji_adamnet_scan_result(i,&s);

      set_wifi_print_rssi(&s,i);
      set_wifi_print_ssid(&s,i);
    }

  bar_set(0,3,numNetworks,0);

  eos_start_read_keyboard();
  
  while (1)
    {
      switch(input())
	{
	case 0x0D:
	  return set_wifi_save_network(bar_get(),context);
	case 0x84:
	  return set_wifi_save_network(SSID_CUSTOM,context); // Custom SSID
	case 0x85:
	  return set_wifi_save_network(SSID_SKIP,context); // Skip
	case 0x86:
	  return SET_WIFI; // re-scan
	case 0xA0:
	  bar_up();
	  break;
	case 0xA2:
	  bar_down();
	  break;
	}
    }
  
  return new_state; // we never actually get here
}
