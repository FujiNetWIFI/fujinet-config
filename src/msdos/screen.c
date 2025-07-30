#ifdef __WATCOMC__

/**
 * @brief screen routines
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 */
#include <stdio.h>
#include <dos.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include "screen.h"
#include "bar.h"
#include "io.h"

/**
 * @brief screen globals
 */
unsigned char far *video = NULL;
unsigned char screen_cols = 40;
unsigned char screen_mode = 0x03;
bool screen_is_color = true;
char text_empty[] = "Empty";
char fn[256];
char _visibleEntries;

extern bool copy_mode;

/**
 * @brief Determine video segment address, return it.
 * @return far pointer to video segment address (0xb000, 0xb800, or 0xa000)
 * @verbose may be moved elsewhere, but it's here, for now.
 */
static unsigned char far *screen_get_video_segment_address(void)
{
    union REGS regs;
    unsigned char video_mode=0;

    regs.h.ah = 0x0F; // Get video mode
    int86(0x10,&regs,&regs);

    video_mode = regs.h.al;

    if (video_mode <= 0x07)
    {
        // Get equipment flags to determine MDA
        unsigned short equip = *(unsigned short far *)MK_FP(0x0040, 0x0010);

        // If bits 4 and 5 are set, then we have MDA.
        if ((equip & 0x30) == 0x30)
            return MK_FP(0xB000,0x0000);
        else
            return MK_FP(0xB800,0x0000);
    }
    else if (video_mode >= 0x0d && video_mode <= 0x0f) // CGA video modes.
        return MK_FP(0xB800,0x0000);

    return MK_FP(0xA000,0x0000); // Assume EGA/VGA if all else fails.
}

/**
 * @brief place a character on screen, at x, y, with selected attribute
 * @param x Column (00-screen_cols-1)
 * @param y Line (00-24)
 * @param a The color attribute
 * @param c The character to display
 */
void screen_putc(unsigned char x, unsigned char y, unsigned char a, const char c)
{
    union REGS r;

    /* Set cursor position */
    r.h.ah = 0x02;
    r.h.bh = 0;
    r.h.dh = y;
    r.h.dl = x;
    int86(0x10,&r,&r);

    r.h.ah = 0x09;
    r.h.al = c;
    r.h.bh = 0;
    r.h.bl = a;
    r.x.cx = 1;
    int86(0x10,&r,&r);
}

/**
 * @brief Centered positioning of string s on line y
 * @param y Line (00-24)
 * @param a Attribute to use
 * @param s NULL terminated String to display
 */
void screen_puts_center(unsigned char y, unsigned char a, const char *s)
{
    unsigned char x = (screen_cols >> 1) - (strlen(s) >> 1);
    screen_puts(x,y,a,s);
}

/**
 * @brief place a NULL terminated string on screen, with selected attribute
 * @param x Column (00-screen_cols-1)
 * @param y Line (00-24)
 * @param
 */
void screen_puts(unsigned char x, unsigned char y, unsigned char a, const char *s)
{
    char c=0;

    while (c = *s++)
    {
        if (x>screen_cols-1)
        {
            x=0;
            y++;
        }

        screen_putc(x++,y,a,c);
    }
}

/**
 * @brief display mounting all slots message
 */
void screen_mount_and_boot(void)
{
    screen_clear();
    screen_puts(0,0,0x0f,"MOUNTING ALL SLOTS...");
}

void screen_set_wifi(AdapterConfig *ac)
{
  char tmp[20];

  sprintf(tmp,"%02X:%02X:%02X:%02X:%02X:%02X",
	  ac->macAddress[0],
	  ac->macAddress[1],
	  ac->macAddress[2],
	  ac->macAddress[3],
	  ac->macAddress[4],
	  ac->macAddress[5]);

  screen_clear();
  screen_puts_center(0,ATTRIBUTE_HEADER,"WELCOME TO FUJINET!");
  screen_puts_center(24,ATTRIBUTE_NORMAL,"SCANNING NETWORKS...");
  screen_puts(0,2,ATTRIBUTE_NORMAL,"MAC Address:");
  screen_puts(13,2,ATTRIBUTE_BOLD,tmp);
}

void screen_set_wifi_print_rssi(SSIDInfo *s, unsigned char i)
{
    char out[4] = {0x20, 0x20, 0x20, 0x00};
    unsigned char x=screen_cols == 40 ? 35 : 70;

    if (s->rssi > -40)
    {
        out[0] = 0xB2;
        out[1] = 0xB2;
        out[2] = 0xB2;
    }
    else if (s->rssi > -60)
    {
        out[0] = 0xB1;
        out[1] = 0xB1;
    }
    else
    {
        out[0] = 0xB0;
    }

    screen_puts(x, i + NETWORKS_START_Y, ATTRIBUTE_BOLD ,out);
}

void screen_set_wifi_display_ssid(char n, SSIDInfo *s)
{
    unsigned char x = screen_cols == 40 ? 2 : 32;
    screen_puts(x, n + NETWORKS_START_Y, ATTRIBUTE_NORMAL, (char *)s->ssid);
    screen_set_wifi_print_rssi(s, n);
}

void screen_set_wifi_select_network(unsigned char nn)
{
    unsigned char x = screen_cols == 40 ? 2 : 32;

    screen_clear_line(numNetworks + NETWORKS_START_Y);
    screen_puts(x, NETWORKS_START_Y + numNetworks, ATTRIBUTE_NORMAL, "<Enter a specific SSID>");

    screen_puts(x-2, 23, ATTRIBUTE_BOLD, " SELECT NET, S SKIP "
                "   ESC TO RE-SCAN   ");

    bar_set(2,1,nn,0);
}

void screen_set_wifi_custom(void)
{
    screen_clear_line(20);
    screen_puts_center(22, ATTRIBUTE_BOLD, " ENTER NETWORK NAME "
                "  AND PRESS <RETURN>  ");
}

void screen_set_wifi_password(void)
{
    screen_clear_line(23);
    screen_clear_line(24);
    screen_puts_center(23, ATTRIBUTE_BOLD, "    ENTER PASSWORD");
}

/*
 * Display the 'info' screen
 */
void screen_show_info(int printerEnabled, AdapterConfig *ac)
{
    unsigned char x = screen_cols == 40 ? 0 : 22;
    char tmp[80];

    screen_clear();

    screen_puts(x+3, 5, ATTRIBUTE_HEADER,"FUJINET CONFIG");
    screen_puts_center(17,ATTRIBUTE_NORMAL,"[R]econnect  Change [S]SID");
    screen_puts(x+9, 19, ATTRIBUTE_BOLD,"Any other key to return");
    screen_puts(x+5, 7, ATTRIBUTE_NORMAL,"      SSID:");
    screen_puts(x+5, 8, ATTRIBUTE_NORMAL,"  Hostname:");
    screen_puts(x+5, 9, ATTRIBUTE_NORMAL,"IP Address:");
    screen_puts(x+5, 10, ATTRIBUTE_NORMAL,"   Gateway:");
    screen_puts(x+5, 11, ATTRIBUTE_NORMAL,"       DNS:");
    screen_puts(x+5, 12, ATTRIBUTE_NORMAL,"   Netmask:");
    screen_puts(x+5, 13, ATTRIBUTE_NORMAL,"       MAC:");
    screen_puts(x+5, 14, ATTRIBUTE_NORMAL,"     BSSID:");
    screen_puts(x+5, 15, ATTRIBUTE_NORMAL,"   Version:");

    screen_puts(x+17, 7, ATTRIBUTE_BOLD,ac->ssid);
    screen_puts(x+17, 8, ATTRIBUTE_BOLD,ac->hostname);

    sprintf(tmp,"%03u.%03u.%03u.%03u",
	    ac->localIP[0],
	    ac->localIP[1],
	    ac->localIP[2],
	    ac->localIP[3]);
    screen_puts(x+17, 9, ATTRIBUTE_BOLD,tmp);

    sprintf(tmp,"%03u.%03u.%03u.%03u",
	    ac->gateway[0],
	    ac->gateway[1],
	    ac->gateway[2],
	    ac->gateway[3]);
    screen_puts(x+17, 10, ATTRIBUTE_BOLD,tmp);

    sprintf(tmp,"%03u.%03u.%03u.%03u",
	    ac->dnsIP[0],
	    ac->dnsIP[1],
	    ac->dnsIP[2],
	    ac->dnsIP[3]);
    screen_puts(x+17, 11, ATTRIBUTE_BOLD,tmp);

    sprintf(tmp,"%03u.%03u.%03u.%03u",
	    ac->netmask[0],
	    ac->netmask[1],
	    ac->netmask[2],
	    ac->netmask[3]);
    screen_puts(x+17, 12, ATTRIBUTE_BOLD,tmp);

    sprintf(tmp,"%02X:%02X:%02X:%02X:%02X:%02X",
	    ac->macAddress[0],
	    ac->macAddress[1],
	    ac->macAddress[2],
	    ac->macAddress[3],
	    ac->macAddress[4],
	    ac->macAddress[5]);
    screen_puts(x+17, 13, ATTRIBUTE_BOLD,tmp);

    sprintf(tmp,"%02X:%02X:%02X:%02X:%02X:%02X",
	    ac->bssid[0],
	    ac->bssid[1],
	    ac->bssid[2],
	    ac->bssid[3],
	    ac->bssid[4],
	    ac->bssid[5]);
    screen_puts(x+17, 14, ATTRIBUTE_BOLD,tmp);
    screen_puts(x+17, 15, ATTRIBUTE_BOLD,ac->fn_version);
}

void screen_select_slot(char *e)
{
  screen_clear();

  screen_puts_center(22,ATTRIBUTE_NORMAL,"[1-8] Slot [RETURN] Select [E]ject [ESC] Abort");

  screen_puts_center(0, ATTRIBUTE_HEADER, "MOUNT TO DRIVE SLOT");

  // Show file details if it's an existing file only.
  if ( create == false )
  {
    // Modified time
    // sprintf(d, "%8s %04u-%02u-%02u %02u:%02u:%02u", "MTIME:", (*e++) + 1970, *e++, *e++, *e++, *e++, *e++);

    // Remove for now (wasn't in original config, not really all that important and removng sprintf usage), so skip over the 6 bytes for the file date/time info.
    e += 6;

    // File size
    // only 2 bytes, so max size is 65535.. don't show for now until SIO method is changed to return more.
    // Result is unreliable since if the file was < 65535 bytes it will be ok, but if it was more we don't
    // know how to interpret the 2 bytes we have available to us.
    //s = (unsigned int *)e;
    //sprintf(d, "%8s %u bytes", "SIZE:", *s);
    //sprintf(d, "%8s %u K", "SIZE:", *s >> 10);
    //screen_puts(0, DEVICES_END_MOUNT_Y + 4, d);

    // Skip next 4 bytes to get to the filename (2 for the size, 2 for flags we don't care about)
    e += 4;

    // Filename
    screen_puts(1, DEVICES_END_MOUNT_Y + 2, ATTRIBUTE_NORMAL, "FILE:");
    screen_puts(7, DEVICES_END_MOUNT_Y + 2, ATTRIBUTE_BOLD, e);
  }

  screen_hosts_and_devices_device_slots(DEVICES_START_MOUNT_Y, &deviceSlots, &deviceEnabled[0]);

  bar_set(1,1,NUM_DEVICE_SLOTS,0);
}

void screen_select_slot_mode(void)
{
    screen_clear_line(21);
    screen_puts_center(22,ATTRIBUTE_NORMAL,"[RETURN] Read Only [W] Read/Write [ESC] Abort");
}

void screen_select_slot_choose(void)
{
}

void screen_select_slot_eject(unsigned char ds)
{
}

void screen_hosts_and_devices_eject(unsigned char ds)
{

}

void screen_select_slot_build_eos_directory(void)
{
}

void screen_select_slot_build_eos_directory_label(void)
{
}

void screen_select_slot_build_eos_directory_creating(void)
{
}

void screen_select_file(void)
{
    screen_clear();
    bar_clear(false);

    screen_puts_center(0, ATTRIBUTE_HEADER, "DISK IMAGES");

    if (copy_mode == false)
    {
        screen_puts_center(21,ATTRIBUTE_NORMAL, "[BACKSPACE] Up Dir [N]ew [F]ilter [C]opy");
    }
    else
    {
        screen_puts_center(21,ATTRIBUTE_NORMAL, "[BACKSPACE] Up Dir [N]ew [F]ilter [C]Do It!");
    }
    screen_puts_center(22,ATTRIBUTE_NORMAL,"[RETURN] Choose [F1] Boot [ESC] Abort");
}

void screen_select_file_display(char *p, char *f)
{

    unsigned char i;
    unsigned char x = screen_cols == 40 ? 0 : 20;

    // Host
    screen_puts(x+0, 1, ATTRIBUTE_HEADER, "Host:");
    screen_puts(x+5, 1, ATTRIBUTE_BOLD, selected_host_name);

    // Filter
    screen_puts(x+0, 2, ATTRIBUTE_HEADER, "Fltr:");
    screen_puts(5, 2, ATTRIBUTE_BOLD, f);

    // Path - the path can wrap to line 4 (maybe 5?) so clear both to be safe.
    screen_clear_line(3);
    screen_clear_line(4);
    screen_puts(x+0, 3, ATTRIBUTE_HEADER, "Path:");
    screen_puts(5, 3, ATTRIBUTE_BOLD, p);

    // Clear out the file area
    for (i = FILES_START_Y; i < FILES_START_Y + ENTRIES_PER_PAGE; i++)
    {
        screen_clear_line(i);
    }

    // clear Prev/next page lines. Sometimes they're left on the screen during directory devance.
    screen_clear_line(FILES_START_Y + ENTRIES_PER_PAGE);
    screen_clear_line(FILES_START_Y - 1);
}

void screen_select_file_display_long_filename(char *e)
{
    screen_puts(0, 24, ATTRIBUTE_NORMAL, e);
}

void screen_select_file_clear_long_filename(void)
{
    screen_clear_line(24);
    screen_clear_line(25);
}

void screen_select_file_filter(void)
{
    // No need to display anything additional, we're already showing the filter on the screen.
}

void screen_select_file_next(void)
{
    if (dir_eof == false)
    {
        screen_puts(0, FILES_START_Y + ENTRIES_PER_PAGE, ATTRIBUTE_NORMAL, "[PgDn] Next Page");
    }
    if (pos == 0)
    {
        // We're on the first page, clear the line with the "Previous Page" text.
        screen_clear_line(FILES_START_Y - 1);
    }
}

void screen_select_file_prev(void)
{
    if (pos > 0)
    {
        screen_puts(0, FILES_START_Y - 1, ATTRIBUTE_NORMAL, "[PgUp] Previous Page");
    }

    if (dir_eof == true)
    {
        // We're on the last page, clear the line with the "Next Page" text.
        screen_clear_line(FILES_START_Y + ENTRIES_PER_PAGE);
    }
}

void screen_select_file_display_entry(unsigned char y, char *e, unsigned entryType)
{

    /*
      if (e[strlen(e)-1]=='/')
      screen_puts(0,FILES_START_Y+y,CH_FOLDER);
      else if (e[0]=='=')
      screen_puts(0,FILES_START_Y+y,CH_SERVER);
      else
    */

    if (e[strlen(e)-1]=='/')
    {
        screen_puts(0,FILES_START_Y+y,ATTRIBUTE_BOLD, "/");
    }
    else if (e[0]=='=')
    {
        screen_puts(0,FILES_START_Y+y,ATTRIBUTE_BOLD,"=");
    }

    screen_puts(3, FILES_START_Y + y, ATTRIBUTE_NORMAL, e);

}

void screen_select_file_choose(char visibleEntries)
{
    bar_set(FILES_START_Y,0,visibleEntries,0);
    _visibleEntries = visibleEntries;
}

void screen_select_file_new_type(void)
{
    // Not used on Atari
}

void screen_select_file_new_size(unsigned char k)
{
    unsigned char x = screen_cols == 40 ? 0 : 22;

    screen_clear_line(20);
    screen_clear_line(21);

    screen_puts(x+0, 20, ATTRIBUTE_BOLD, "[1] 360K [2] 720K [3] 1.2MB [4] 1.44MB");
}

void screen_select_file_new_custom(void)
{
    screen_clear_line(20);
    screen_clear_line(21);

    screen_puts(0, 20, ATTRIBUTE_NORMAL, "# 512 byte Sectors?");
}

void screen_select_file_new_name(void)
{
    screen_clear_line(20);
    screen_clear_line(21);
    screen_puts(0, 20, ATTRIBUTE_NORMAL, "Enter name of new disk image file");
}

void screen_select_file_new_creating(void)
{
    screen_clear();
    screen_puts(3, 0, ATTRIBUTE_BOLD, "Creating File");
}

/**
 * @brief Clear the screen
 * @verbose we'll just assume 80 cols with attributes.
 */
void screen_clear(void)
{
    _fmemset(video,0,4000);
}

void screen_error(const char *msg)
{
    screen_clear_line(24);
    screen_puts(0, 24, ATTRIBUTE_BOLD, msg);
}

void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d, bool *e)
{
    unsigned char x = screen_cols == 40 ? 0 : 22;

    screen_clear();
    bar_clear(false);

    screen_puts(x+5, 0, ATTRIBUTE_HEADER, "HOST LIST");
    screen_puts(x+4, 11, ATTRIBUTE_HEADER, "DRIVE SLOTS");

    screen_hosts_and_devices_host_slots(&hostSlots[0]);

    screen_hosts_and_devices_device_slots(DEVICES_START_Y, &deviceSlots[0], NULL);
}

// Show the keys that are applicable when we are on the Hosts portion of the screen.
void screen_hosts_and_devices_hosts(void)
{
    unsigned char x = screen_cols == 40 ? 0 : 22;

    screen_clear_line(22);
    screen_clear_line(23);
    screen_puts(x+0,22,ATTRIBUTE_NORMAL, "[1-8] Slot [E]dit [RETURN] Browse [L]obby [C]onfig [TAB] Drives [ESC] Boot");

    // TODO check this
    bar_set(1,1,8,selected_host_slot);
}

// Show the keys that are applicable when we are on the Devices portion of the screen.
void screen_hosts_and_devices_devices(void)
{
    unsigned char x = screen_cols == 40 ? 0 : 22;

    screen_clear_line(22);
    screen_clear_line(23);

    screen_clear_line(11);
    screen_puts(x+4, 11, ATTRIBUTE_HEADER, "DRIVE SLOTS");

    screen_puts(x+0, 22, ATTRIBUTE_NORMAL, "[1-8] Slot [E]ject [Del] Clear Slots [L]obby [TAB] Hosts [R]ead [W]rite [C]onfig");

    bar_set(DEVICES_START_Y, 1, NUM_DEVICE_SLOTS, selected_device_slot);
}

void screen_hosts_and_devices_host_slots(HostSlot *h)
{
    unsigned char slotNum;

    for (slotNum = 0; slotNum < NUM_HOST_SLOTS; slotNum++)
    {
        char s[2] = {0x00,0x00};

        s[0] = slotNum + '1';

        screen_puts(2,slotNum+HOSTS_START_Y,ATTRIBUTE_BOLD, s);
        screen_puts(4,slotNum+HOSTS_START_Y,ATTRIBUTE_NORMAL, hostSlots[slotNum][0] != 0x00 ? (char *)hostSlots[slotNum] : text_empty);
    }
}

// Since 'deviceSlots' is a global, do we need to access the input parameter at all?
// Maybe globals.h wasn't supposed in be part of screen? I needed it for something..
void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *dslot, bool *e)
{
  unsigned char slotNum;
  unsigned char dinfo[6];

  // Get full filename for device slot 8
  if (deviceSlots[7].file[0] != 0x00)
    io_get_filename_for_device_slot(7, fn);

  // Display device slots
  for (slotNum = 0; slotNum < NUM_DEVICE_SLOTS; slotNum++)
  {
    dinfo[1] = 0x20;
    dinfo[2] = (slotNum == 7 && strstr(fn, ".cas") != NULL) ? 'C' : (0x31 + slotNum);
    dinfo[4] = 0x20;
    dinfo[5] = 0x00;

    if (deviceSlots[slotNum].file[0] != 0x00)
    {
      dinfo[0] = deviceSlots[slotNum].hostSlot + 0x31;
      dinfo[3] = (deviceSlots[slotNum].mode == 0x02 ? 'W' : 'R');
    }
    else
    {
      dinfo[0] = 0x20;
      dinfo[3] = 0x20;
    }

    screen_puts(0, slotNum + y, ATTRIBUTE_BOLD, dinfo);

    screen_puts(4, slotNum + y, ATTRIBUTE_BOLD, deviceSlots[slotNum].file[0] != 0x00 ? (char *)deviceSlots[slotNum].file : text_empty);
  }
}

void screen_hosts_and_devices_devices_clear_all(void)
{
    screen_clear_line(11);
    screen_puts(0, 11, ATTRIBUTE_NORMAL, "EJECTING ALL.. WAIT");
}

void screen_hosts_and_devices_clear_host_slot(unsigned char i)
{
    // nothing to do, edit_line handles clearing correct space on screen, and doesn't touch the list numbers
}

void screen_hosts_and_devices_edit_host_slot(unsigned char i)
{
    // nothing to do, edit_line handles clearing correct space on screen, and doesn't touch the list numbers
}

void screen_hosts_and_devices_host_slot_empty(unsigned char hs)
{
    // When this gets called it seems like the cursor is right where we want it to be.
    // so no need to move to a position first.
    screen_puts(4, HOSTS_START_Y+hs, ATTRIBUTE_BOLD, text_empty);
}

void screen_hosts_and_devices_long_filename(char *f)
{
}

void screen_destination_host_slot(char *h, char *p)
{
    screen_clear();
    screen_puts(0,22,ATTRIBUTE_NORMAL, "[1-8] Slot [RETURN] Select [ESC] Abort");

    screen_puts(0, 18, ATTRIBUTE_NORMAL, h);
    screen_puts(0, 19, ATTRIBUTE_NORMAL, p);

    screen_puts(0, 0, ATTRIBUTE_HEADER, "COPY TO HOST SLOT");
    bar_set(HOSTS_START_Y,1,NUM_HOST_SLOTS,0);
}

void screen_destination_host_slot_choose(void)
{

}

void screen_perform_copy(char *sh, char *p, char *dh, char *dp)
{
    screen_clear();
    bar_clear(false);
    screen_puts(0, 0, ATTRIBUTE_HEADER, "COPYING, PLEASE WAIT");
}

void screen_connect_wifi(NetConfig *nc)
{
    screen_clear();
    bar_clear(false);

    screen_puts(0, 0, ATTRIBUTE_HEADER, "WELCOME TO FUJINET! CONNECTING TO NET");
    screen_puts(2, 3, ATTRIBUTE_BOLD, nc->ssid);
    bar_set(3, 1, 1, 0);
}

void screen_clear_line(unsigned char y)
{
    union REGS r;

    /* Set cursor position */
    r.h.ah = 0x02;
    r.h.bh = 0;
    r.h.dh = y;
    r.h.dl = 0;
    int86(0x10,&r,&r);

    /* Plot spaces with zero attributes */
    r.h.ah = 0x09;
    r.h.al = 0x20; // Blank character.
    r.h.bh = 0;
    r.h.bl = 0x00;
    r.x.cx = screen_cols;
    int86(0x10,&r,&r);
}

/**
 * @brief Initialize screen, and set up screen globals.
 */
void screen_init(void)
{
    union REGS r;

    // Use BIOS to return video mode characteristics.
    r.h.ah = 0x0f;
    int86(0x10,&r,&r);
    screen_cols = r.h.ah;
    screen_mode = r.h.al;

    // Go ahead and set video segment for the moments we need direct video access.
    video = screen_get_video_segment_address();
    screen_clear();
}

#endif /* __WATCOMC__ */
