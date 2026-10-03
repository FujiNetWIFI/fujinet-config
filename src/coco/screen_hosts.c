#ifdef _CMOC_VERSION_

/**
 * Hosts and devices screen, the lobby prompt and the exit to the text screen for mounting.
 */

#include "coco_screen.h"
#include "../globals.h"
#include "../constants.h"

/* What the rows last showed. The shared loop calls screen_hosts_and_devices() on every switch
   between hosts and drives; rows are only redrawn when their data changed. */
static HostSlot shown_hosts[NUM_HOST_SLOTS];
static DeviceSlot shown_devices[NUM_DEVICE_SLOTS];
static bool hosts_stale;

/* The boxes are drawn once per screen_clear(). */
void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d, bool *e)
{
  static bool drawn;
  static unsigned drawn_generation;
  bool fresh = !drawn || drawn_generation != screen_generation;

  if (fresh)
  {
    screen_frame("FujiNet Config");
    screen_box(0, HOSTS_BOX_Y, SCREEN_COLS, 10, "HOST SLOTS");
    screen_box(0, DEVICES_BOX_Y, SCREEN_COLS, 6, "DRIVE SLOTS");
    drawn = true;
    drawn_generation = screen_generation;
  }
  if (fresh || hosts_stale || memcmp(shown_hosts, h, sizeof(shown_hosts)))
  {
    screen_hosts_and_devices_host_slots(h);
    memcpy(shown_hosts, h, sizeof(shown_hosts));
    hosts_stale = false;
  }
  if (fresh || memcmp(shown_devices, d, sizeof(shown_devices)))
  {
    screen_hosts_and_devices_device_slots(DEVICES_Y, d, e);
    memcpy(shown_devices, d, sizeof(shown_devices));
  }
}

/* The drive-slot menu is redrawn only when it is not already showing (it is after CLEAR's ejects). */
static bool devices_menu;
static unsigned devices_menu_generation;

void screen_hosts_and_devices_hosts(void)
{
  devices_menu = false;
  screen_menu_clear();
  screen_move(MENU_X, MENU_Y1);
  screen_print_menu("1-8", " Slot  ");
  screen_print_menu("E", "dit  ");
  screen_print_menu("ENTER", " Browse  ");
  screen_print_menu("L", "obby");
  screen_move(MENU_X, MENU_Y2);
  screen_print_menu("C", "onfig  ");
  screen_print_menu("SHIFT-DN", " Drives  ");
  screen_print_menu("BREAK", " Quit");
  bar_set(HOSTS_Y, 1, NUM_HOST_SLOTS, selected_host_slot);
}

void screen_hosts_and_devices_devices(void)
{
  screen_clear_line(ASK_Y);
  if (!devices_menu || devices_menu_generation != screen_generation)
  {
    screen_menu_clear();
    screen_move(MENU_X, MENU_Y1);
    screen_print_menu("0-3", " Slot  ");
    screen_print_menu("E", "ject  ");
    screen_print_menu("R", "ead  ");
    screen_print_menu("W", "rite  ");
    screen_print_menu("C", "onfig");
    screen_move(MENU_X, MENU_Y2);
    screen_print_menu("CLEAR", " All Slots  ");
    screen_print_menu("SHIFT-UP", " Hosts");
    devices_menu = true;
    devices_menu_generation = screen_generation;
  }
  bar_set(DEVICES_Y, 1, NUM_DEVICE_SLOTS, selected_device_slot);
}

void screen_hosts_and_devices_devices_clear_all(void)
{
  screen_puts((SCREEN_COLS - 28) / 2, ASK_Y, "EJECTING ALL... PLEASE WAIT.");
}

/* The line editor draws over the row, so redraw it after the edit even if the name is unchanged. */
void screen_hosts_and_devices_edit_host_slot(int i)
{
  hosts_stale = true;
}

void screen_hosts_and_devices_eject(unsigned char ds)
{
  screen_hosts_and_devices_device_slots(DEVICES_Y, &deviceSlots[0], &deviceEnabled[0]);
}

void screen_hosts_and_devices_long_filename(const char *f)
{
}

void screen_mount_and_boot(void)
{
  screen_leave_graphics();
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
