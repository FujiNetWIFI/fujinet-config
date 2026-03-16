#ifdef BUILD_MSDOS

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
#include "globals.h"
#include "screen.h"
#include "cursor.h"
#include "io.h"
#include "../system.h"

/* CP437 single-line box drawing characters */
#define BOX_TL   '\xDA'   /* top-left corner    ┌ */
#define BOX_TR   '\xBF'   /* top-right corner   ┐ */
#define BOX_BL   '\xC0'   /* bottom-left corner └ */
#define BOX_BR   '\xD9'   /* bottom-right corner┘ */
#define BOX_H    '\xC4'   /* horizontal bar     ─ */
#define BOX_V    '\xB3'   /* vertical bar       │ */

/**
 * @brief screen globals
 */
unsigned char far *video = NULL;
unsigned char screen_cols = 80;
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
    static union REGS regs;
    unsigned char video_mode=0;

    regs.h.ah = 0x0F; // Get video mode
    int86(0x10,(union REGS *)&regs,(union REGS *)&regs);

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
    static union REGS r;

    /* Set cursor position */
    r.h.ah = 0x02;
    r.h.bh = 0;
    r.h.dh = y;
    r.h.dl = x;
    int86(0x10,(union REGS *)&r,(union REGS *)&r);

    r.h.ah = 0x09;
    r.h.al = c;
    r.h.bh = 0;
    r.h.bl = a;
    r.x.cx = 1;
    int86(0x10,(union REGS *)&r,(union REGS *)&r);
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
    screen_header("FujiNet Config");
    screen_puts_center(12, ATTRIBUTE_BOLD, "Mounting all slots...");
}

/**
 * @brief Display the WiFi network scan screen with the adapter MAC address.
 * @param acx Pointer to extended adapter configuration containing the MAC address.
 */
void screen_set_wifi_extended(AdapterConfigExtended *acx)
{
    static char tmp[40];

    sprintf((char *)tmp, "MAC: %02X:%02X:%02X:%02X:%02X:%02X",
        acx->macAddress[0], acx->macAddress[1], acx->macAddress[2],
        acx->macAddress[3], acx->macAddress[4], acx->macAddress[5]);

    screen_clear();
    screen_header("Welcome to FujiNet!");
    screen_puts_center(1, ATTRIBUTE_BOLD, (const char *)tmp);

    /* Box around network list: top border row 2, inner rows 3-21, bottom border row 22.
       Fits MAX_WIFI_NETWORKS (18) entries + 1 "Enter a specific SSID" = 19 inner rows. */
    screen_draw_box_titled(0, 2, screen_cols, 21, ATTRIBUTE_HEADER, false, "Available Networks");

    screen_status("Scanning networks...");
}

/**
 * @brief Print a 3-character signal-strength indicator for a WiFi network entry.
 * @param s Pointer to the SSIDInfo containing the RSSI value.
 * @param i Row index within the network list (added to NETWORKS_START_Y).
 */
void screen_set_wifi_print_rssi(SSIDInfo *s, unsigned char i)
{
    /* 3-char RSSI field just inside the right box border (screen_cols-4 to screen_cols-2).
     * \xB2 = dense (strong), \xB1 = medium, \xB0 = light (weak) */
    static char out[4];
    unsigned char x = screen_cols - 4;

    out[0] = 0x20; out[1] = 0x20; out[2] = 0x20; out[3] = 0x00;

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

    screen_puts(x, i + NETWORKS_START_Y, ATTRIBUTE_BOLD, (const char *)out);
}

/**
 * @brief Display one SSID entry and its RSSI indicator in the network list.
 * @param n Row index within the network list (0-based, added to NETWORKS_START_Y).
 * @param s Pointer to the SSIDInfo containing the SSID name and RSSI.
 */
void screen_set_wifi_display_ssid(char n, SSIDInfo *s)
{
    /* Column 1: one cell inside the left box border */
    screen_puts(1, n + NETWORKS_START_Y, ATTRIBUTE_NORMAL, (char *)s->ssid);
    screen_set_wifi_print_rssi(s, n);
}

/**
 * @brief Render the "Enter a specific SSID" entry at the bottom of the network list and initialise the bar.
 * @param nn Total number of scanned networks; the extra entry is placed at row NETWORKS_START_Y + nn.
 */
void screen_set_wifi_sel_net(unsigned char nn)
{
    unsigned char y = NETWORKS_START_Y + numNetworks;

    /* Clear the line inside the box, then restore the box borders */
    screen_clear_line(y);
    screen_putc(0,              y, ATTRIBUTE_HEADER, BOX_V);
    screen_putc(screen_cols-1,  y, ATTRIBUTE_HEADER, BOX_V);
    screen_puts(1, y, ATTRIBUTE_NORMAL, "<Enter a specific SSID>");

    screen_status("[ENTER] Select  [ESC] Re-scan  [S] Skip");

    bar_set(NETWORKS_START_Y, 1, nn + 1, 0);
}

/**
 * @brief Display the prompt for manual SSID entry on row 23.
 */
void screen_set_wifi_custom(void)
{
    static const char blank[33] = "                                ";
    unsigned char bx = (screen_cols - (32 + 2)) / 2;
    screen_clear_line(23);
    screen_putc(bx,      23, ATTRIBUTE_NORMAL, '[');
    screen_puts(bx + 1,  23, ATTRIBUTE_NORMAL, (const char *)blank);
    screen_putc(bx + 33, 23, ATTRIBUTE_NORMAL, ']');
    screen_status("Enter network name and press [ENTER]");
}

/**
 * @brief Display the password entry bracket on row 23.
 */
void screen_set_wifi_password(void)
{
    static const char blank[65] = "                                                                ";
    unsigned char bx = (screen_cols - (64 + 2)) / 2;
    screen_clear_line(23);
    screen_putc(bx,      23, ATTRIBUTE_NORMAL, '[');
    screen_puts(bx + 1,  23, ATTRIBUTE_NORMAL, (const char *)blank);
    screen_putc(bx + 65, 23, ATTRIBUTE_NORMAL, ']');
    screen_status("Enter password and press [ENTER]");
}

/**
 * @brief Display the network information screen with IP, MAC, and firmware details.
 * @param printerEnabled Unused on MS-DOS; present for cross-platform API compatibility.
 * @param acx            Pointer to extended adapter configuration with network details.
 */
void screen_show_info_extended(bool printerEnabled, AdapterConfigExtended* acx)
{
    unsigned char x = (screen_cols - 44) / 2;
    char tmp[80];

    screen_clear();

    screen_header("FujiNet Config");
    screen_draw_box_titled(x, 5, 44, 13, ATTRIBUTE_HEADER, true, "Network Info");
    screen_status("[R]econnect  Change [S]SID  [Any key] Return");
    screen_puts(x+5, 7, ATTRIBUTE_NORMAL,"      SSID:");
    screen_puts(x+5, 8, ATTRIBUTE_NORMAL,"  Hostname:");
    screen_puts(x+5, 9, ATTRIBUTE_NORMAL,"IP Address:");
    screen_puts(x+5, 10, ATTRIBUTE_NORMAL,"   Gateway:");
    screen_puts(x+5, 11, ATTRIBUTE_NORMAL,"       DNS:");
    screen_puts(x+5, 12, ATTRIBUTE_NORMAL,"   Netmask:");
    screen_puts(x+5, 13, ATTRIBUTE_NORMAL,"       MAC:");
    screen_puts(x+5, 14, ATTRIBUTE_NORMAL,"     BSSID:");
    screen_puts(x+5, 15, ATTRIBUTE_NORMAL,"   Version:");

    screen_puts(x+17, 7, ATTRIBUTE_BOLD,acx->ssid);
    screen_puts(x+17, 8, ATTRIBUTE_BOLD,acx->hostname);

    sprintf((char *)tmp,"%03u.%03u.%03u.%03u",
	    acx->localIP[0],
	    acx->localIP[1],
	    acx->localIP[2],
	    acx->localIP[3]);
    screen_puts(x+17, 9, ATTRIBUTE_BOLD,(const char *)tmp);

    sprintf((char *)tmp,"%03u.%03u.%03u.%03u",
	    acx->gateway[0],
	    acx->gateway[1],
	    acx->gateway[2],
	    acx->gateway[3]);
    screen_puts(x+17, 10, ATTRIBUTE_BOLD,(const char *)tmp);

    sprintf((char *)tmp,"%03u.%03u.%03u.%03u",
	    acx->dnsIP[0],
	    acx->dnsIP[1],
	    acx->dnsIP[2],
	    acx->dnsIP[3]);
    screen_puts(x+17, 11, ATTRIBUTE_BOLD,(const char *)tmp);

    sprintf((char *)tmp,"%03u.%03u.%03u.%03u",
	    acx->netmask[0],
	    acx->netmask[1],
	    acx->netmask[2],
	    acx->netmask[3]);
    screen_puts(x+17, 12, ATTRIBUTE_BOLD,(const char *)tmp);

    sprintf((char *)tmp,"%02X:%02X:%02X:%02X:%02X:%02X",
	    acx->macAddress[0],
	    acx->macAddress[1],
	    acx->macAddress[2],
	    acx->macAddress[3],
	    acx->macAddress[4],
	    acx->macAddress[5]);
    screen_puts(x+17, 13, ATTRIBUTE_BOLD,(const char *)tmp);

    sprintf((char *)tmp,"%02X:%02X:%02X:%02X:%02X:%02X",
	    acx->bssid[0],
	    acx->bssid[1],
	    acx->bssid[2],
	    acx->bssid[3],
	    acx->bssid[4],
	    acx->bssid[5]);
    screen_puts(x+17, 14, ATTRIBUTE_BOLD,(const char *)tmp);
    screen_puts(x+17, 15, ATTRIBUTE_BOLD,acx->fn_version);
}

/* Matches rs232Fuji::set_additional_direntry_details() packed layout:
     dirEntryTimestamp (6) + uint32_t size (4) + uint8_t flags (1) + uint8_t mediatype (1) = 12 bytes
   followed immediately by the null-terminated filename. */
#pragma pack(1)
struct _file_info {
    unsigned char  year;      /* offset  0: years since 1970 (firmware subtracts 70) */
    unsigned char  month;     /* offset  1                                            */
    unsigned char  day;       /* offset  2                                            */
    unsigned char  hour;      /* offset  3                                            */
    unsigned char  minute;    /* offset  4                                            */
    unsigned char  second;    /* offset  5                                            */
    unsigned long  size;      /* offset  6: uint32_t little-endian file size          */
    unsigned char  flags;     /* offset 10: DET_FF_DIR=0x01, DET_FF_TRUNC=0x02       */
    unsigned char  mediatype; /* offset 11                                            */
    /* filename (null-terminated) follows at offset 12 = sizeof(struct _file_info)   */
};
#pragma pack()

/**
 * @brief Display the drive-slot selection screen with file details for the selected entry.
 * @param e Pointer to a packed _file_info struct followed by the null-terminated filename.
 */
void screen_select_slot(const char *e)
{
  struct _file_info *i = (struct _file_info *)e;
  static char fullpath[256];
  static char tmp[84];

  screen_clear();

  screen_header("Mount to Drive Slot");
  screen_draw_box_titled(0, 1, screen_cols, 12, ATTRIBUTE_HEADER, false, "Drive Slots");
  screen_status("[ENTER] Read Only  [W] Read/Write  [E]ject  [ESC] Abort");

  if (create == false)
  {
    /* Filename follows struct at offset 13 (= sizeof struct _file_info) */
    sprintf((char *)fullpath, "%s%s", path, (char *)(i + 1));
    fullpath[screen_cols - 2] = '\0';

    screen_draw_box_titled(0, 14, screen_cols, 5, ATTRIBUTE_HEADER, false, "File Details");

    screen_puts(1, 15, ATTRIBUTE_BOLD, (const char *)fullpath);

    /* Year is years-since-1970 */
    screen_puts(1, 16, ATTRIBUTE_NORMAL, "Date: ");
    sprintf((char *)tmp, "%04u-%02u-%02u  %02u:%02u:%02u",
            (unsigned)(1970 + i->year), i->month, i->day, i->hour, i->minute, i->second);
    screen_puts(7, 16, ATTRIBUTE_BOLD, (const char *)tmp);

    screen_puts(1, 17, ATTRIBUTE_NORMAL, "Size: ");
    if (i->size >= 1024UL)
      sprintf((char *)tmp, "%lu K", i->size >> 10);
    else
      sprintf((char *)tmp, "%lu bytes", i->size);
    screen_puts(7, 17, ATTRIBUTE_BOLD, (const char *)tmp);
  }

  screen_hosts_and_devices_device_slots(DEVICES_START_MOUNT_Y, &deviceSlots, &deviceEnabled[0]);

  bar_set(DEVICES_START_MOUNT_Y, 1, NUM_DEVICE_SLOTS, 0);
}

/**
 * @brief Display the mount-mode prompt on rows 21-22.
 */
void screen_select_slot_mode(void)
{
    screen_clear_line(21);
    screen_puts_center(22,ATTRIBUTE_NORMAL,"[ENTER] Read Only [W] Read/Write [ESC] Abort");
}

/**
 * @brief Select-slot choose update stub — no additional display needed on MS-DOS.
 */
void screen_select_slot_choose(void)
{
}

/**
 * @brief Redraw a single device slot row to show it as ejected (empty).
 * @param y  Base row of the device slot list.
 * @param ds Device slot index (0-based) to redraw.
 */
static void eject_draw_slot(unsigned char y, unsigned char ds)
{
    static unsigned char dinfo[9];
    static char padded[71];
    unsigned char row = y + ds;
    unsigned char w   = screen_cols - 10;
    char dl;

    system_refresh_drive_letters();
    dl = deviceDriveLetters[ds];

    dinfo[0] = 0x20; dinfo[1] = 0x20;
    dinfo[2] = 0x31 + ds;
    dinfo[3] = 0x20; dinfo[4] = 0x20;
    dinfo[5] = (dl ? (unsigned char)dl : ' ');
    dinfo[6] = (dl ? ':' : ' ');
    dinfo[7] = 0x20; dinfo[8] = 0x00;

    /* Draw unhighlighted state first so bar_set captures correct attrs */
    screen_puts(1, row, ATTRIBUTE_BOLD,   (const char *)dinfo);

    memcpy((void *)padded, text_empty, 5);
    memset((void *)(padded + 5), ' ', w - 5);
    padded[w] = 0;
    screen_puts(9, row, ATTRIBUTE_NORMAL, (const char *)padded);

    /* Now bar_set saves the correct BOLD/NORMAL attrs before highlighting */
    bar_set(y, 1, NUM_DEVICE_SLOTS, ds);
}

/**
 * @brief Redraw a device slot in the mount screen as ejected.
 * @param ds Device slot index (0-based).
 */
void screen_select_slot_eject(uint8_t ds)
{
    eject_draw_slot(DEVICES_START_MOUNT_Y, ds);
}

/**
 * @brief Redraw a device slot as ejected in whichever list is currently active.
 * @param ds Device slot index (0-based).
 */
void screen_hosts_and_devices_eject(uint8_t ds)
{
    unsigned char y = mounting ? DEVICES_START_MOUNT_Y : DEVICES_START_Y;
    eject_draw_slot(y, ds);
}

/**
 * @brief EOS directory build screen stub — no-op on MS-DOS.
 */
void screen_select_slot_build_eos_directory(void)
{
}

/**
 * @brief EOS directory label screen stub — no-op on MS-DOS.
 */
void screen_select_slot_build_eos_directory_label(void)
{
}

/**
 * @brief EOS directory creation progress screen stub — no-op on MS-DOS.
 */
void screen_select_slot_build_eos_directory_creating(void)
{
}

/* CP437 single-line box drawing characters */
#define BOX_TL   '\xDA'   /* top-left corner    ┌ */
#define BOX_TR   '\xBF'   /* top-right corner   ┐ */
#define BOX_BL   '\xC0'   /* bottom-left corner └ */
#define BOX_BR   '\xD9'   /* bottom-right corner┘ */
#define BOX_H    '\xC4'   /* horizontal bar     ─ */
#define BOX_V    '\xB3'   /* vertical bar       │ */

/**
 * @brief Draw/redraw the top border of the file list box, optionally with a
 *        "[PgUp]" label at the left end of the horizontal rule.
 */
static void sf_draw_file_top_border(bool pgup)
{
    unsigned char row = FILES_START_Y - 1;
    unsigned char a = ATTRIBUTE_HEADER;
    unsigned char i;

    screen_putc(0, row, a, BOX_TL);
    if (pgup)
    {
        const char *lbl = "[PgUp]";
        for (i = 0; lbl[i]; i++) screen_putc(1 + i, row, a, lbl[i]);
        for (i = 1 + 6; i < screen_cols - 1; i++) screen_putc(i, row, a, BOX_H);
    }
    else
    {
        for (i = 1; i < screen_cols - 1; i++) screen_putc(i, row, a, BOX_H);
    }
    screen_putc(screen_cols - 1, row, a, BOX_TR);
}

/**
 * @brief Draw/redraw the bottom border of the file list box, optionally with
 *        a "[PgDn]" label at the left end of the horizontal rule.
 */
static void sf_draw_file_bot_border(bool pgdn)
{
    unsigned char row = FILES_START_Y + ENTRIES_PER_PAGE;
    unsigned char a = ATTRIBUTE_HEADER;
    unsigned char i;

    screen_putc(0, row, a, BOX_BL);
    if (pgdn)
    {
        const char *lbl = "[PgDn]";
        for (i = 0; lbl[i]; i++) screen_putc(1 + i, row, a, lbl[i]);
        for (i = 1 + 6; i < screen_cols - 1; i++) screen_putc(i, row, a, BOX_H);
    }
    else
    {
        for (i = 1; i < screen_cols - 1; i++) screen_putc(i, row, a, BOX_H);
    }
    screen_putc(screen_cols - 1, row, a, BOX_BR);
}

/**
 * @brief Draw the full file-browser screen including header, info box, and file list box.
 */
void screen_select_file(void)
{
    screen_clear();
    bar_clear(false);

    screen_header("Disk Images");

    screen_draw_box(0, 1, screen_cols, 5, ATTRIBUTE_HEADER, false);

    /* File list box: top border row 7 (FILES_START_Y-1), bottom border row 23. No shadow (row 24 = status bar). */
    screen_draw_box(0, FILES_START_Y - 1, screen_cols, ENTRIES_PER_PAGE + 2, ATTRIBUTE_HEADER, false);

    if (copy_mode == false)
        screen_status("[BKSP] Up Dir  [N]ew  [F]ilter  [C]opy  [ENTER] Choose  [ESC] Abort");
    else
        screen_status("[BKSP] Up Dir  [N]ew  [F]ilter  [C] Do Copy!  [ENTER] Choose  [ESC] Abort");
}

/**
 * @brief Refresh the host, filter, and path fields and clear the file list area.
 * @param p Current directory path string.
 * @param f Current filename filter string.
 */
void screen_select_file_display(char *p, char *f)
{
    unsigned char i;

    // Host — row 2, inside info box
    screen_puts(1, 2, ATTRIBUTE_NORMAL, "Host: ");
    screen_puts(7, 2, ATTRIBUTE_BOLD, selected_host_name);

    // Filter — row 3
    screen_puts(1, 3, ATTRIBUTE_NORMAL, "Fltr: ");
    screen_puts(7, 3, ATTRIBUTE_BOLD, f);

    // Path — row 4 (clear first so a shorter new path doesn't leave old text behind)
    screen_clear_line(4);
    screen_putc(0, 4, ATTRIBUTE_HEADER, BOX_V);
    screen_putc(screen_cols - 1, 4, ATTRIBUTE_HEADER, BOX_V);
    screen_puts(1, 4, ATTRIBUTE_NORMAL, "Path: ");
    screen_puts(7, 4, ATTRIBUTE_BOLD, p);

    // Clear out the file entry area, then restore the box's vertical borders
    for (i = FILES_START_Y; i < FILES_START_Y + ENTRIES_PER_PAGE; i++)
    {
        screen_clear_line(i);
        screen_putc(0,              i, ATTRIBUTE_HEADER, BOX_V);
        screen_putc(screen_cols-1,  i, ATTRIBUTE_HEADER, BOX_V);
    }

    // Restore file list box borders (removes any stale PgUp/PgDn indicators)
    sf_draw_file_top_border(false);
    sf_draw_file_bot_border(false);
}

/**
 * @brief Long-filename display stub — row 24 is the status bar on MS-DOS; no-op.
 * @param e Unused entry pointer.
 */
void screen_select_file_display_long_filename(const char *e)
{
    /* Row 24 is the status bar on MS-DOS — do not overwrite it. */
    (void)e;
}

/**
 * @brief Long-filename clear stub — row 24 is the status bar on MS-DOS; no-op.
 */
void screen_select_file_clear_long_filename(void)
{
    /* Row 24 is the status bar on MS-DOS — do not clear it. */
}

/**
 * @brief Filter display stub — filter is shown inline in the info box; no additional display needed.
 */
void screen_select_file_filter(void)
{
    // No need to display anything additional, we're already showing the filter on the screen.
}

/**
 * @brief Update the file list box borders after advancing to the next page.
 */
void screen_select_file_next(void)
{
    sf_draw_file_bot_border(dir_eof == false);
    if (pos == 0)
        sf_draw_file_top_border(false);
}

/**
 * @brief Update the file list box borders after going back to the previous page.
 */
void screen_select_file_prev(void)
{
    sf_draw_file_top_border(pos > 0);
    if (dir_eof == true)
        sf_draw_file_bot_border(false);
}

/**
 * @brief Display a single directory entry in the file list.
 * @param y         Row offset within the file list (0-based, added to FILES_START_Y).
 * @param e         Null-terminated entry name string.
 * @param entryType Entry type code (ENTRY_TYPE_FOLDER, ENTRY_TYPE_LINK, etc.).
 */
void screen_select_file_display_entry(uint8_t y, const char *e, uint16_t entryType)
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
        screen_puts(1,FILES_START_Y+y,ATTRIBUTE_BOLD, "/");
    }
    else if (e[0]=='=')
    {
        screen_puts(1,FILES_START_Y+y,ATTRIBUTE_BOLD,"=");
    }

    screen_puts(4, FILES_START_Y + y, ATTRIBUTE_NORMAL, e);

}

/**
 * @brief Initialise the highlight bar over the file list.
 * @param visibleEntries Number of entries currently visible in the list.
 */
void screen_select_file_choose(char visibleEntries)
{
    bar_set(FILES_START_Y,1,visibleEntries,0);
    _visibleEntries = visibleEntries;
}

/**
 * @brief New-file type selection screen stub — type is fixed on MS-DOS; no-op.
 */
void screen_select_file_new_type(void)
{
}

/**
 * @brief Display the floppy disk size selection prompt.
 * @param k Unused key code; included for cross-platform API compatibility.
 */
void screen_select_file_new_size(uint8_t k)
{
    screen_clear_line(NEW_PROMPT_Y);
    screen_clear_line(NEW_INPUT_Y);
    screen_puts_center(NEW_PROMPT_Y, ATTRIBUTE_BOLD, "[1] 360K  [2] 720K  [3] 1.2MB  [4] 1.44MB  [C]ustom");
}

/**
 * @brief Draw a centred bracket pair of width w on row y for text input display.
 * @param y Row on which to draw the brackets.
 * @param w Interior width in characters (bracket characters not included).
 */
static void screen_draw_input_brackets(unsigned char y, unsigned char w)
{
    static char brackets[NEW_NAME_WIDTH + 3]; /* large enough for either field */
    unsigned char bx = (screen_cols - (w + 2)) / 2;
    unsigned char i;

    brackets[0] = '[';
    for (i = 1; i <= w; i++) brackets[i] = ' ';
    brackets[w + 1] = ']';
    brackets[w + 2] = '\0';
    screen_puts(bx, y, ATTRIBUTE_NORMAL, (const char *)brackets);
}

/**
 * @brief Display the custom sector-count entry prompt and input brackets.
 */
void screen_select_file_new_custom(void)
{
    screen_clear_line(NEW_PROMPT_Y);
    screen_clear_line(NEW_INPUT_Y);
    screen_puts_center(NEW_PROMPT_Y, ATTRIBUTE_NORMAL, "Number of sectors:");
    screen_draw_input_brackets(NEW_INPUT_Y, NEW_SECTORS_WIDTH);
}

/**
 * @brief Display the new filename entry prompt and input brackets.
 */
void screen_select_file_new_name(void)
{
    screen_clear_line(NEW_PROMPT_Y);
    screen_clear_line(NEW_INPUT_Y);
    screen_puts_center(NEW_PROMPT_Y, ATTRIBUTE_NORMAL, "Filename:");
    screen_draw_input_brackets(NEW_INPUT_Y, NEW_NAME_WIDTH);
}

/**
 * @brief Display the "Creating file…" progress screen while the image is being written.
 */
void screen_select_file_new_creating(void)
{
    screen_clear();
    screen_header("New Disk Image");
    screen_puts_center(12, ATTRIBUTE_BOLD, "Creating file...");
}

/**
 * @brief Fill an entire screen line with spaces at the given attribute.
 */
void screen_fill_line(unsigned char y, unsigned char a)
{
    static union REGS r;
    r.h.ah = 0x02; r.h.bh = 0; r.h.dh = y; r.h.dl = 0;
    int86(0x10,(union REGS *)&r,(union REGS *)&r);
    r.h.ah = 0x09; r.h.al = 0x20; r.h.bh = 0; r.h.bl = a; r.x.cx = screen_cols;
    int86(0x10,(union REGS *)&r,(union REGS *)&r);
}

/**
 * @brief Draw a full-width title bar on line 0 (EDIT.EXE menu bar style).
 */
void screen_header(const char *title)
{
    screen_fill_line(0, ATTRIBUTE_HEADER);
    screen_puts_center(0, ATTRIBUTE_HEADER, title);
}

/**
 * @brief Draw a full-width status bar on line 24 (EDIT.EXE status bar style).
 */
void screen_status(const char *text)
{
    screen_fill_line(24, ATTRIBUTE_STATUS);
    screen_puts_center(24, ATTRIBUTE_STATUS, text);
}

/**
 * @brief Draw a box with optional drop shadow using CP437 line-drawing chars.
 * @param x     Left column
 * @param y     Top row
 * @param w     Width (including borders)
 * @param h     Height (including borders)
 * @param a     Attribute for the border
 * @param shadow true = draw a one-cell drop shadow to the right and bottom
 */
void screen_draw_box(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char a, bool shadow)
{
    unsigned char i;

    /* Top border */
    screen_putc(x,       y,   a, BOX_TL);
    for (i = 1; i < w-1; i++) screen_putc(x+i, y,   a, BOX_H);
    screen_putc(x+w-1,   y,   a, BOX_TR);

    /* Sides and interior fill */
    for (i = 1; i < h-1; i++)
    {
        screen_putc(x,     y+i, a, BOX_V);
        screen_putc(x+w-1, y+i, a, BOX_V);
    }

    /* Bottom border */
    screen_putc(x,       y+h-1, a, BOX_BL);
    for (i = 1; i < w-1; i++) screen_putc(x+i, y+h-1, a, BOX_H);
    screen_putc(x+w-1,   y+h-1, a, BOX_BR);

    /* Drop shadow: one column to the right, one row below */
    if (shadow)
    {
        for (i = 1; i < h; i++)
            screen_putc(x+w, y+i, ATTRIBUTE_SHADOW, 0xDB); /* full block */
        for (i = 1; i <= w; i++)
            screen_putc(x+i, y+h, ATTRIBUTE_SHADOW, 0xDB);
    }
}

/**
 * @brief Draw a box with a title centered in the top border.
 */
void screen_draw_box_titled(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char a, bool shadow, const char *title)
{
    unsigned char tlen, tstart, i;

    screen_draw_box(x, y, w, h, a, shadow);

    if (title && *title)
    {
        tlen   = (unsigned char)strlen(title) + 2; /* flanking spaces */
        tstart = x + (w >> 1) - (tlen >> 1);
        screen_putc(tstart - 1, y, a, BOX_H); /* overwrite the ┌ approach */
        screen_putc(tstart,     y, a, ' ');
        for (i = 0; title[i]; i++)
            screen_putc(tstart + 1 + i, y, a, title[i]);
        screen_putc(tstart + 1 + i, y, a, ' ');
    }
}

/**
 * @brief Clear the screen, filling with the desktop background attribute.
 */
void screen_clear(void)
{
    unsigned short i;
    for (i = 0; i < 4000; i += 2)
    {
        video[i]   = 0x20;             /* space character */
        video[i+1] = ATTRIBUTE_NORMAL; /* white on blue   */
    }
}

/**
 * @brief Display an error message centred on the status bar (row 24).
 * @param msg Null-terminated error message string.
 */
void screen_error(const char *msg)
{
    screen_clear_line(24);
    screen_puts_center(24, ATTRIBUTE_BOLD, msg);
}

/**
 * @brief Draw the complete hosts-and-devices main screen.
 * @param h Pointer to the host slot array.
 * @param d Pointer to the device slot array.
 * @param e Pointer to the device-enabled flags array.
 */
void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d, bool *e)
{
    screen_clear();
    bar_clear(false);

    screen_header("FujiNet Config");

    screen_draw_box_titled(0, 2, screen_cols, 10, ATTRIBUTE_HEADER, false, "HOST SLOTS");
    screen_draw_box_titled(0, 13, screen_cols, 10, ATTRIBUTE_HEADER, false, "DRIVE SLOTS");

    screen_hosts_and_devices_host_slots(&hostSlots[0]);
    screen_hosts_and_devices_device_slots(DEVICES_START_Y, &deviceSlots[0], NULL);
}

/**
 * @brief Update the status bar with host-panel key hints and reposition the bar on the host list.
 */
void screen_hosts_and_devices_hosts(void)
{
    if (screen_cols <= 40)
        screen_status("[1-8] [E]dit [RET] Browse [TAB] Drives");
    else
        screen_status("[1-8] [E]dit  [RETURN] Browse  [L]obby  [C]onfig  [TAB] Drives  [ESC] Exit");

    bar_set(HOSTS_START_Y, 1, NUM_HOST_SLOTS, selected_host_slot);
}

/**
 * @brief Update the status bar with device-panel key hints and reposition the bar on the device list.
 */
void screen_hosts_and_devices_devices(void)
{
    if (screen_cols <= 40)
        screen_status("[1-8] [E]ject [Del] Clear [TAB] Hosts");
    else
        screen_status("[1-8] [E]ject  [Del] Clear  [L]obby  [TAB] Hosts  [R]ead  [W]rite  [C]onfig");

    bar_set(DEVICES_START_Y, 1, NUM_DEVICE_SLOTS, selected_device_slot);
}

/**
 * @brief Render all host slot labels and names in the HOST SLOTS box.
 * @param h Pointer to the host slot array.
 */
void screen_hosts_and_devices_host_slots(HostSlot *h)
{
    unsigned char slotNum;

    for (slotNum = 0; slotNum < NUM_HOST_SLOTS; slotNum++)
    {
        char s[2] = {0x00,0x00};

        s[0] = slotNum + '1';

        screen_puts(2,slotNum+HOSTS_START_Y,ATTRIBUTE_BOLD, (const char *)s);
        screen_puts(5,slotNum+HOSTS_START_Y,ATTRIBUTE_NORMAL, hostSlots[slotNum][0] != 0x00 ? (char *)hostSlots[slotNum] : text_empty);
    }
}

/**
 * @brief Render all device slot rows starting at row y, including drive letters and filenames.
 * @param y     Base row for the first device slot entry.
 * @param dslot Pointer to the device slot array.
 * @param e     Pointer to per-slot enabled flags (may be NULL).
 */
void screen_hosts_and_devices_device_slots(uint8_t y, DeviceSlot *dslot, const bool *e)
{
  unsigned char slotNum;
  unsigned char dinfo[9];

  system_refresh_drive_letters();

  // Display device slots
  for (slotNum = 0; slotNum < NUM_DEVICE_SLOTS; slotNum++)
  {
    char dl;

    dinfo[1] = 0x20;
    dinfo[2] = 0x31 + slotNum;
    dinfo[4] = 0x20;

    dl = deviceDriveLetters[slotNum];
    dinfo[5] = (dl ? (unsigned char)dl : ' ');
    dinfo[6] = (dl ? ':' : ' ');

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

    dinfo[7] = 0x20;
    dinfo[8] = 0x00;

    screen_puts(1, slotNum + y, ATTRIBUTE_BOLD, (const char *)dinfo);

    {
      static char padded[71];
      const char *src = deviceSlots[slotNum].file[0] != 0x00 ? (char *)deviceSlots[slotNum].file : text_empty;
      unsigned char w = screen_cols - 10; /* cols 9..screen_cols-2, after 8-char prefix */
      unsigned char srclen = (unsigned char)strlen(src);
      if (srclen > w) srclen = w;
      memcpy(padded, src, srclen);
      memset(padded + srclen, ' ', w - srclen);
      padded[w] = 0;
      screen_puts(9, slotNum + y, ATTRIBUTE_NORMAL, (const char *)padded);
    }
  }
}

/**
 * @brief Display an "EJECTING ALL.. WAIT" message while clearing all device slots.
 */
void screen_hosts_and_devices_devices_clear_all(void)
{
    screen_clear_line(11);
    screen_puts(0, 11, ATTRIBUTE_NORMAL, "EJECTING ALL.. WAIT");
}

/**
 * @brief Clear the display of a single host slot name (overwrite with spaces).
 * @param i Host slot index (0-based).
 */
void screen_hosts_and_devices_clear_host_slot(uint_fast8_t i)
{
    static char spaces[33];
    memset((void *)spaces, ' ', 32);
    spaces[32] = 0;
    screen_puts(5, HOSTS_START_Y + i, ATTRIBUTE_NORMAL, (const char *)spaces);
}

/**
 * @brief Prepare the display for host slot inline editing by clearing the bar highlight.
 * @param i Host slot index (0-based).
 */
void screen_hosts_and_devices_edit_host_slot(uint_fast8_t i)
{
    bar_clear(false);
}

/**
 * @brief Display "Empty" in the given host slot after its contents have been cleared.
 * @param hs Host slot index (0-based).
 */
void screen_hosts_and_devices_host_slot_empty(uint_fast8_t hs)
{
    // When this gets called it seems like the cursor is right where we want it to be.
    // so no need to move to a position first.
    screen_puts(5, HOSTS_START_Y+hs, ATTRIBUTE_NORMAL, text_empty);
}

/**
 * @brief Long-filename display stub for the hosts-and-devices screen — no-op on MS-DOS.
 * @param f Unused filename pointer.
 */
void screen_hosts_and_devices_long_filename(const char *f)
{
}

/**
 * @brief Display the copy-destination host slot selection screen.
 * @param h Source host name string.
 * @param p Source file path string.
 */
void screen_destination_host_slot(char *h, char *p)
{
    static char label[84];
    screen_clear();
    screen_header("Copy to Host Slot");
    screen_draw_box_titled(0, 2, screen_cols, NUM_HOST_SLOTS + 2, ATTRIBUTE_HEADER, false, "Host Slots");

    screen_draw_box_titled(0, 13, screen_cols, 4, ATTRIBUTE_HEADER, false, "Disk Image Details");
    sprintf(label, "Host: %s", h);
    screen_puts(1, 14, ATTRIBUTE_BOLD,   (const char *)label);
    sprintf(label, "Path: %s", p);
    screen_puts(1, 15, ATTRIBUTE_NORMAL, (const char *)label);

    screen_status("[1-8] Slot  [ENTER] Select  [ESC] Abort");
}

/**
 * @brief Initialise the highlight bar on the destination host slot list.
 */
void screen_destination_host_slot_choose(void)
{
    bar_set(HOSTS_START_Y,1,NUM_HOST_SLOTS,0);
}

/**
 * @brief Display the copy-in-progress screen.
 * @param sh Source host name.
 * @param p  Source file path.
 * @param dh Destination host name.
 * @param dp Destination file path.
 */
void screen_perform_copy(char *sh, char *p, char *dh, char *dp)
{
    screen_clear();
    bar_clear(false);
    screen_header("Copying File");
    screen_puts_center(12, ATTRIBUTE_BOLD, "Please wait...");
}

/**
 * @brief Display the "Connecting to WiFi" progress screen.
 * @param nc Pointer to the NetConfig containing the SSID to display.
 */
void screen_connect_wifi(NetConfig *nc)
{
    unsigned char bx = (screen_cols - 36) / 2;

    screen_clear();
    bar_clear(false);

    screen_header("Connecting to WiFi");
    screen_draw_box_titled(bx, 8, 36, 5, ATTRIBUTE_HEADER, true, "Please Wait");
    screen_puts_center(10, ATTRIBUTE_NORMAL, "Connecting to:");
    screen_puts_center(11, ATTRIBUTE_BOLD,   nc->ssid);
    screen_status("[ESC] Abort");
}

/**
 * @brief Clear a single screen row by writing spaces with the normal background attribute.
 * @param y Row to clear (0-24).
 */
void screen_clear_line(unsigned char y)
{
    static union REGS r;

    /* Set cursor position */
    r.h.ah = 0x02;
    r.h.bh = 0;
    r.h.dh = y;
    r.h.dl = 0;
    int86(0x10,(union REGS *)&r,(union REGS *)&r);

    /* Plot spaces with desktop background attribute */
    r.h.ah = 0x09;
    r.h.al = 0x20; // Blank character.
    r.h.bh = 0;
    r.h.bl = ATTRIBUTE_NORMAL;
    r.x.cx = screen_cols;
    int86(0x10,(union REGS *)&r,(union REGS *)&r);
}

/**
 * @brief Initialize screen, and set up screen globals.
 */
void screen_init(void)
{
    static union REGS r;

    // Use BIOS to return video mode characteristics.
    r.h.ah = 0x0f;
    int86(0x10,(union REGS *)&r,(union REGS *)&r);
    screen_cols = r.h.ah;
    screen_mode = r.h.al;

    // Go ahead and set video segment for the moments we need direct video access.
    video = screen_get_video_segment_address();
    screen_clear();

    cursor(false);
}

/**
 * @brief Restore the hardware cursor on program exit.
 */
void screen_end(void)
{
    cursor(true);
}

#endif /* BUILD_MSDOS */
