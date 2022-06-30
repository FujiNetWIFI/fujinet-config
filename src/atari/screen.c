#ifdef BUILD_ATARI

/**
 * FujiNet Configuration Program: screen functions
 * The screen functions are a common set of graphics and text string functions to display information to the video output device.
 * These screen functions is comprised of two files; screen.c and screen.h.  Screen.c sets up the display dimensions, memory and
 * initializes the display as well as provide various text manipulations functions for proper display.  The screen.h file include
 * defines for various items such as keyboard codes, functions and screen memory addresses.
 *
 **/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <stdint.h>
#include <peekpoke.h>
#include "screen.h"
#include "io.h"
#include "globals.h"
#include "bar.h"

unsigned char *video_ptr;  // a pointer to the memory address containing the screen contents
unsigned char *cursor_ptr; // a pointer to the current cursor position on the screen
char _visibleEntries;
extern bool copy_mode;
char text_empty[] = "Empty";
char fn[256];

// The currently active screen (basically each time a different displaylist is used that will be considered a screen)
// Used by 'set_cursor' so it knows the layout of the screen and can properly figure out the memory offset needed whengiven an (x, y) coordinate pair.
//
_screen active_screen = SCREEN_HOSTS_AND_DEVICES;

// Set up the initial display list.
// Note: Using 25 lines in this version vs 23 in the original config (2 additional at the bottom).
void config_dlist =
    {
        DL_BLK8,              // 0x00 (8 Blank Scanlines)
        DL_BLK8,              // 0x01 (8 Blank Scanlines)
        DL_BLK8,              // 0x02 (8 Blank Scanlines)
        DL_LMS(DL_CHR20x8x2), // 0x03 Line 0 (first line of displayable text, will start at coordinates 0,0)
        DISPLAY_MEMORY,       // 0x04 and 0x05 This is the high order bit location of the display list.  Defined in screen.h
        DL_CHR20x8x2,         // 0x06  Line 1
        DL_CHR40x8x1,         // 0x07  Line 2
        DL_CHR40x8x1,         // 0x08  Line 3
        DL_CHR40x8x1,         // 0x09  Line 4
        DL_CHR40x8x1,         // 0x0a  Line 5
        DL_CHR40x8x1,         // 0x0b  Line 6
        DL_CHR40x8x1,         // 0x0c  Line 7
        DL_CHR40x8x1,         // 0x0d  Line 8
        DL_CHR40x8x1,         // 0x0e  Line 9
        DL_CHR40x8x1,         // 0x0f  Line 10
        DL_CHR40x8x1,         // 0x10  Line 11
        DL_CHR40x8x1,         // 0x11  Line 12
        DL_CHR40x8x1,         // 0x12  Line 13
        DL_CHR40x8x1,         // 0x13  Line 14
        DL_CHR40x8x1,         // 0x14  Line 15
        DL_CHR40x8x1,         // 0x15  Line 16
        DL_CHR40x8x1,         // 0x16  Line 17
        DL_CHR40x8x1,         // 0x17  Line 18
        DL_CHR40x8x1,         // 0x18  Line 19
        DL_CHR40x8x1,         // 0x19  Line 20
        DL_CHR40x8x1,         // 0x1a  Line 21
        DL_CHR20x8x2,         // 0x1b  Line 22
        DL_CHR20x8x2,         // 0x1c  Line 23
        DL_CHR40x8x1,         // 0x1d  Line 24
        DL_CHR40x8x1,         // 0x1e  Line 25
        DL_JVB,               // Signal to ANTIC end of DISPLAY_LIST has been reached and loop back to the beginning.  The jump to the begining is located at the next two bits defined below.
        DISPLAY_LIST          // 0x1f, 0x20  Memory address containing the entire display list.
};

// Patch to the character set to add things like the folder icon and the wifi-signal-strength bars.
// Each new character is 8-bytes.
//
unsigned char fontPatch[48] = {
    0, 0, 0, 0, 0, 0, 3, 51,
    0, 0, 3, 3, 51, 51, 51, 51,
    48, 48, 48, 48, 48, 48, 48, 48,
    0, 120, 135, 255, 255, 255, 255, 0,
    0x00, 0x78, 0x87, 0xff, 0xff, 0xff, 0xff, 0x00,
    0, 48, 120, 252, 48, 48, 48, 0};

void show_line_nums(void)
{
  screen_puts(0, 0, "0");
  screen_puts(0, 1, "1");
  screen_puts(0, 2, "2");
  screen_puts(0, 3, "3");
  screen_puts(0, 4, "4");
  screen_puts(0, 5, "5");
  screen_puts(0, 6, "6");
  screen_puts(0, 7, "7");
  screen_puts(0, 8, "8");
  screen_puts(0, 9, "9");
  screen_puts(0, 10, "10");
  screen_puts(0, 11, "11");
  screen_puts(0, 12, "12");
  screen_puts(0, 13, "13");
  screen_puts(0, 14, "14");
  screen_puts(0, 15, "15");
  screen_puts(0, 16, "16");
  screen_puts(0, 17, "17");
  screen_puts(0, 18, "18");
  screen_puts(0, 19, "19");
  screen_puts(0, 20, "20");
  screen_puts(0, 21, "21");
  screen_puts(0, 22, "22");
  screen_puts(0, 23, "23");
  screen_puts(0, 24, "24");
  screen_puts(0, 25, "25");

  while (!kbhit())
  {
  }
  cgetc();
  screen_clear();
}

void set_active_screen(unsigned char screen)
{
  active_screen = screen;
}

void set_cursor(unsigned char x, unsigned char y)
{
  // Each screen uses a different displaylist, so we need to know how many columns each row is so we can calculate
  // how many bytes to add to the base video memory pointer. Previously it assumed 40 columns, which resulted in the Y
  // coordinate not always use correctly in screen functions.
  //
  // This is a hacky way to do it - basically we just see what screen we're on, and since we "know" the layout so we do the math
  // appopriately. This could be more elegant by doing somethihgn like "walking" through the displaylist, PEEKing the value,
  // determining how many colums in the returned mode, and doing the math accordingly, but this is good for now, just
  // verbose.
  //
  if (active_screen == SCREEN_HOSTS_AND_DEVICES)
  {
    // 2x20 column (host header)
    // 8x40 column (host list)
    // 2x20 column (drive slot header)
    // rest 40 column (drive slots and commands)
    if (y < 2)
    {
      cursor_ptr = video_ptr + x + (y * 20); // * 20 since these are 20 column lines.
    }
    else if (y < 10)
    {
      cursor_ptr = video_ptr + 40 + x + ((y - 2) * 40); // * 40 since these are 40 column lines, and add 40 to cover the 1st 2 lines of 20 (prev condition)
    }
    else if (y < 12)
    {
      cursor_ptr = video_ptr + 40 + 320 + x + ((y - 10) * 20); // * 20 since these are 20 column lines, add 320 to cover the previous set of 40 chol lines, and 40 for the 1st set of 20 col lines.
    }
    else
    {
      cursor_ptr = video_ptr + 40 + 320 + 40 + x + ((y - 12) * 40); // * 20, rest of the the lines are 40 column, add 40 (1st 2), 320 (next 8 of 40), 40 (next 2 of 20)
    }
  }
  else if (active_screen == SCREEN_SHOW_INFO)
  {
    // 2x20
    // 3x40
    // 1x20 (double height)
    // 1x20 normal
    // rest 40
    if (y < 2)
    {
      cursor_ptr = video_ptr + x + (y * 20);
    }
    else if (y < 5)
    {
      cursor_ptr = video_ptr + 40 + x + ((y - 2) * 40);
    }
    else if (y < 6)
    {
      cursor_ptr = video_ptr + 40 + 120 + x + ((y - 5) * 20);
    }
    else if (y < 7)
    {
      cursor_ptr = video_ptr + 40 + 140 + x + ((y - 6) * 20);
    }
    else
    {
      cursor_ptr = video_ptr + 40 + 160 + x + ((y - 7) * 40);
    }
  }
  // Right now these two are the same, break apart in future if they start to differ.
  else if (active_screen == SCREEN_SELECT_FILE || active_screen == SCREEN_SELECT_SLOT || active_screen == SCREEN_MOUNT_AND_BOOT)
  {
    // 1x20
    // rest 40
    if (y < 1)
    {
      cursor_ptr = video_ptr + x + (y * 20);
    }
    else
    {
      cursor_ptr = video_ptr + 20 + x + ((y - 1) * 40);
    }
  }
  else if (active_screen == SCREEN_SET_WIFI)
  {
    // 2x20
    // 20x40
    // 2x20
    // 2x40
    if (y < 2)
    {
      cursor_ptr = video_ptr + x + (y * 20);
    }
    else if (y < 22)
    {
      cursor_ptr = video_ptr + 40 + x + ((y - 2) * 40);
    }
    else if (y < 24)
    {
      cursor_ptr = video_ptr + 40 + 800 + ((y - 22) * 20);
    }
    else
    {
      cursor_ptr = video_ptr + 40 + 800 + 40 + ((y - 24) * 40);
    }
  }
  else if (active_screen == SCREEN_CONNECT_WIFI)
  {
    // 2x20
    // 20x40
    // 2x20
    // rest 40
    if (y < 2)
    {
      cursor_ptr = video_ptr + x + (y * 20);
    }
    else if (y < 22)
    {
      cursor_ptr = video_ptr + 40 + x + ((y - 2) * 40);
    }
    else if (y < 24)
    {
      cursor_ptr = video_ptr + 40 + 800 + ((y - 22) * 20);
    }
    else
    {
      cursor_ptr = video_ptr + 40 + 800 + 40 + ((y - 24) * 40);
    }
  }
  /*
  else if (active_screen == SCREEN_MOUNT_AND_BOOT)
  {
    // 2x20
    // 8x40
    // 2x20
    // 10x40
    // 2x20
    // rest 40
    if (y < 2)
    {
      cursor_ptr = video_ptr + x + (y * 20);
    }
    else if (y < 10)
    {
      cursor_ptr = video_ptr + 40 + x + ((y - 2) * 40);
    }
    else if (y < 12)
    {
      cursor_ptr = video_ptr + 40 + 320 + x + ((y - 10) * 20);
    }
    else if (y < 22)
    {
      cursor_ptr = video_ptr + 40 + 320 + 40 + ((y - 12) * 40);
    }
    else if (y < 24)
    {
      cursor_ptr = video_ptr + 40 + 320 + 40 + 400 + ((y - 22) * 20);
    }
    else
    {
      cursor_ptr = video_ptr + 40 + 320 + 40 + 400 + 40 + ((y - 24) * 40);
    }
  }*/
  else
  {
    // Default to all 40 character lines.
    cursor_ptr = video_ptr + x + (y * 40);
  }
}

/**********************
 * Print ATASCII string to display memory.  Note: ATASCII is not a 1:1 mapping for ASCII.  It is a ven diagram with significant overlap.
 */
void put_char(char c)
{
  char offset;
  if (c < 32)
  {
    offset = 64;
  }
  else if (c < 96)
  {
    offset = -32;
  }
  else
  {
    offset = 0;
  }
  POKE(cursor_ptr++, c + offset); // Insert into the locaiton in memory for next bit in cursor_ptr the ATASCI character.  c+offset is the ATASCI character desired to be displayed.
}

void screen_append(char *s)
{
  while (*s != 0)
  {
    put_char(*s);
    ++s;
  }
}

void screen_debug(char *message)
{
  screen_clear_line(24);
  screen_puts(0, 24, message);
}

void screen_puts(unsigned char x, unsigned char y, char *s)
{
  set_cursor(x, y);
  screen_append(s);
}

void font_init()
{
  // Copy ROM font
  memcpy((unsigned char *)FONT_MEMORY, (unsigned char *)0xE000, 1024);

  // And patch it.
  memcpy(FONT_MEMORY + 520, &fontPatch, sizeof(fontPatch));

  OS.chbas = FONT_MEMORY >> 8; // use the charset
}

void screen_mount_and_boot()
{
  screen_dlist_mount_and_boot();
  set_active_screen(SCREEN_MOUNT_AND_BOOT);
  screen_clear();
  bar_clear(false);
}

void screen_set_wifi(AdapterConfig *ac)
{
  char mactmp[3];
  unsigned char i = 0;
  unsigned char x = 13;

  screen_dlist_set_wifi();
  set_active_screen(SCREEN_SET_WIFI);
  screen_clear();
  bar_clear(false);
  screen_puts(0, 0, "WELCOME TO #FUJINET!");
  screen_puts(0, 22, "SCANNING NETWORKS...");
  screen_puts(0, 2, "MAC Address:");
  screen_puts(15, 2, ":");
  screen_puts(18, 2, ":");
  screen_puts(21, 2, ":");
  screen_puts(24, 2, ":");
  screen_puts(27, 2, ":");

  // screen_set_wifi_display_mac_address(ac);
  for (i = 0; i < 6; i++)
  {
    itoa(ac->macAddress[i], mactmp, 16);
    screen_puts(x, 2, mactmp);
    x += 3;
  }
}

void screen_set_wifi_print_rssi(SSIDInfo *s, unsigned char i)
{
  char out[4] = {0x20, 0x20, 0x20, 0x00};

  if (s->rssi > -40)
  {
    out[0] = 0x01;
    out[1] = 0x02;
    out[2] = 0x03;
  }
  else if (s->rssi > -60)
  {
    out[0] = 0x01;
    out[1] = 0x02;
  }
  else
  {
    out[0] = 0x01;
  }

  screen_puts(35, i + NETWORKS_START_Y, out);
}

void screen_set_wifi_display_ssid(char n, SSIDInfo *s)
{
  screen_puts(2, n + NETWORKS_START_Y, (char *)s->ssid);
  screen_set_wifi_print_rssi(s, n);
}

void screen_set_wifi_select_network(unsigned char nn)
{
  screen_clear_line(numNetworks + NETWORKS_START_Y);
  screen_puts(2, NETWORKS_START_Y + numNetworks, "<Enter a specific SSID>");

  screen_puts(0, 22, " SELECT NET, S SKIP "
                     "   ESC TO RE-SCAN   ");

  bar_show(NETWORKS_START_Y);
}

void screen_set_wifi_custom(void)
{
  screen_clear_line(20);
  screen_puts(0, 22, " ENTER NETWORK NAME "
                     "  AND PRESS return  ");
}

void screen_set_wifi_password(void)
{
  screen_clear_line(22);
  screen_puts(3, 22, "   ENTER PASSWORD");
}

void screen_print_ip(unsigned char x, unsigned char y, unsigned char *buf)
{
  unsigned char i = 0;
  unsigned char tmp[4];

  set_cursor(x, y);
  for (i = 0; i < 4; i++)
  {
    itoa(buf[i], tmp, 10);
    screen_append(tmp);
    if (i == 3)
      break;
    screen_append(".");
  }
}

/**
 * Convert hex to a string and print as a MAC address at position x, y
 */
void screen_print_mac(unsigned char x, unsigned char y, unsigned char *buf)
{
  unsigned char i = 0;
  unsigned char tmp[3];

  set_cursor(x, y);
  for (i = 0; i < 6; i++)
  {
    itoa_hex(buf[i], tmp);
    screen_append(tmp);
    if (i == 5)
      break;
    screen_append(":");
  }
}

/**
 * Convert hex to a string.  Special hex output of numbers under 16, e.g. 9 -> 09, 10 -> 0A
 */
void itoa_hex(unsigned char val, char *buf)
{

  if (val < 16)
  {
    *(buf++) = '0';
  }
  itoa(val, buf, 16);
}

/*
 * Display the 'info' screen
 */
void screen_show_info(int printerEnabled, AdapterConfig *ac)
{
  unsigned char i;
  screen_dlist_show_info();
  set_active_screen(SCREEN_SHOW_INFO);
  screen_clear();
  bar_clear(false);

  screen_puts(3, 5, "#FUJINET CONFIG");
  screen_puts(7, 17,
              CH_KEY_LABEL_L CH_INV_C CH_KEY_LABEL_R "RECONNECT " CH_KEY_LABEL_L CH_INV_S CH_KEY_LABEL_R "CHANGE SSID");
  screen_puts(9, 19, "Any other key to return");
  screen_puts(5, 7, "      SSID:");
  screen_puts(5, 8, "  Hostname:");
  screen_puts(5, 9, "IP Address:");
  screen_puts(5, 10, "   Gateway:");
  screen_puts(5, 11, "       DNS:");
  screen_puts(5, 12, "   Netmask:");
  screen_puts(5, 13, "       MAC:");
  screen_puts(5, 14, "     BSSID:");
  screen_puts(5, 15, "   Version:");

  screen_puts(17, 7, ac->ssid);
  screen_puts(17, 8, ac->hostname);
  screen_print_ip(17, 9, ac->localIP);
  screen_print_ip(17, 10, ac->gateway);
  screen_print_ip(17, 11, ac->dnsIP);
  screen_print_ip(17, 12, ac->netmask);
  screen_print_mac(17, 13, ac->macAddress);
  screen_print_mac(17, 14, ac->bssid);
  screen_puts(17, 15, ac->fn_version);
}

void screen_select_slot(char *e)
{
  unsigned int *s;
  unsigned char d[40];

  screen_dlist_select_slot();
  set_active_screen(SCREEN_SELECT_SLOT);

  screen_clear();

  screen_puts(0, 22,
              CH_KEY_1TO8 "Slot" CH_KEY_RETURN "Select" CH_KEY_LABEL_L CH_INV_E CH_KEY_LABEL_R "ject" CH_KEY_ESC "Abort");

  screen_puts(0, 0, "MOUNT TO DRIVE SLOT");

  // Show file details if it's an existing file only.
  if ( create == false )
  {
    // Modified time 
    sprintf(d, "%8s %04u-%02u-%02u %02u:%02u:%02u", "MTIME:", (*e++) + 1970, *e++, *e++, *e++, *e++, *e++);
    screen_puts(0, DEVICES_END_MOUNT_Y + 3, d);

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
    screen_puts(3, DEVICES_END_MOUNT_Y + 2, "FILE:");
    screen_puts(9, DEVICES_END_MOUNT_Y + 2, e);
  }

  screen_hosts_and_devices_device_slots(DEVICES_START_MOUNT_Y, &deviceSlots, &deviceEnabled);

  bar_show(DEVICES_START_MOUNT_Y);
}

void screen_select_slot_mode(void)
{
  screen_clear_line(21);
  screen_puts(0, 22,
              CH_KEY_RETURN "Read Only" CH_KEY_LABEL_L CH_INV_W CH_KEY_LABEL_R "Read/Write" CH_KEY_ESC "Abort");
}

void screen_select_slot_choose(void)
{
}

void screen_select_slot_eject(unsigned char ds)
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
  screen_dlist_select_file();
  set_active_screen(SCREEN_SELECT_FILE);
  screen_clear();
  bar_clear(false);

  screen_puts(4, 0, "DISK IMAGES");

  if (copy_mode == false)
  {
    screen_puts(0, 21,
                CH_KEY_LEFT CH_KEY_DELETE "Up Dir" CH_KEY_N "ew" CH_KEY_F "ilter" CH_KEY_C "opy");
  }
  else
  {
    screen_puts(0, 21,
                CH_KEY_LEFT CH_KEY_DELETE "Up Dir" CH_KEY_N "ew" CH_KEY_F "ilter" CH_KEY_C "Do It!");
  }
  screen_puts(0, 22,
              CH_KEY_RIGHT CH_KEY_RETURN "Choose" CH_KEY_OPTION "Boot" CH_KEY_ESC "Abort");
}

void screen_select_file_display(char *p, char *f)
{
  unsigned char i;
  // Host
  screen_puts(0, 1, "Host:");
  screen_puts(5, 1, selected_host_name);

  // Filter
  screen_puts(0, 2, "Fltr:");
  screen_puts(5, 2, f);

  // Path - the path can wrap to line 4 (maybe 5?) so clear both to be safe.
  screen_clear_line(3);
  screen_clear_line(4);
  screen_puts(0, 3, "Path:");
  screen_puts(5, 3, p);

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
  screen_puts(0, 24, "LF:");
  screen_puts(4, 24, e);
}

void screen_select_file_clear_long_filename(void)
{
}

void screen_select_file_filter(void)
{
  // No need to display anything additional, we're already showing the filter on the screen.
}

void screen_select_file_next(void)
{
  if (dir_eof == false)
  {
    screen_puts(0, FILES_START_Y + ENTRIES_PER_PAGE, CH_KEY_GT "Next Page");
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
    screen_puts(0, FILES_START_Y - 1, CH_KEY_LT "Previous Page");
  }

  if (dir_eof == true)
  {
    // We're on the last page, clear the line with the "Next Page" text.
    screen_clear_line(FILES_START_Y + ENTRIES_PER_PAGE);
  }
}

void screen_select_file_display_entry(unsigned char y, char *e)
{
  screen_puts(3, FILES_START_Y + y, e);
}

void screen_select_file_choose(char visibleEntries)
{
  bar_show(FILES_START_Y);
  _visibleEntries = visibleEntries;
}

void screen_select_file_new_type(void)
{
  // Not used on Atari
}

void screen_select_file_new_size(unsigned char k)
{
  screen_clear_line(20);
  screen_clear_line(21);

  screen_puts(0, 20, "Size?" CH_KEY_LABEL_L CH_INV_1 CH_KEY_LABEL_R "90K  " CH_KEY_LABEL_L CH_INV_2 CH_KEY_LABEL_R "130K  " CH_KEY_LABEL_L CH_INV_3 CH_KEY_LABEL_R "180K  " CH_KEY_LABEL_L CH_INV_4 CH_KEY_LABEL_R "360K  ");
  screen_puts(5, 21,
              CH_KEY_LABEL_L CH_INV_5 CH_KEY_LABEL_R "720K " CH_KEY_LABEL_L CH_INV_6 CH_KEY_LABEL_R "1440K " CH_KEY_LABEL_L CH_INV_7 CH_KEY_LABEL_R "Custom ?");
}

void screen_select_file_new_custom(void)
{
  screen_clear_line(20);
  screen_clear_line(21);

  screen_puts(0, 20, "# Sectors?");
  screen_puts(0, 21, "Sector Size (128/256/512)?");
}

void screen_select_file_new_name(void)
{
  screen_clear_line(20);
  screen_clear_line(21);
  screen_puts(0, 20, "Enter name of new disk image file");
}

void screen_select_file_new_creating(void)
{
  screen_clear();
  screen_puts(3, 0, "Creating File");
}

void screen_clear_line(unsigned char y)
{
  set_cursor(0, y);          // move the cursor X position 0 and Y position.
  memset(cursor_ptr, 0, 40); // fill the memory with spaces.  The zeros in the memory addresses are interpreted as spaces on the console
}

void screen_error(const char *msg)
{
  screen_clear_line(24);
  screen_puts(0, 24, msg);
}

void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d, unsigned char *e)
{
  unsigned char retry = 5;
  unsigned char i;
  char temp[10];

  screen_dlist_hosts_and_devices();
  set_active_screen(SCREEN_HOSTS_AND_DEVICES);

  screen_clear();
  bar_clear(false);

  screen_puts(3, 0, "TNFS HOST LIST");
  // A = 41
  // INV A = C1
  // INV A = A1 in this
  // screen_puts(0,2, "\x07" " xx" );
  // xx();
  // POKE(cursor_ptr++, CSI_A+32);

  screen_puts(4, 11, "DRIVE SLOTS");

  while (retry > 0)
  {
    io_get_host_slots(&hostSlots[0]);

    if (io_error())
      retry--;
    else
      break;
  }

  if (io_error())
  {
    screen_error("ERROR READING HOST SLOTS");
    while (!kbhit())
    {
    }
    cold_start();
  }

  retry = 5;

  screen_hosts_and_devices_host_slots(&hostSlots[0]);

  while (retry > 0)
  {
    io_get_device_slots(&deviceSlots[0]);

    if (io_error())
      retry--;
    else
      break;
  }

  if (io_error())
  {
    screen_error("ERROR READING DEVICE SLOTS");
    // die();
    while (!kbhit())
    {
    }
    cold_start();
  }

  screen_hosts_and_devices_device_slots(DEVICES_START_Y, &deviceSlots[0], "");
}

void screen_clear()
{
  cursor_ptr = video_ptr;                       // Assign the current position of the cursor position address to the address of the video screen
  memset(video_ptr, 0, GRAPHICS_0_SCREEN_SIZE); // Fill the memory address with blanks of the size of the screen; in this case Atari graphics mode 0 size.
}

// Show the keys that are applicable when we are on the Hosts portion of the screen.
void screen_hosts_and_devices_hosts(void)
{
  screen_clear_line(22);
  screen_clear_line(23);
  screen_puts(0, 22,
              CH_KEY_1TO8 "Slot" CH_KEY_LABEL_L CH_INV_E CH_KEY_LABEL_R "dit Slot" CH_KEY_RETURN "Select Files");
  screen_puts(2, 23,
              CH_KEY_LABEL_L CH_INV_C CH_KEY_LABEL_R "onfig" CH_KEY_TAB "Drive Slots" CH_KEY_OPTION "Boot");

  // bar_show(2);
  bar_show(selected_host_slot + HOSTS_START_Y);
}

// Show the keys that are applicable when we are on the Devices portion of the screen.
void screen_hosts_and_devices_devices(void)
{
  screen_clear_line(22);
  screen_clear_line(23);

  screen_clear_line(11);
  screen_puts(4, 11, "DRIVE SLOTS");

  screen_puts(3, 22,
              CH_KEY_1TO8 "Slot" CH_KEY_LABEL_L CH_INV_E CH_KEY_LABEL_R "ject" CH_KEY_LABEL_L CH_INV_C CH_INV_L CH_INV_E CH_INV_A CH_INV_R CH_KEY_LABEL_R "All Slots");
  screen_puts(3, 23,
              CH_KEY_TAB "Hosts" CH_KEY_LABEL_L CH_INV_R CH_KEY_LABEL_R "ead " CH_KEY_LABEL_L CH_INV_W CH_KEY_LABEL_R "rite" CH_KEY_LABEL_L CH_INV_C CH_KEY_LABEL_R "onfig");
  bar_show(selected_device_slot + DEVICES_START_Y);
}

void screen_hosts_and_devices_host_slots(HostSlot *h)
{
  unsigned char slotNum;

  for (slotNum = 0; slotNum < NUM_HOST_SLOTS; slotNum++)
  {
    set_cursor(2, slotNum + HOSTS_START_Y);
    put_char(slotNum + '1');
    cursor_ptr += 2;

    screen_append((hostSlots[slotNum][0] != 0x00) ? (char *)hostSlots[slotNum] : text_empty);
  }
}

// Since 'deviceSlots' is a global, do we need to access the input parameter at all?
// Maybe globals.h wasn't supposed in be part of screen? I needed it for something..
void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *dslot, unsigned char *e)
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

    screen_puts(0, slotNum + y, dinfo);

    screen_append(deviceSlots[slotNum].file[0] != 0x00 ? (char *)deviceSlots[slotNum].file : text_empty);
  }
}

void screen_hosts_and_devices_devices_clear_all(void)
{
  screen_clear_line(11);
  screen_puts(0, 11, "EJECTING ALL.. WAIT");
}

void screen_hosts_and_devices_clear_host_slot(unsigned char i)
{
  // i comes in as the place in the array for this host slot. To get the corresponding position on the screen, add HOSTS_START_Y
  screen_clear_line(i + HOSTS_START_Y);
}

void screen_hosts_and_devices_edit_host_slot(unsigned char i)
{
  char tmp[2] = {0, 0};
  int newloc = i + HOSTS_START_Y;

  screen_clear_line(newloc);
  tmp[0] = newloc - HOSTS_START_Y + 0x31;
  screen_puts(2, newloc, tmp);
}

void screen_hosts_and_devices_eject(unsigned char ds)
{
  char tmp[2] = {0, 0};
  tmp[0] = ds + '1';

  bar_show(DEVICES_START_Y + ds);
  screen_clear_line(DEVICES_START_Y + ds);
  screen_puts(2, DEVICES_START_Y + ds, tmp);
  screen_puts(5, DEVICES_START_Y + ds, text_empty);
}

void screen_hosts_and_devices_host_slot_empty(unsigned char hs)
{
  // When this gets called it seems like the cursor is right where we want it to be.
  // so no need to move to a position first.
  screen_append(text_empty);
}

void screen_hosts_and_devices_long_filename(char *f)
{
  // show_line_nums();
}

void screen_init(void)
{
  memcpy((void *)DISPLAY_LIST, &config_dlist, sizeof(config_dlist)); // copy display list to $0600
  OS.sdlst = (void *)DISPLAY_LIST;                                   // and use it.
  video_ptr = (unsigned char *)(DISPLAY_MEMORY);                     // assign the value of DISPLAY_MEMORY to video_ptr

  font_init();
  bar_setup_regs();
  bar_clear(false);

  selected_host_slot = 0;
  selected_device_slot = 0;
}

void screen_destination_host_slot(char *h, char *p)
{
  screen_clear();
  screen_puts(0, 22,
              CH_KEY_1TO8 "Slot" CH_KEY_RETURN "Select" CH_KEY_ESC "Abort");

  screen_puts(0, 18, h);
  screen_puts(0, 19, p);

  screen_puts(0, 0, "COPY TO HOST SLOT");
  bar_show(HOSTS_START_Y);
}

void screen_destination_host_slot_choose(void)
{
  // show_line_nums();
}

void screen_perform_copy(char *sh, char *p, char *dh, char *dp)
{
  // show_line_nums();
  screen_clear();
  bar_clear(false);
  screen_puts(0, 0, "COPYING, PLEASE WAIT");
}

void screen_dlist_select_file(void)
{
  POKE(DISPLAY_LIST + 0x06, 2);
  POKE(DISPLAY_LIST + 0x0f, 2);
  POKE(DISPLAY_LIST + 0x10, 2);
  POKE(DISPLAY_LIST + 0x1b, 2);
  POKE(DISPLAY_LIST + 0x1c, 2);
}

void screen_dlist_select_slot(void)
{
  POKE(DISPLAY_LIST + 0x06, 2);
  POKE(DISPLAY_LIST + 0x0f, 2);
  POKE(DISPLAY_LIST + 0x10, 2);
  POKE(DISPLAY_LIST + 0x1b, 2);
  POKE(DISPLAY_LIST + 0x1c, 2);
}

void screen_dlist_show_info(void)
{
  // 2x20
  // 3x40
  // 1x20 (double height)
  // 1x20 normal
  // rest 40

  POKE(DISPLAY_LIST + 0x0a, 7);
  POKE(DISPLAY_LIST + 0x0b, 6);
  POKE(DISPLAY_LIST + 0x0f, 2);
  POKE(DISPLAY_LIST + 0x10, 2);
}

void screen_dlist_set_wifi(void)
{
  POKE(DISPLAY_LIST + 0x0a, 2);
  POKE(DISPLAY_LIST + 0x0b, 2);
  POKE(DISPLAY_LIST + 0x1b, 6);
  POKE(DISPLAY_LIST + 0x1c, 6);
}

void screen_dlist_mount_and_boot(void)
{
  // Same as wifi layout
  //screen_dlist_set_wifi();
  screen_dlist_select_file();
}

void screen_connect_wifi(NetConfig *nc)
{
  screen_dlist_show_info();
  screen_dlist_set_wifi();
  set_active_screen(SCREEN_CONNECT_WIFI);
  screen_clear();
  bar_clear(false);

  screen_puts(0, 0, "WELCOME TO #FUJINET! CONNECTING TO NET");
  screen_puts(2, 3, nc->ssid);
  bar_show(3);
}

void screen_dlist_hosts_and_devices(void)
{
  // Screen Layout
  // 2x20 column (host header)
  // 8x40 column (host list)
  // 2x20 column (drive slot header)
  // rest 40 column (drive slots and commands)

  POKE(DISPLAY_LIST + 0x06, 6);
  POKE(DISPLAY_LIST + 0x0f, 6);
  POKE(DISPLAY_LIST + 0x10, 6);
  POKE(DISPLAY_LIST + 0x0a, 2);
  POKE(DISPLAY_LIST + 0x0b, 2);
  POKE(DISPLAY_LIST + 0x1b, 2);
  POKE(DISPLAY_LIST + 0x1c, 2);
}

int _screen_input(unsigned char x, unsigned char y, char *s, unsigned char maxlen)
{
  unsigned char k, o;
  unsigned char *input_start_ptr;

  o = strlen(s);                // assign to local var the size of s which contains the current string
  set_cursor(x, y);             // move the cusor to coordinates x,y
  input_start_ptr = cursor_ptr; // assign the value currently in cursor_ptr to local var input_start
  screen_append(s);             // call screen_append function and pass by value (a copy) the contents of s

  POKE(cursor_ptr, 0x80); // turn on cursor

  // Start capturing the keyboard input into local var k
  do
  {
    k = cgetc(); // Capture keyboard input into k

    if (k == KCODE_ESCAPE) // KCODE_ESCAPE is the ATASCI code for the escape key which is commonly used to cancel.
      return -1;

    if (k == KCODE_BACKSP) // KCODE_BACKSP is the ATASCI backspace key.  This if clause test for backspace and updates cursor_ptr to the remainder of contents upto the last backspace
    {
      if (cursor_ptr > input_start_ptr) // execute only if cursor_ptr is greater than input_start_ptr
      {
        s[--o] = 0;                  //
        POKEW(--cursor_ptr, 0x0080); // move the last bit of the cursor_ptr back one and write the location of the cursor_ptr contents to user zero page address 0x80
      }
    }
    else if ((k > 0x1F) && (k < 0x80)) // Display printable ascii to screen
    {
      if (o < maxlen - 1)
      {
        put_char(k);
        s[o++] = k;
        POKE(cursor_ptr, 0x80);
      }
    }
  } while (k != KCODE_RETURN); // Continue to capture keyboard input until return (0x9B)
  POKE(cursor_ptr, 0x00);      // clear cursor
}

/*
int util_ellipsize(const char *src, char *dst, int dstsize)
{
    // Don't do much if there's no space to copy anything
    if (dstsize <= 1)
    {
        if (dstsize == 1)
            dst[0] = '\0';
        return 0;
    }

    int srclen = strlen(src);

    // Do a simple copy if we have the room for it (or if we don't have room to create a string with ellipsis in the middle)
    if (srclen < dstsize || dstsize < 6)
    {
        return strlcpy(dst, src, dstsize);
    }

    // Account for both the 3-character ellipses and the null character that needs to fit in the destination
    int rightlen = (dstsize - 4) / 2;
    // The left side gets one more character if the destination is odd
    int leftlen = rightlen + dstsize % 2;

    strlcpy(dst, src, leftlen + 1); // Add one because strlcpy wants to add its own NULL

    dst[leftlen] = dst[leftlen + 1] = dst[leftlen + 2] = '.';

    strlcpy(dst + leftlen + 3, src + (srclen - rightlen), rightlen + 1); // Add one because strlcpy wants to add its own NULL

    return dstsize;
}
*/
#endif