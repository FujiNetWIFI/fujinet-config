#ifdef _CMOC_VERSION_

/**
 * File browser, slot selection, copy destination and new-image input.
 */

#include "coco_input.h"
#include "scroll.h"
#include "strendswith.h"
#include "../input.h"
#include "../globals.h"
#include "../key_codes.h"
#include "../constants.h"
#include "../select_file.h"
#include "../select_slot.h"

extern bool copy_mode;
extern unsigned char copy_host_slot;
extern unsigned short entry_timer;

void input_line_filter(char *c)
{
  input_line(FILTER_X, FILTER_Y, 0, c, 31, false);
}

unsigned char input_select_file_new_type(void)
{
  return 1;
}

unsigned long input_select_file_new_size(unsigned char t)
{
  char c[16];

  memset(c, 0, sizeof(c));
  input_line(MENU_X, STATUS_Y, 0, c, 16, false);

  return (long)atol(c);
}

unsigned long input_select_file_new_custom(void)
{
  return 0;
}

void input_select_file_new_name(char *c)
{
  input_line(MENU_X, STATUS_Y, 0, c, 255, false);

  if (!strendswith(c, ".dsk") && !strendswith(c, ".DSK"))
    strcat(c, ".DSK");
}

bool input_select_slot_build_eos_directory(void)
{
  return false;
}

void input_select_slot_build_eos_directory_label(char *c)
{
}

SFSubState input_select_file_choose(void)
{
  char k;
  unsigned entryType = 0;

  scroll_reset(true);

  while (true)
  {
    word now = getTimer();
    k = inkey();
    if (k == ' ')
    {
      color_toggle();
      k = KEY_REDRAW;
    }
    if (!k)
      k = input_handle_joystick();

    switch (k)
    {
    case 'C':
    case 'c':
      scroll_reset(false);
      if (copy_mode == true)
        return SF_DONE;
      pos += bar_get();
      select_file_set_source_filename();
      copy_host_slot = selected_host_slot;
      return SF_COPY;
    case 'F':
    case 'f':
      return SF_FILTER;
    case 'N':
    case 'n':
      return SF_NEW;
    case KEY_BREAK:
      state = HOSTS_AND_DEVICES;
      return SF_DONE;
    case KEY_REDRAW:
      scroll_reset(false);
      redraw_row = (byte)bar_get();
      dir_eof = false;
      screen_clear();
      return SF_DISPLAY;
    case KEY_LEFT_ARROW:
      return strcmp(path, "/") == 0 ? SF_CHOOSE : SF_DEVANCE_FOLDER;
    case KEY_RIGHT_ARROW:
    case KEY_ENTER:
      pos += bar_get();
      entryType = select_file_entry_type();
      if (entryType == ENTRY_TYPE_FOLDER)
        return SF_ADVANCE_FOLDER;
      else if (entryType == ENTRY_TYPE_LINK)
        return SF_LINK;
      else
        return SF_DONE;
    case KEY_UP_ARROW:
      scroll_reset(false);
      if ((bar_get() == 0) && (pos > 0))
        return SF_PREV_PAGE;
      entry_timer = ENTRY_TIMER_DUR;
      bar_up();
      select_display_long_filename();
      return SF_CHOOSE;
    case KEY_SHIFT_UP_ARROW:
      scroll_reset(false);
      if (pos > 0)
        return SF_PREV_PAGE;
      break;
    case KEY_DOWN_ARROW:
      scroll_reset(false);
      if ((bar_get() == ENTRIES_PER_PAGE - 1) && (dir_eof == false))
        return SF_NEXT_PAGE;
      entry_timer = ENTRY_TIMER_DUR;
      bar_down();
      select_display_long_filename();
      return SF_CHOOSE;
    case KEY_SHIFT_DOWN_ARROW:
      scroll_reset(false);
      if (dir_eof == false)
        return SF_NEXT_PAGE;
      break;
    }

    if ((word)(now - lastTimer) >= SCROLL_DELAY_TICKS)
    {
      lastTimer = now;

      if (idleCounter > 0)
        idleCounter--;
      else
        scroll_step();
    }
  }

  return SF_CHOOSE;
}

SSSubState input_select_slot_choose(void)
{
  char c = waitkey_joystick();

  switch (c)
  {
  case KEY_BREAK:
    state = HOSTS_AND_DEVICES;
    return SS_ABORT;
  case KEY_REDRAW:
    screen_clear();
    return SS_DISPLAY;
  case 'E':
  case 'e':
    select_slot_eject((char)bar_get());
    break;
  case KEY_ENTER:
  case 'R':
  case 'r':
    mode = MODE_READ;
    selected_device_slot = (char)bar_get();
    strncpy(source_path, path, 224);
    old_pos = pos;
    return SS_DONE;
  case 'W':
  case 'w':
    mode = MODE_WRITE;
    selected_device_slot = (char)bar_get();
    return SS_DONE;
  case KEY_UP_ARROW:
    bar_up();
    return SS_CHOOSE;
  case KEY_DOWN_ARROW:
    bar_down();
    return SS_CHOOSE;
  default:
    return SS_CHOOSE;
  }
  return SS_CHOOSE;
}

unsigned char input_select_slot_mode(char *mode)
{
  return 1;
}

DHSubState input_destination_host_slot_choose(void)
{
  char k = waitkey_joystick();

  switch (k)
  {
  case KEY_BREAK:
    state = HOSTS_AND_DEVICES;
    return DH_ABORT;
  case KEY_REDRAW:
    screen_clear();
    return DH_INIT;
  case KEY_UP_ARROW:
    bar_up();
    return DH_CHOOSE;
  case KEY_DOWN_ARROW:
    bar_down();
    return DH_CHOOSE;
  case KEY_ENTER:
    selected_host_slot = (unsigned char)bar_get();
    copy_mode = true;
    strcpy((char *)selected_host_name, (char *)hostSlots[selected_host_slot]);
    return DH_DONE;
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
    bar_jump(k - '1');
    return DH_CHOOSE;
  default:
    return DH_CHOOSE;
  }
}

void set_device_slot_mode(unsigned char slot, unsigned char mode)
{
}

#endif
