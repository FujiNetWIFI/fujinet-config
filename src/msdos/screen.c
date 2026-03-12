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

void screen_set_wifi_display_ssid(char n, SSIDInfo *s)
{
    /* Column 1: one cell inside the left box border */
    screen_puts(1, n + NETWORKS_START_Y, ATTRIBUTE_NORMAL, (char *)s->ssid);
    screen_set_wifi_print_rssi(s, n);
}

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

void screen_set_wifi_custom(void)
{
    /* Row 23: between the network box (bottom border row 22) and status bar (row 24) */
    screen_clear_line(23);
    screen_puts_center(23, ATTRIBUTE_NORMAL, "Enter network name and press [ENTER]");
}

void screen_set_wifi_password(void)
{
    static const char blank[65] = "                                                                ";
    unsigned char bx = (screen_cols - (64 + 2)) / 2;
    screen_putc(bx,      22, ATTRIBUTE_NORMAL, '[');
    screen_puts(bx + 1,  22, ATTRIBUTE_NORMAL, (const char *)blank);
    screen_putc(bx + 65, 22, ATTRIBUTE_NORMAL, ']');
    screen_clear_line(23);
    screen_puts_center(23, ATTRIBUTE_NORMAL, "Enter password and press [ENTER]");
}

/*
 * Display the 'info' screen
 */
void screen_show_info_extended(bool printerEnabled, AdapterConfigExtended* acx)
{
    unsigned char x = (screen_cols - 40) / 2;
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

void screen_select_slot(char *e)
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
    sprintf((char *)tmp, "Date: %04u-%02u-%02u  %02u:%02u:%02u",
            (unsigned)(1970 + i->year), i->month, i->day, i->hour, i->minute, i->second);
    screen_puts(1, 16, ATTRIBUTE_NORMAL, (const char *)tmp);

    if (i->size >= 1024UL)
      sprintf((char *)tmp, "Size: %lu K", i->size >> 10);
    else
      sprintf((char *)tmp, "Size: %lu bytes", i->size);
    screen_puts(1, 17, ATTRIBUTE_NORMAL, (const char *)tmp);
  }

  screen_hosts_and_devices_device_slots(DEVICES_START_MOUNT_Y, &deviceSlots, &deviceEnabled[0]);

  bar_set(DEVICES_START_MOUNT_Y, 1, NUM_DEVICE_SLOTS, 0);
}

void screen_select_slot_mode(void)
{
    screen_clear_line(21);
    screen_puts_center(22,ATTRIBUTE_NORMAL,"[ENTER] Read Only [W] Read/Write [ESC] Abort");
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

void screen_select_file_display(char *p, char *f)
{
    unsigned char i;

    // Host — row 2, inside info box
    screen_puts(1, 2, ATTRIBUTE_HEADER, "Host: ");
    screen_puts(7, 2, ATTRIBUTE_BOLD, selected_host_name);

    // Filter — row 3
    screen_puts(1, 3, ATTRIBUTE_HEADER, "Fltr: ");
    screen_puts(7, 3, ATTRIBUTE_BOLD, f);

    // Path — row 4 (clear first so a shorter new path doesn't leave old text behind)
    screen_clear_line(4);
    screen_putc(0, 4, ATTRIBUTE_HEADER, BOX_V);
    screen_putc(screen_cols - 1, 4, ATTRIBUTE_HEADER, BOX_V);
    screen_puts(1, 4, ATTRIBUTE_HEADER, "Path: ");
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

void screen_select_file_display_long_filename(char *e)
{
    /* Row 24 is the status bar on MS-DOS — do not overwrite it. */
    (void)e;
}

void screen_select_file_clear_long_filename(void)
{
    /* Row 24 is the status bar on MS-DOS — do not clear it. */
}

void screen_select_file_filter(void)
{
    // No need to display anything additional, we're already showing the filter on the screen.
}

void screen_select_file_next(void)
{
    sf_draw_file_bot_border(dir_eof == false);
    if (pos == 0)
        sf_draw_file_top_border(false);
}

void screen_select_file_prev(void)
{
    sf_draw_file_top_border(pos > 0);
    if (dir_eof == true)
        sf_draw_file_bot_border(false);
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
        screen_puts(1,FILES_START_Y+y,ATTRIBUTE_BOLD, "/");
    }
    else if (e[0]=='=')
    {
        screen_puts(1,FILES_START_Y+y,ATTRIBUTE_BOLD,"=");
    }

    screen_puts(4, FILES_START_Y + y, ATTRIBUTE_NORMAL, e);

}

void screen_select_file_choose(char visibleEntries)
{
    bar_set(FILES_START_Y,1,visibleEntries,0);
    _visibleEntries = visibleEntries;
}

void screen_select_file_new_type(void)
{
}

void screen_select_file_new_size(unsigned char k)
{
    unsigned char x = (screen_cols - 40) / 2;

    screen_clear_line(20);
    screen_clear_line(21);

    screen_puts(x, 20, ATTRIBUTE_BOLD, "[1] 360K [2] 720K [3] 1.2MB [4] 1.44MB");
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
        tstart = x + (w - tlen) / 2;
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

void screen_error(const char *msg)
{
    screen_clear_line(24);
    screen_puts_center(24, ATTRIBUTE_BOLD, msg);
}

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

// Show the keys that are applicable when we are on the Hosts portion of the screen.
void screen_hosts_and_devices_hosts(void)
{
    if (screen_cols <= 40)
        screen_status("[1-8] [E]dit [RET] Browse [TAB] Drives");
    else
        screen_status("[1-8] [E]dit  [RETURN] Browse  [L]obby  [C]onfig  [TAB] Drives  [ESC] Exit");

    bar_set(HOSTS_START_Y, 1, NUM_HOST_SLOTS, selected_host_slot);
}

// Show the keys that are applicable when we are on the Devices portion of the screen.
void screen_hosts_and_devices_devices(void)
{
    if (screen_cols <= 40)
        screen_status("[1-8] [E]ject [Del] Clear [TAB] Hosts");
    else
        screen_status("[1-8] [E]ject  [Del] Clear  [L]obby  [TAB] Hosts  [R]ead  [W]rite  [C]onfig");

    bar_set(DEVICES_START_Y, 1, NUM_DEVICE_SLOTS, selected_device_slot);
}

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

// Since 'deviceSlots' is a global, do we need to access the input parameter at all?
// Maybe globals.h wasn't supposed in be part of screen? I needed it for something..
void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *dslot, bool *e)
{
  unsigned char slotNum;
  unsigned char dinfo[6];

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

    screen_puts(1, slotNum + y, ATTRIBUTE_BOLD, (const char *)dinfo);

    screen_puts(6, slotNum + y, ATTRIBUTE_NORMAL, deviceSlots[slotNum].file[0] != 0x00 ? (char *)deviceSlots[slotNum].file : text_empty);
  }
}

void screen_hosts_and_devices_devices_clear_all(void)
{
    screen_clear_line(11);
    screen_puts(0, 11, ATTRIBUTE_NORMAL, "EJECTING ALL.. WAIT");
}

void screen_hosts_and_devices_clear_host_slot(unsigned char i)
{
    static char spaces[33];
    memset((void *)spaces, ' ', 32);
    spaces[32] = 0;
    screen_puts(5, HOSTS_START_Y + i, ATTRIBUTE_NORMAL, (const char *)spaces);
}

void screen_hosts_and_devices_edit_host_slot(unsigned char i)
{
    bar_clear(false);
}

void screen_hosts_and_devices_host_slot_empty(unsigned char hs)
{
    // When this gets called it seems like the cursor is right where we want it to be.
    // so no need to move to a position first.
    screen_puts(5, HOSTS_START_Y+hs, ATTRIBUTE_BOLD, text_empty);
}

void screen_hosts_and_devices_long_filename(char *f)
{
}

void screen_destination_host_slot(char *h, char *p)
{
    screen_clear();
    screen_header("Copy to Host Slot");
    screen_draw_box_titled(0, 1, screen_cols, 10, ATTRIBUTE_HEADER, true, "Host Slots");

    screen_puts(1, 18, ATTRIBUTE_BOLD,   h);
    screen_puts(1, 19, ATTRIBUTE_NORMAL, p);

    screen_status("[1-8] Slot  [ENTER] Select  [ESC] Abort");
    bar_set(HOSTS_START_Y,1,NUM_HOST_SLOTS,0);
}

void screen_destination_host_slot_choose(void)
{

}

void screen_perform_copy(char *sh, char *p, char *dh, char *dp)
{
    screen_clear();
    bar_clear(false);
    screen_header("Copying File");
    screen_puts_center(12, ATTRIBUTE_BOLD, "Please wait...");
}

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

void screen_end(void)
{
    cursor(true);
}

#endif /* BUILD_MSDOS */
