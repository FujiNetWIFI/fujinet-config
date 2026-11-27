#ifdef BUILD_TRS80M1

/**
 * @file input.c
 * @brief Keyboard input and the screens' key handling - TRS-80 Model I.
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
#include "../mount_and_boot.h"
#include "../select_file.h"
#include "../select_slot.h"
#include "../set_wifi.h"
#include "vid.h"
#include "trs80m1.h"
#include "kbd.h"
#include "layout.h"
#include "ui.h"

extern unsigned char copy_host_slot;

bool mounting = false;
static bool line_cancelled;

uint8_t input(void)
{
  static uint8_t key;

  key = kbd_get();
  if (!key)
  {
    vid_flush();
    key = kbd_get();
  }
  return key;
}

/** BREAK on the hosts or drives: mount every drive and boot slot 1, which is
    drive 0, the one the ROM boots from. Returns only if slot 1 can't boot. */
static void boot(void)
{
  if (deviceSlots[0].hostSlot >= FUJI_MAX_HOSTS || !deviceSlots[0].file[0])
  {
    ui_message("Slot 1 is empty: mount an image in it to boot.");
    return;
  }
  mount_and_boot();
  screen_hosts_and_devices_again();
}

/**
 * Edit text in a field at col,row. size is the size of text, so at most size-1
 * characters.
 * ENTER keeps the text, BREAK puts the old text back.
 */
void input_line(uint8_t col, uint8_t row, uint8_t offset, char *text, uint8_t size, bool password)
{
  static char orig[256];
  static uint8_t len, width, key, idx, shown, from;
  static bool done;

  (void)offset;
  line_cancelled = false;
  if (!size)
    return;

  strncpy(orig, text, sizeof(orig) - 1);
  orig[sizeof(orig) - 1] = 0;
  text[size - 1] = 0;
  len = (uint8_t)strlen(text);

  /* The field runs to the right edge at most, with a cell for the cursor
     after the text */
  width = size;
  if (width > VID_COLS - col)
    width = VID_COLS - col;

  done = false;
  while (!done)
  {
    /* Show the end of the text when it is longer than the field */
    from = len >= width ? len - width + 1 : 0;
    shown = len - from;
    vid_fill(col, row, width, ' ');
    for (idx = 0; idx < shown; idx++)
      vid_text(col + idx, row, password ? "*" : &text[from + idx], 1, false);
    vid_cursor(col + shown, row);

    key = kbd_wait();
    switch (key)
    {
    case KEY_RETURN:
      done = true;
      break;
    case KEY_ESCAPE:
      strncpy(text, orig, size - 1);
      text[size - 1] = 0;
      len = (uint8_t)strlen(text);
      line_cancelled = true;
      done = true;
      break;
    case KEY_BACKSPACE:
      if (len > 0)
        text[--len] = 0;
      break;
    default:
      if (key >= KEY_SPACE && key <= 0x7E && len < size - 1)
      {
        text[len++] = key;
        text[len] = 0;
      }
      break;
    }
  }

  /* Leave the text behind, without the cursor */
  vid_fill(col, row, width, ' ');
  for (idx = 0; idx < len && idx < width; idx++)
    vid_text(col + idx, row, password ? "*" : &text[idx], 1, false);
  vid_cursor(VID_NONE, 0);
  vid_flush();
}

/*
 * Hosts and devices
 */

static uint8_t digit_slot(uint8_t key)
{
  if (key >= '1' && key <= '8')
    return key - '1';
  return 0xFF;
}

HDSubState input_hosts_and_devices_hosts(void)
{
  static uint8_t key, slot;

  key = input();
  slot = digit_slot(key);
  if (slot != 0xFF)
  {
    bar_jump(slot);
    selected_host_slot = bar_get();
    return HD_HOSTS;
  }

  switch (key)
  {
  case KEY_UP_ARROW:
    bar_up();
    selected_host_slot = bar_get();
    return HD_HOSTS;
  case KEY_DOWN_ARROW:
    /* Off the bottom of the hosts into the drives */
    if (bar_get() == FUJI_MAX_HOSTS - 1)
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
  case KEY_RIGHT_ARROW:
    return HD_DEVICES;
  case 'c':
  case 'C':
    state = SHOW_INFO;
    return HD_DONE;
  case 'k':
  case 'K':
    card_settings();
    screen_hosts_and_devices_again();
    return HD_HOSTS;
  case KEY_ESCAPE:
    boot();
    return HD_HOSTS;
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
static void devices_set_mode(uint8_t mode_bits)
{
  static char saved_host;

  selected_device_slot = bar_get();
  if (deviceSlots[selected_device_slot].hostSlot >= FUJI_MAX_HOSTS
      || !deviceSlots[selected_device_slot].file[0])
    return;

  saved_host = selected_host_slot;
  selected_host_slot = deviceSlots[selected_device_slot].hostSlot;
  hosts_and_devices_devices_set_mode(mode_bits);
  selected_host_slot = saved_host;

  screen_drive_row(DRIVES_Y + selected_device_slot, selected_device_slot);
}

HDSubState input_hosts_and_devices_devices(void)
{
  static uint8_t key, slot;

  key = input();
  slot = digit_slot(key);
  if (slot != 0xFF)
  {
    bar_jump(slot);
    devices_moved();
    return HD_DEVICES;
  }

  switch (key)
  {
  case KEY_UP_ARROW:
    /* Off the top of the drives into the hosts */
    if (bar_get() == 0)
    {
      selected_host_slot = FUJI_MAX_HOSTS - 1;
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
    devices_set_mode(FUJI_DISK_MODE_READ);
    return HD_DEVICES;
  case 'w':
  case 'W':
    devices_set_mode(FUJI_DISK_MODE_WRITE);
    return HD_DEVICES;
  case KEY_LEFT_ARROW:
    return HD_HOSTS;
  case 'c':
  case 'C':
    state = SHOW_INFO;
    return HD_DONE;
  case 'k':
  case 'K':
    card_settings();
    screen_hosts_and_devices_again();
    return HD_DEVICES;
  case KEY_ESCAPE:
    boot();
    return HD_DEVICES;
  default:
    return HD_DEVICES;
  }
}

void input_line_hosts_and_devices_host_slot(uint_fast8_t idx, uint_fast8_t offset, char *text)
{
  input_line(NAME_X, HOSTS_Y + idx, offset, text, sizeof(HostSlot), false);
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
  static uint8_t key;
  static unsigned entry_type;

  key = input();
  switch (key)
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
    if (!at_root())
      return SF_DEVANCE_FOLDER;
    return SF_CHOOSE;
  case KEY_RETURN:
    if (!_visibleEntries)
      return SF_CHOOSE;
    pos += bar_get();
    entry_type = select_file_entry_type();
    if (entry_type == ENTRY_TYPE_FOLDER)
      return SF_ADVANCE_FOLDER;
    if (entry_type == ENTRY_TYPE_LINK)
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

void input_line_filter(char *text)
{
  input_line(ui_field_x, INFO_Y, 0, text, 32, false);
}

uint8_t input_select_file_new_type(void)
{
  return 1;
}

/* One size, a 35-track JV1 (system_create_new), so nothing to ask */
uint32_t input_select_file_new_size(uint8_t entry_type)
{
  (void)entry_type;
  return 88;
}

uint32_t input_select_file_new_custom(void)
{
  return 0;
}

static bool ends_dsk(const char *text)
{
  static uint8_t len;

  len = (uint8_t)strlen(text);
  return len >= 4 && text[len - 4] == '.' && (text[len - 3] | 0x20) == 'd'
    && (text[len - 2] | 0x20) == 's' && (text[len - 1] | 0x20) == 'k';
}

void input_select_file_new_name(char *text)
{
  input_line(ui_field_x, INFO_Y, 0, text, NEW_NAME_SIZE - 4, false);
  if (text[0] && !ends_dsk(text))
    strcat(text, ".dsk");
}

/*
 * Select slot
 */

SSSubState input_select_slot_choose(void)
{
  static uint8_t key, slot;

  key = input();
  slot = digit_slot(key);
  if (slot != 0xFF)
  {
    bar_jump(slot);
    selected_device_slot = bar_get();
    return SS_CHOOSE;
  }

  switch (key)
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
    mode = FUJI_DISK_MODE_READ;
    return SS_DONE;
  case 'w':
  case 'W':
    selected_device_slot = bar_get();
    mode = FUJI_DISK_MODE_WRITE;
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
  static uint8_t key, slot;

  key = input();
  slot = digit_slot(key);
  if (slot != 0xFF)
  {
    bar_jump(slot);
    selected_host_slot = bar_get();
    return DH_CHOOSE;
  }

  switch (key)
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
      ui_message("That host slot is empty.");
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
  static uint8_t key;

  key = input();
  switch (key)
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

/* ESC goes back to the network list */
void input_line_set_wifi_custom(char *text)
{
  memset(text, 0, SSID_MAXLEN);
  input_line(ui_field_x, INFO_Y, 0, text, SSID_MAXLEN, false);
  if (line_cancelled)
    ws_subState = WS_SELECT;
}

void input_line_set_wifi_password(char *text)
{
  memset(text, 0, MAX_PASSWORD_LEN);
  input_line(ui_field_x, INFO_Y, 0, text, MAX_PASSWORD_LEN, true);
  if (line_cancelled)
    ws_subState = WS_SELECT;
}

SISubState input_show_info(void)
{
  static uint8_t key;

  for (;;)
  {
    key = input();
    switch (key)
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
    case 'k':
    case 'K':
      card_settings();
      state = SHOW_INFO;
      return SI_DONE;
    default:
      state = HOSTS_AND_DEVICES;
      return SI_DONE;
    }
  }
}

#endif /* BUILD_TRS80M1 */
