#ifdef _CMOC_VERSION_

/**
 * Hosts and devices screen, the lobby prompt and the mounting screen.
 */

#include "coco_screen.h"
#include "../globals.h"
#include "../constants.h"

/* The boxes are drawn once per screen_clear(); later calls only refresh the rows. */
void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d, bool *e)
{
  static bool drawn;
  static unsigned drawn_generation;

  if (!drawn || drawn_generation != screen_generation)
  {
    screen_frame("FujiNet Config");
    screen_box(0, HOSTS_BOX_Y, SCREEN_COLS, 10, "HOST SLOTS");
    screen_box(0, DEVICES_BOX_Y, SCREEN_COLS, 6, "DRIVE SLOTS");
    drawn = true;
    drawn_generation = screen_generation;
  }
  screen_hosts_and_devices_host_slots(h);
  screen_hosts_and_devices_device_slots(DEVICES_Y, d, e);
}

void screen_hosts_and_devices_hosts(void)
{
  screen_menu_clear();
  moveCursor(MENU_X, MENU_Y1);
  screen_print_menu("1-8", " Slot  ");
  screen_print_menu("E", "dit  ");
  screen_print_menu("ENTER", " Browse  ");
  screen_print_menu("L", "obby");
  moveCursor(MENU_X, MENU_Y2);
  screen_print_menu("C", "onfig  ");
  screen_print_menu("SHIFT-DN", " Drives  ");
  screen_print_menu("BREAK", " Quit");
  bar_set(HOSTS_Y, 1, NUM_HOST_SLOTS, selected_host_slot);
}

void screen_hosts_and_devices_devices(void)
{
  screen_clear_line(ASK_Y);
  screen_menu_clear();
  moveCursor(MENU_X, MENU_Y1);
  screen_print_menu("0-3", " Slot  ");
  screen_print_menu("E", "ject  ");
  screen_print_menu("R", "ead  ");
  screen_print_menu("W", "rite  ");
  screen_print_menu("C", "onfig");
  moveCursor(MENU_X, MENU_Y2);
  screen_print_menu("CLEAR", " All Slots  ");
  screen_print_menu("SHIFT-UP", " Hosts");
  screen_hosts_and_devices_device_slots(DEVICES_Y, &deviceSlots[0], &deviceEnabled[0]);
  bar_set(DEVICES_Y, 1, NUM_DEVICE_SLOTS, selected_device_slot);
}

void screen_hosts_and_devices_devices_clear_all(void)
{
  screen_puts((SCREEN_COLS - 28) / 2, ASK_Y, "EJECTING ALL... PLEASE WAIT.");
}

void screen_hosts_and_devices_edit_host_slot(int i)
{
}

void screen_hosts_and_devices_eject(unsigned char ds)
{
  screen_hosts_and_devices_devices();
}

void screen_hosts_and_devices_long_filename(const char *f)
{
}

void screen_mount_and_boot(void)
{
  screen_clear();
  screen_title("FujiNet Config");
  moveCursor(0, 2);
}

bool screen_mount_and_boot_lobby(void)
{
  unsigned char k;

  screen_puts(MENU_X, ASK_Y, "Boot to lobby? Y/N");
  k = waitkey(0);
  screen_clear_line(ASK_Y);
  if (k != 'Y' && k != 'y')
    return false;

  screen_puts(MENU_X, ASK_Y, "Booting lobby...");
  return true;
}

#endif
