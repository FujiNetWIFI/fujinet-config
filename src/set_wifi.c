/**
 * #FUJINET CONFIG
 * Set Wifi Network
 */

#include <msx.h>
#include <stdlib.h>
#include <string.h>
#include <smartkeys.h>
#include <stdio.h>
#include <conio.h>
#include "fuji_adam.h"
#include "set_wifi.h"
#include "fuji_typedefs.h"

#define MAX_NETWORKS 16 // Max visible networks on screen

#define DISPLAY_ROWS 17
#define DISPLAY_LENGTH 60
#define ADDR_PATH_LINE MODE2_ATTR
#define ATTR_PATH_LINE 0xF5
#define ADDR_FILE_LIST ADDR_PATH_LINE + 256
#define ATTR_FILE_LIST 0x1F
#define ADDR_FILE_TYPE ADDR_FILE_LIST
#define ATTR_FILE_TYPE 0xF5
#define ATTR_BAR 0x13

extern char tmp[256];

/**
 * Setup 
 */
void set_wifi_setup(void)
{
  // Paint path line
  msx_vfill(ADDR_PATH_LINE,ATTR_PATH_LINE,256);

  // Paint ssid list 
  msx_vfill(ADDR_FILE_LIST,ATTR_FILE_LIST,4352);

  // Paint rssi column
  msx_vfill_v(ADDR_FILE_TYPE,ATTR_FILE_TYPE,136);
  msx_vfill_v(ADDR_FILE_TYPE+8,ATTR_FILE_TYPE,136);
  msx_vfill_v(ADDR_FILE_TYPE+16,ATTR_FILE_TYPE,136);
  

  msx_color(15,5,7);
  gotoxy(7,0); cprintf("WELCOME TO #FUJINET");
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
State set_wifi_save_network(unsigned char selectedNetwork, unsigned char numNetworks, Context *context)
{
  SSIDInfo s;
  NetConfig n;

  return CONNECT_WIFI;  
}

/**
 * Select a network
 */
State set_wifi_select_network(unsigned char numNetworks, Context *context)
{
  State new_state = CONNECT_WIFI;
  
  return new_state;
}

/**
 * Bar to select SSID
 */
void set_wifi_bar(unsigned char oldy, unsigned char y)
{
  msx_vfill(ADDR_FILE_LIST * (oldy * 256) + 256, ATTR_PATH_LINE, 24);
  msx_vfill(ADDR_FILE_LIST + (oldy * 256) + 256 + 24,ATTR_FILE_LIST, 232);
  msx_vfill(ADDR_FILE_LIST * (y * 256) + 256, ATTR_BAR, 256);
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

  for (unsigned char i=0;i<numNetworks;i++)
    {
      SSIDInfo s;

      fuji_adamnet_scan_result(i,&s);

      set_wifi_print_rssi(&s,i);
      set_wifi_print_ssid(&s,i);
    }

  set_wifi_bar(1,0);
  
  while (1) {}
  
  return new_state;
}
