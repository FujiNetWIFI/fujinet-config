#ifdef BUILD_MODEL2

/**
 * @file input.c
 * @brief Keyboard input and the screens' key handling - TRS-80 Model II.
 *
 * input() sends the screen only when no key is waiting, so keys typed (or
 * held) faster than the screen can redraw skip the screens in between.
 */

#include <stdlib.h>
#include <string.h>
#include "../input.h"
#include "../screen.h"
#include "../globals.h"
#include "../constants.h"
#include "../die.h"
#include "../hosts_and_devices.h"
#include "../select_file.h"
#include "../select_slot.h"
#include "../set_wifi.h"
#include "vid.h"
#include "model2.h"
#include "kbd.h"
#include "layout.h"
#include "ui.h"

extern unsigned char copy_host_slot;

bool mounting = false;

uint8_t input(void)
{
  static uint8_t k;

  k = kbd_get();
  if (!k)
  {
    vid_flush();
    k = kbd_get();
  }
  if (k == KEY_CTRL_C)
    quit();
  return k;
}

/**
 * Edit c in a field at x,y. l is the size of c, so at most l-1 characters.
 * ENTER keeps the text, ESC puts the old text back.
 */
void input_line(uint8_t x, uint8_t y, uint8_t o, char *c, uint8_t l, bool password)
{
  static char orig[256];
  static uint8_t n, w, k, i, shown, from;
  static bool done;

  (void)o;
  if (!l)
    return;

  strncpy(orig, c, sizeof(orig) - 1);
  orig[sizeof(orig) - 1] = 0;
  c[l - 1] = 0;
  n = (uint8_t)strlen(c);

  /* The field runs to the frame's right edge at most, with a cell for the
     cursor after the text */
  w = l;
  if (w > VID_COLS - 1 - x)
    w = VID_COLS - 1 - x;

  done = false;
  while (!done)
  {
    /* Show the end of the text when it is longer than the field */
    from = n >= w ? n - w + 1 : 0;
    shown = n - from;
    for (i = 0; i < w; i++)
    {
      if (i < shown)
        vid_put(x + i, y, (password ? '*' : c[from + i]) | REV);
      else
        vid_put(x + i, y, ' ' | REV);
    }
    vid_cursor(x + shown, y);

    k = kbd_wait();
    switch (k)
    {
    case KEY_RETURN:
      done = true;
      break;
    case KEY_ESCAPE:
      strncpy(c, orig, l - 1);
      c[l - 1] = 0;
      n = (uint8_t)strlen(c);
      done = true;
      break;
    case KEY_CTRL_C:
      quit();
      break;
    case KEY_BACKSPACE:
    case KEY_DELETE:
    case KEY_LEFT_ARROW:
      if (n > 0)
        c[--n] = 0;
      break;
    default:
      if (k >= 0x20 && k <= 0x7E && n < l - 1)
      {
        c[n++] = k;
        c[n] = 0;
      }
      break;
    }
  }

  /* Leave the text behind, out of reverse */
  for (i = 0; i < w; i++)
    vid_put(x + i, y, i < n && i < w ? (password ? '*' : c[i]) : ' ');
  vid_cursor(VID_NONE, 0);
  vid_flush();
}

/*
 * Hosts and devices
 */

static uint8_t digit_slot(uint8_t k)
{
  if (k >= '1' && k <= '8')
    return k - '1';
  return 0xFF;
}

HDSubState input_hosts_and_devices_hosts(void)
{
  static uint8_t k, s;

  k = input();
  s = digit_slot(k);
  if (s != 0xFF)
  {
    bar_jump(s);
    selected_host_slot = bar_get();
    return HD_HOSTS;
  }

  switch (k)
  {
  case KEY_UP_ARROW:
    bar_up();
    selected_host_slot = bar_get();
    return HD_HOSTS;
  case KEY_DOWN_ARROW:
    /* Off the bottom of the hosts into the drives */
    if (bar_get() == NUM_HOST_SLOTS - 1)
    {
      selected_device_slot = 0;
      return HD_DEVICES;
    }
    bar_down();
    selected_host_slot = bar_get();
    return HD_HOSTS;
  case 'e':
  case 'E':
    hosts_and_devices_edit_host_slot(bar_get());
    return HD_HOSTS;
  case KEY_RETURN:
    selected_host_slot = bar_get();
    if (hostSlots[selected_host_slot][0] == 0)
    {
      /* Nothing to browse yet: name it first */
      hosts_and_devices_edit_host_slot(selected_host_slot);
      return HD_HOSTS;
    }
    strncpy(selected_host_name, (const char *)hostSlots[selected_host_slot], 31);
    selected_host_name[31] = 0;
    state = SELECT_FILE;
    return HD_DONE;
  case KEY_TAB:
    return HD_DEVICES;
  case 'c':
  case 'C':
    state = SHOW_INFO;
    return HD_DONE;
  case 'q':
  case 'Q':
    quit();
    return HD_HOSTS;
  case KEY_ESCAPE:
    return HD_DONE;
  default:
    return HD_HOSTS;
  }
}

static void devices_moved(void)
{
  selected_device_slot = bar_get();
  screen_drive_path();
}

/** R or W on a drive. The shared code sets the drive's filename against
    selected_host_slot, so point that at the drive's own host meanwhile. */
static void devices_set_mode(uint8_t m)
{
  static char hs;

  selected_device_slot = bar_get();
  if (deviceSlots[selected_device_slot].hostSlot >= NUM_HOST_SLOTS
      || !deviceSlots[selected_device_slot].file[0])
    return;

  hs = selected_host_slot;
  selected_host_slot = deviceSlots[selected_device_slot].hostSlot;
  hosts_and_devices_devices_set_mode(m);
  selected_host_slot = hs;

  screen_drive_row(DRIVES_Y + selected_device_slot, selected_device_slot);
}

HDSubState input_hosts_and_devices_devices(void)
{
  static uint8_t k, s;

  k = input();
  s = digit_slot(k);
  if (s != 0xFF)
  {
    bar_jump(s);
    devices_moved();
    return HD_DEVICES;
  }

  switch (k)
  {
  case KEY_UP_ARROW:
    /* Off the top of the drives into the hosts */
    if (bar_get() == 0)
    {
      selected_host_slot = NUM_HOST_SLOTS - 1;
      return HD_HOSTS;
    }
    bar_up();
    devices_moved();
    return HD_DEVICES;
  case KEY_DOWN_ARROW:
    bar_down();
    devices_moved();
    return HD_DEVICES;
  case 'e':
  case 'E':
    hosts_and_devices_eject(bar_get());
    devices_moved();
    return HD_DEVICES;
  case 'x':
  case 'X':
    return HD_CLEAR_ALL_DEVICES;
  case 'r':
  case 'R':
    devices_set_mode(MODE_READ);
    return HD_DEVICES;
  case 'w':
  case 'W':
    devices_set_mode(MODE_WRITE);
    return HD_DEVICES;
  case KEY_TAB:
    return HD_HOSTS;
  case 'c':
  case 'C':
    state = SHOW_INFO;
    return HD_DONE;
  case 'q':
  case 'Q':
    quit();
    return HD_DEVICES;
  case KEY_ESCAPE:
    return HD_DONE;
  default:
    return HD_DEVICES;
  }
}

void input_line_hosts_and_devices_host_slot(uint_fast8_t i, uint_fast8_t o, char *c)
{
  input_line(NAME_X, HOSTS_Y + i, o, c, sizeof(HostSlot), false);
}

/*
 * Select file
 */

static bool at_root(void)
{
  return path[0] == '/' && path[1] == 0;
}

SFSubState input_select_file_choose(void)
{
  static uint8_t k;
  static unsigned t;

  k = input();
  switch (k)
  {
  case KEY_UP_ARROW:
    if (bar_get() == 0 && pos > 0)
      return SF_PREV_PAGE;
    bar_up();
    return SF_CHOOSE;
  case KEY_DOWN_ARROW:
    if (bar_get() == (uint8_t)_visibleEntries - 1 && !dir_eof)
      return SF_NEXT_PAGE;
    bar_down();
    return SF_CHOOSE;
  case '<':
  case ',':
    if (pos > 0)
      return SF_PREV_PAGE;
    return SF_CHOOSE;
  case KEY_RIGHT_ARROW:
  case '>':
  case '.':
    if (!dir_eof)
      return SF_NEXT_PAGE;
    return SF_CHOOSE;
  case KEY_LEFT_ARROW:
    if (pos > 0)
      return SF_PREV_PAGE;
    if (!at_root())
      return SF_DEVANCE_FOLDER;
    copy_mode = false;
    state = HOSTS_AND_DEVICES;
    return SF_DONE;
  case KEY_BACKSPACE:
  case KEY_DELETE:
    if (!at_root())
      return SF_DEVANCE_FOLDER;
    return SF_CHOOSE;
  case KEY_RETURN:
    if (!_visibleEntries)
      return SF_CHOOSE;
    pos += bar_get();
    t = select_file_entry_type();
    if (t == ENTRY_TYPE_FOLDER)
      return SF_ADVANCE_FOLDER;
    if (t == ENTRY_TYPE_LINK)
      return SF_LINK;
    strncpy(source_path, path, 224);
    old_pos = pos;
    return SF_DONE;
  case 'f':
  case 'F':
    return SF_FILTER;
  case 'n':
  case 'N':
    if (copy_mode)
      return SF_CHOOSE;
    return SF_NEW;
  case 'c':
  case 'C':
    if (copy_mode)
      return SF_DONE;
    if (!_visibleEntries)
      return SF_CHOOSE;
    pos += bar_get();
    if (select_file_entry_type() != ENTRY_TYPE_FILE)
    {
      pos -= bar_get();
      ui_message("Only a file can be copied.");
      return SF_CHOOSE;
    }
    select_file_set_source_filename();
    copy_host_slot = selected_host_slot;
    return SF_COPY;
  case KEY_ESCAPE:
    copy_mode = false;
    state = HOSTS_AND_DEVICES;
    return SF_DONE;
  default:
    return SF_CHOOSE;
  }
}

void input_line_filter(char *c)
{
  input_line(ui_field_x, INFO_Y, 0, c, 32, false);
}

uint8_t input_select_file_new_type(void)
{
  return 1;
}

/* One size, IBM-3740 (system_create_new), so nothing to ask */
uint32_t input_select_file_new_size(uint8_t t)
{
  (void)t;
  return 250;
}

uint32_t input_select_file_new_custom(void)
{
  return 0;
}

static bool ends_imd(const char *c)
{
  static uint8_t n;

  n = (uint8_t)strlen(c);
  return n >= 4 && c[n - 4] == '.' && (c[n - 3] | 0x20) == 'i'
    && (c[n - 2] | 0x20) == 'm' && (c[n - 1] | 0x20) == 'd';
}

void input_select_file_new_name(char *c)
{
  input_line(ui_field_x, INFO_Y, 0, c, NEW_NAME_SIZE - 4, false);
  if (c[0] && !ends_imd(c))
    strcat(c, ".imd");
}

/*
 * Select slot
 */

SSSubState input_select_slot_choose(void)
{
  static uint8_t k, s;

  k = input();
  s = digit_slot(k);
  if (s != 0xFF)
  {
    bar_jump(s);
    selected_device_slot = bar_get();
    return SS_CHOOSE;
  }

  switch (k)
  {
  case KEY_UP_ARROW:
    bar_up();
    selected_device_slot = bar_get();
    return SS_CHOOSE;
  case KEY_DOWN_ARROW:
    bar_down();
    selected_device_slot = bar_get();
    return SS_CHOOSE;
  case 'e':
  case 'E':
    mounting = true;
    select_slot_eject(bar_get());
    mounting = false;
    return SS_CHOOSE;
  case KEY_RETURN:
  case 'r':
  case 'R':
    selected_device_slot = bar_get();
    /* A new disk is for writing on */
    mode = create ? MODE_WRITE : MODE_READ;
    return SS_DONE;
  case 'w':
  case 'W':
    selected_device_slot = bar_get();
    mode = MODE_WRITE;
    return SS_DONE;
  case KEY_ESCAPE:
    state = SELECT_FILE;
    backToFiles = true;
    return SS_ABORT;
  default:
    return SS_CHOOSE;
  }
}

/*
 * Copy
 */

DHSubState input_destination_host_slot_choose(void)
{
  static uint8_t k, s;

  k = input();
  s = digit_slot(k);
  if (s != 0xFF)
  {
    bar_jump(s);
    selected_host_slot = bar_get();
    return DH_CHOOSE;
  }

  switch (k)
  {
  case KEY_UP_ARROW:
    bar_up();
    selected_host_slot = bar_get();
    return DH_CHOOSE;
  case KEY_DOWN_ARROW:
    bar_down();
    selected_host_slot = bar_get();
    return DH_CHOOSE;
  case KEY_RETURN:
    selected_host_slot = bar_get();
    if (hostSlots[selected_host_slot][0] == 0)
    {
      vid_bell();
      return DH_CHOOSE;
    }
    strncpy(selected_host_name, (const char *)hostSlots[selected_host_slot], 31);
    selected_host_name[31] = 0;
    copy_mode = true;
    return DH_DONE;
  case KEY_ESCAPE:
    copy_mode = false;
    state = HOSTS_AND_DEVICES;
    return DH_ABORT;
  default:
    return DH_CHOOSE;
  }
}

/*
 * WiFi and info
 */

WSSubState input_set_wifi_select(void)
{
  static uint8_t k;

  k = input();
  switch (k)
  {
  case KEY_UP_ARROW:
    bar_up();
    return WS_SELECT;
  case KEY_DOWN_ARROW:
    bar_down();
    return WS_SELECT;
  case KEY_RETURN:
    if (bar_get() < numNetworks)
    {
      set_wifi_set_ssid(bar_get());
      return WS_PASSWORD;
    }
    return WS_CUSTOM;
  case 'r':
  case 'R':
    return WS_SCAN;
  case KEY_ESCAPE:
    state = HOSTS_AND_DEVICES;
    return WS_DONE;
  default:
    return WS_SELECT;
  }
}

void input_line_set_wifi_custom(char *c)
{
  memset(c, 0, SSID_MAXLEN);
  input_line(ui_field_x, INFO_Y, 0, c, SSID_MAXLEN, false);
}

void input_line_set_wifi_password(char *c)
{
  memset(c, 0, MAX_PASSWORD_LEN);
  input_line(ui_field_x, INFO_Y, 0, c, MAX_PASSWORD_LEN, true);
}

SISubState input_show_info(void)
{
  static uint8_t k;

  for (;;)
  {
    k = input();
    switch (k)
    {
    case 0:
      break;
    case 'r':
    case 'R':
      state = CONNECT_WIFI;
      return SI_DONE;
    case 's':
    case 'S':
      state = SET_WIFI;
      return SI_DONE;
    default:
      state = HOSTS_AND_DEVICES;
      return SI_DONE;
    }
  }
}

#endif /* BUILD_MODEL2 */
