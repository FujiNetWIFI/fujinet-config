/**
 * #FUJINET CONFIG
 * Diskulator Info/Devices
 */

#define VERSION "2.5.2020-08-25"

#include <msx.h>
#include <conio.h>
#include <smartkeys.h>
#include "diskulator_info.h"
#include "fuji_typedefs.h"
#include "fuji_adam.h"
#include "input.h"

extern AdapterConfig ac;

/**
 * Connect wifi State
 */
State diskulator_info(void)
{
  State new_state = DISKULATOR_HOSTS;
  char k=0;

  smartkeys_set_mode();
  clrscr();
  
  fuji_adamnet_read_adapter_config(&ac);

  msx_vfill(MODE2_ATTR+1792,0xF4,256);
  
  msx_color(15,4,7);
  gotoxy(11,7); cprintf("FUJINET CONFIGURATION");
  
  msx_vfill(MODE2_ATTR,0xF2,256);
  msx_vfill(MODE2_ATTR+768,0xF2,256);

  for (char i=0;i<7;i++)
    {
      msx_vfill(MODE2_ATTR+2048+(i<<8),0xF4,112);
      msx_vfill(MODE2_ATTR+2136+(i<<8),0x1F,168);
    }
  
  msx_color(15,2,7);
  gotoxy(28,0); cprintf("SSID");
  gotoxy(24,3); cprintf("HOSTNAME");

  msx_color(15,4,7);
  gotoxy(2,8);  cprintf("     IP:");
  gotoxy(2,9);  cprintf("NETMASK:");
  gotoxy(2,10); cprintf("GATEWAY:");
  gotoxy(2,11); cprintf("    DNS:");
  gotoxy(2,12); cprintf("    MAC:");
  gotoxy(2,13); cprintf("  BSSID:");
  gotoxy(2,14); cprintf("VERSION:");
  
  msx_color(1,15,7);
  gotoxy(0,1); cprintf("%32s",ac.ssid);
  gotoxy(0,4); cprintf("%32s",ac.hostname);
  gotoxy(12,8); cprintf("%u.%u.%u.%u\n",ac.localIP[0],ac.localIP[1],ac.localIP[2],ac.localIP[3]);
  gotoxy(12,9); cprintf("%u.%u.%u.%u\n",ac.netmask[0],ac.netmask[1],ac.netmask[2],ac.netmask[3]);
  gotoxy(12,10); cprintf("%u.%u.%u.%u\n",ac.gateway[0],ac.gateway[1],ac.gateway[2],ac.gateway[3]);
  gotoxy(12,11); cprintf("%u.%u.%u.%u\n",ac.dnsIP[0],ac.dnsIP[1],ac.dnsIP[2],ac.dnsIP[3]);
  gotoxy(12,12); cprintf("%02X:%02X:%02X:%02X:%02X:%02X\n",ac.macAddress[0],ac.macAddress[1],ac.macAddress[2],ac.macAddress[3],ac.macAddress[4],ac.macAddress[5]);
  gotoxy(12,13); cprintf("%02X:%02X:%02X:%02X:%02X:%02X\n",ac.bssid[0],ac.bssid[1],ac.bssid[2],ac.bssid[3],ac.bssid[4],ac.bssid[5]);
  gotoxy(12,14); cprintf("%s",ac.fn_version);
  
  smartkeys_display(NULL,NULL,NULL,NULL," CHANGE\n  SSID"," CONNECT");

  k=cgetc();

  switch(k)
    {
    case 0x86:
      new_state = CONNECT_WIFI;
      break;
    case 0x85:
      new_state = SET_WIFI;
      break;
    }

  return new_state;
}
