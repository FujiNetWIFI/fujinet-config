#ifdef _CMOC_VERSION_

/**
 * File browser, device slot selection and copy screens.
 */

#include "coco_screen.h"
#include "../globals.h"
#include "../constants.h"

static void put_padded(byte x, byte y, const char *s, byte w)
{
  char buf[LIST_W + 1];
  byte n = (byte)strlen(s);

  if (n > w)
    n = w;
  memset(buf, ' ', w);
  memcpy(buf, s, n);
  buf[w] = 0;
  screen_puts(x, y, buf);
}

static char *put2(char *p, byte v)
{
  *p++ = (char)('0' + v / 10);
  *p++ = (char)('0' + v % 10);
  return p;
}

static char *put_uint(char *p, unsigned v)
{
  char d[5];
  byte n = 0;

  do
  {
    d[n++] = (char)('0' + v % 10);
    v /= 10;
  } while (v);
  while (n)
    *p++ = d[--n];
  return p;
}

static void put_wrapped_at(byte x, byte y, const char *s, byte lines, byte w)
{
  byte n = (byte)strlen(s);

  while (lines--)
  {
    put_padded(x, y++, s, w);
    s += n > w ? w : n;
    n -= n > w ? w : n;
  }
}

static void put_wrapped(byte y, const char *s, byte lines)
{
  put_wrapped_at(INFO_X, y, s, lines, INFO_W);
}

void screen_select_slot(const char *e)
{
  struct _additl_info
  {
    byte year;
    byte month;
    byte day;
    byte hour;
    byte min;
    byte sec;
    unsigned long size;
    byte isdir;
    byte trunc;
    byte type;
    byte *filename;
  } *i = (struct _additl_info *)e;
  char s[LIST_W + 1];
  char *p;

  screen_frame("FujiNet Config");
  screen_box(0, SLOT_BOX_Y, SCREEN_COLS, NUM_DEVICE_SLOTS + 2, "DRIVE SLOTS");
  screen_hosts_and_devices_device_slots(SLOT_Y, &deviceSlots[0], &deviceEnabled[0]);
  screen_box(0, DETAIL_BOX_Y, SCREEN_COLS, 6, "FILE DETAILS");

  if (create == true)
  {
    put_wrapped(DETAIL_BOX_Y + 1, e + 11, 2);
  }
  else
  {
    put_wrapped(DETAIL_BOX_Y + 1, e + 13, 2);
    strcpy(s, "Date: 20");
    p = put2(s + 8, i->year);
    *p++ = '-';
    p = put2(p, i->month);
    *p++ = '-';
    p = put2(p, i->day);
    *p++ = ' ';
    p = put2(p, i->hour);
    *p++ = ':';
    p = put2(p, i->min);
    *p++ = ':';
    p = put2(p, i->sec);
    *p = 0;
    screen_puts(INFO_X, DETAIL_BOX_Y + 3, s);

    strcpy(s, "Size: ");
    if (i->size >= 1048576UL)
    {
      p = put_uint(s + 6, (unsigned)(i->size >> 20));
      strcpy(p, " M");
    }
    else if (i->size >= 1024UL)
    {
      p = put_uint(s + 6, (unsigned)(i->size >> 10));
      strcpy(p, " K");
    }
    else
    {
      p = put_uint(s + 6, (unsigned)i->size);
      strcpy(p, " bytes");
    }
    screen_puts(INFO_X, DETAIL_BOX_Y + 4, s);
  }

  screen_move(MENU_X, MENU_Y1);
  screen_print_menu("ENTER", " Read only  ");
  screen_print_menu("W", "rite  ");
  screen_print_menu("E", "ject");
  screen_move(MENU_X, MENU_Y2);
  screen_print_menu("ARROWS", " Select slot  ");
  screen_print_menu("BREAK", " Abort");
  bar_set(SLOT_Y, 1, NUM_DEVICE_SLOTS, 0);
}

void screen_select_slot_mode(void)
{
}

void screen_select_slot_choose(void)
{
}

void screen_select_slot_eject(unsigned char ds)
{
  screen_hosts_and_devices_device_slots(SLOT_Y, &deviceSlots[0], &deviceEnabled[0]);
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

static void file_boxes(void)
{
  static bool drawn;
  static unsigned drawn_generation;

  if (drawn && drawn_generation == screen_generation)
    return;
  screen_frame("FujiNet Config");
  screen_box(0, INFO_BOX_Y, SCREEN_COLS, 5, NULL);
  screen_box(0, FILES_BOX_Y, SCREEN_COLS, ENTRIES_PER_PAGE + 2, NULL);
  drawn = true;
  drawn_generation = screen_generation;
}

static void file_info(const char *p, const char *f)
{
  byte n = (byte)strlen(p);
  byte w = INFO_W - 6;

  put_padded(FILTER_X, INFO_BOX_Y + 1, selected_host_name, w);
  put_padded(FILTER_X, FILTER_Y, f, w);
  put_padded(FILTER_X, INFO_BOX_Y + 3, n > w ? p + n - w : p, w);
  screen_puts(INFO_X, INFO_BOX_Y + 1, "Host: ");
  screen_puts(INFO_X, FILTER_Y, "Fltr: ");
  screen_puts(INFO_X, INFO_BOX_Y + 3, "Path: ");
}

void screen_select_file(void)
{
  file_boxes();
  file_info(path, filter);
  screen_message_target(FILES_Y, true, 120);
  screen_puts(FILE_X + 1, FILES_Y, "Opening...");
}

/* Paging and folder changes come through here again; only the contents are redrawn. */
void screen_select_file_display(char *p, char *f)
{
  byte y, x;

  file_boxes();
  file_info(p, f);
  screen_message_target(FILES_Y, true, 120);

  for (y = FILES_Y; y < FILES_Y + ENTRIES_PER_PAGE; y++)
    put_padded(FILE_X, y, "", FILE_W);
  for (x = SCREEN_COLS - 10; x < SCREEN_COLS - 3; x++)
  {
    screen_put(x, FILES_BOX_Y, BOX_H);
    screen_put(x, FILES_BOX_Y + ENTRIES_PER_PAGE + 1, BOX_H);
  }
}

void screen_select_file_display_long_filename(const char *e)
{
}

void screen_select_file_clear_long_filename(void)
{
}

void screen_select_file_filter(void)
{
  put_padded(FILTER_X, FILTER_Y, "", INFO_W - 6);
}

void screen_select_file_next(void)
{
  screen_puts(SCREEN_COLS - 10, FILES_BOX_Y + ENTRIES_PER_PAGE + 1, " [...] ");
}

void screen_select_file_prev(void)
{
  screen_puts(SCREEN_COLS - 10, FILES_BOX_Y, " [...] ");
}

static bool row_is_dir[ENTRIES_PER_PAGE];

void screen_select_file_display_entry(unsigned char y, const char *e, unsigned entryType)
{
  byte n = (byte)strlen(e);

  row_is_dir[y] = n && e[n - 1] == '/';
  put_padded(FILE_X, FILES_Y + y, e, FILE_W);
}

bool screen_select_file_row_is_dir(byte y)
{
  return row_is_dir[y];
}

byte redraw_row = 0xFF;

void screen_select_file_choose(char visibleEntries)
{
  byte row;

  screen_menu_clear();
  screen_move(MENU_X, MENU_Y1);
  screen_print_menu("ENTER", " Select  ");
  screen_print_menu("<-", " Up dir  ");
  screen_print_menu("BREAK", copy_mode == true ? " Abort" : " Back");
  screen_move(MENU_X, MENU_Y2);
  if (copy_mode == true)
  {
    screen_print_menu("C", "opy here  ");
  }
  else
  {
    screen_print_menu("F", "ilter  ");
    screen_print_menu("N", "ew  ");
    screen_print_menu("C", "opy  ");
  }
  screen_print_menu("SHIFT-UP/DN", " Page");

  row = prev_page ? visibleEntries - 1 : 0;
  if (redraw_row < visibleEntries)
    row = redraw_row;
  redraw_row = 0xFF;
  bar_set(FILES_Y, 1, visibleEntries, row);
}

void screen_select_file_new_type(void)
{
}

void screen_select_file_new_size(unsigned char k)
{
  screen_prompt("Number of drives to create:");
}

void screen_select_file_new_custom(void)
{
}

void screen_select_file_new_name(void)
{
  screen_prompt("Image file name:");
}

void screen_select_file_new_creating(void)
{
  screen_prompt("Creating image, please wait...");
}

void screen_destination_host_slot(char *h, char *p)
{
  screen_frame("FujiNet Config");
  screen_box(0, HOSTS_BOX_Y, SCREEN_COLS, NUM_HOST_SLOTS + 2, "COPY TO HOST SLOT");
  screen_box(0, DEVICES_BOX_Y, SCREEN_COLS, 6, "DISK IMAGE DETAILS");
  screen_puts(INFO_X, DEVICES_BOX_Y + 1, "Host: ");
  screen_puts(INFO_X + 6, DEVICES_BOX_Y + 1, h);
  screen_puts(INFO_X, DEVICES_BOX_Y + 2, "Path: ");
  put_wrapped_at(INFO_X + 6, DEVICES_BOX_Y + 2, p, 2, INFO_W - 6);
}

void screen_destination_host_slot_choose(void)
{
  screen_menu_clear();
  screen_move(MENU_X, MENU_Y1);
  screen_print_menu("1-8", " Slot  ");
  screen_print_menu("ENTER", " Select  ");
  screen_print_menu("BREAK", " Abort");
  bar_set(HOSTS_Y, 1, NUM_HOST_SLOTS, selected_host_slot);
}

void screen_perform_copy(char *sh, char *p, char *dh, char *dp)
{
  screen_frame("FujiNet Config");
  screen_box(0, 6, SCREEN_COLS, 8, "COPYING FILE");
  screen_puts(INFO_X, 7, "From: ");
  screen_puts(INFO_X + 6, 7, sh);
  put_wrapped_at(INFO_X + 6, 8, p, 1, INFO_W - 6);
  screen_puts(INFO_X, 10, "To:   ");
  screen_puts(INFO_X + 6, 10, dh);
  put_wrapped_at(INFO_X + 6, 11, dp, 1, INFO_W - 6);
  screen_puts(MENU_X, MENU_Y1, "Please wait...");
}

#endif
