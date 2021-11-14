#ifdef BUILD_APPLE2
/**
 * #FujiNet CONFIG FOR Apple2
 *
 * Screen Routines
 */

#include "screen.h"
#include "globals.h"
#include "bar.h"
#include <conio.h>
#include <string.h>

void screen_init(void)
{
}

void screen_error(const char *c)
{
}

void screen_set_wifi(AdapterConfig *ac)
{
}

void screen_set_wifi_display_ssid(char n, SSIDInfo *s)
{
}

void screen_set_wifi_select_network(unsigned char nn)
{
}

void screen_set_wifi_custom(void)
{
}

void screen_set_wifi_password(void)
{
}

void screen_connect_wifi(NetConfig *nc)
{
}

char* screen_hosts_and_devices_slot(char *c)
{
  return c[0]==0x00 ? "Empty" : c;
}

void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *d)
{
  for (char i=0;i<4;i++)
    {
      gotoxy(0,i+y); cprintf("%d%s",i+1,screen_hosts_and_devices_slot(d[i].file));
    }
}

void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d)
{
}

void screen_hosts_and_devices_hosts(void)
{
}

void screen_hosts_and_devices_devices(void)
{
}

void screen_hosts_and_devices_clear_host_slot(unsigned char i)
{
}

void screen_hosts_and_devices_edit_host_slot(unsigned char i)
{
}

void screen_show_info(AdapterConfig* ac)
{
}

void screen_select_file(void)
{
}

void screen_select_file_prev(void)
{
}

void screen_select_file_next(void)
{
}

void screen_select_file_display_entry(unsigned char y, char* e)
{
}

void screen_select_file_choose(char visibleEntries)
{
}

void screen_select_file_filter(void)
{
}

void screen_select_slot(char *e)
{
}

void screen_select_slot_choose(void)
{
}

void screen_select_slot_mode(void)
{
}

void screen_select_slot_eject(unsigned char ds)
{
}

void screen_hosts_and_devices_eject(unsigned char ds)
{
}

void screen_hosts_and_devices_host_slot_empty(unsigned char hs)
{
}  
#endif /* BUILD_APPLE2 */
