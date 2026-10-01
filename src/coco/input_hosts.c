#ifdef _CMOC_VERSION_

/**
 * Hosts and devices input.
 */

#include "coco_input.h"
#include "mount_and_boot.h"
#include "../input.h"
#include "../globals.h"
#include "../key_codes.h"
#include "../hosts_and_devices.h"

HDSubState input_hosts_and_devices_hosts(void)
{
  char k = waitkey_joystick();

  switch (k)
  {
  case KEY_BREAK:
    screen_mount_and_boot();
    return HD_DONE;
  case KEY_UP_ARROW:
    bar_up();
    break;
  case KEY_DOWN_ARROW:
    bar_down();
    break;
  case KEY_ENTER:
    selected_host_slot = (unsigned char)bar_get();
    if (hostSlots[selected_host_slot][0] != 0)
    {
      strcpy((char *)selected_host_name, (char *)hostSlots[selected_host_slot]);
      state = SELECT_FILE;
      return HD_DONE;
    }
    return HD_HOSTS;
  case KEY_SHIFT_DOWN_ARROW:
  case KEY_LEFT_ARROW:
    return HD_DEVICES;
  case 'c':
  case 'C':
    state = SHOW_INFO;
    return HD_DONE;
  case 'e':
  case 'E':
    hosts_and_devices_edit_host_slot(bar_get());
    bar_jump(selected_host_slot);
    return HD_HOSTS;
  case 'l':
  case 'L':
    mount_and_boot_lobby();
    return HD_HOSTS;
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
    bar_jump(k - '1');
    break;
  case KEY_REDRAW:
    selected_host_slot = (char)bar_get();
    screen_clear();
    screen_hosts_and_devices(&hostSlots[0], deviceSlots, deviceEnabled);
    screen_hosts_and_devices_hosts();
    break;
  }
  return HD_HOSTS;
}

HDSubState input_hosts_and_devices_devices(void)
{
  char k = waitkey_joystick();

  switch (k)
  {
  case 'C':
  case 'c':
    state = SHOW_INFO;
    return HD_DONE;
  case 'E':
  case 'e':
    hosts_and_devices_eject((byte)bar_get());
    break;
  case 'L':
  case 'l':
    mount_and_boot_lobby();
    return HD_DEVICES;
  case 'R':
  case 'r':
    selected_device_slot = (byte)bar_get();
    hosts_and_devices_devices_set_mode(MODE_READ);
    return HD_DEVICES;
  case 'W':
  case 'w':
    selected_device_slot = (byte)bar_get();
    hosts_and_devices_devices_set_mode(MODE_WRITE);
    return HD_DEVICES;
  case KEY_BREAK:
    screen_mount_and_boot();
    return HD_DONE;
  case KEY_UP_ARROW:
    bar_up();
    break;
  case KEY_DOWN_ARROW:
    bar_down();
    break;
  case KEY_CLEAR:
    return HD_CLEAR_ALL_DEVICES;
  case KEY_SHIFT_UP_ARROW:
  case KEY_LEFT_ARROW:
    return HD_HOSTS;
  case KEY_REDRAW:
    selected_device_slot = (char)bar_get();
    screen_clear();
    screen_hosts_and_devices(&hostSlots[0], deviceSlots, deviceEnabled);
    screen_hosts_and_devices_devices();
    break;
  case KEY_0:
  case KEY_1:
  case KEY_2:
  case KEY_3:
    bar_jump(k - '0');
    break;
  }
  return HD_DEVICES;
}

void input_line_hosts_and_devices_host_slot(uint_fast8_t i, uint_fast8_t o, char *c)
{
  char orig[32];

  bar_clear(false);
  memcpy(orig, c, sizeof(orig));
  input_aborted = false;
  input_abortable = true;
  input_line(LIST_X + 4, HOSTS_Y + (unsigned char)i, 0, c, 32, false);
  input_abortable = false;
  if (input_aborted)
    memcpy(c, orig, sizeof(orig));
}

#endif
