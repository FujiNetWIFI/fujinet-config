/**
 * Hosts and Devices
 */

#include "hosts_and_devices.h"
#include "die.h"
#include "debug.h"
#include "screen.h"
#include "compat.h"
#include "input.h"
#include "constants.h"
#include "globals.h"

HDSubState hd_subState;
DeviceSlot deviceSlots[8];
DeviceSlot temp_deviceSlot;
bool deviceEnabled[8];
HostSlot hostSlots[8];
char selected_host_slot = 0;
char selected_device_slot = 0;
char selected_host_name[32];
char temp_filename[256];

extern bool quick_boot;

void hosts_and_devices_edit_host_slot(uint_fast8_t i)
{
  uint_fast8_t o;
  HostSlot orig_host;

  if (strlen((const char *)hostSlots[i]) == 0)
  {
    screen_hosts_and_devices_clear_host_slot(i);
    o = 0;
  }
  else
    o = strlen((const char *)hostSlots[i]);

  screen_hosts_and_devices_edit_host_slot(i);
  // FRUSTRATINGLY the signature is void return, so noone ever knows if the return was good or bad
  // and just carries on and saves changes anyway. Fortunately ESC now handled well (on atari) and will save same thing back to FN

  // Save off original value of host slot so we can check if it ws modified.
  memcpy(orig_host, hostSlots[i], sizeof(HostSlot));
  input_line_hosts_and_devices_host_slot(i, o, (char *)hostSlots[i]);

  if (strlen((const char*)hostSlots[i]) == 0)
    screen_hosts_and_devices_host_slot_empty(i);

  // if host entry has changed, eject any disks that were mounted from the host slot since they won't be valid anymore.
  if ( memcmp(orig_host, hostSlots[i], sizeof(HostSlot)))
  {
    // re-use 'o' here to save a little memory. If it's original value is needed in some future enhancement,
    // declare a new variable for the loop counter.
    for (o = 0; o<NUM_DEVICE_SLOTS; o++)
    {
      if ( deviceSlots[o].hostSlot == i )
      {
        hosts_and_devices_eject((unsigned char) o);
      }
    }
  }

  fuji_put_host_slots(&hostSlots[0], NUM_HOST_SLOTS);

  // Need to re-render both hosts and devices because some devices
  // may have been eject if they belonged to old host
#ifdef OBSOLETE
  screen_hosts_and_devices_hosts();
#else
  screen_hosts_and_devices(&hostSlots[0], deviceSlots, deviceEnabled);
  screen_hosts_and_devices_hosts();
#endif // OBSOLETE
}

void hosts_and_devices_hosts(void)
{
  fuji_update_devices_enabled(deviceEnabled, NUM_DEVICE_SLOTS);

  if (!quick_boot)
  	screen_hosts_and_devices_hosts();

  while (hd_subState == HD_HOSTS)
    hd_subState = input_hosts_and_devices_hosts();
}

void hosts_and_devices_long_filename(void)
{
  fuji_get_device_filename(selected_device_slot, response);
  screen_hosts_and_devices_long_filename(response);
}

void hosts_and_devices_eject(unsigned char ds)
{
  fuji_unmount_disk_image(ds);
#ifdef OBSOLETE
  memset(deviceSlots[ds].file, 0, FILE_MAXLEN);
  deviceSlots[ds].hostSlot = 0xFF;
  deviceSlots[ds].mode = 0;
  fuji_put_device_slots(deviceSlots, NUM_DEVICE_SLOTS);
#endif // OBSOLETE
  fuji_get_device_slots(deviceSlots, NUM_DEVICE_SLOTS);
  screen_hosts_and_devices_eject(ds);
#ifdef OBSOLETE
  hosts_and_devices_long_filename();
#endif // OBSOLETE
}

void hosts_and_devices_devices_clear_all(void)
{
  char i;

  screen_hosts_and_devices_devices_clear_all();

  // grouping this assumes same number of device and host slots..
  for (i = 0; i < NUM_DEVICE_SLOTS; i++)
    hosts_and_devices_eject(i);

  hd_subState = HD_DEVICES;
}

void hosts_and_devices_devices(void)
{
  char k = 0;

  fuji_update_devices_enabled(deviceEnabled, NUM_DEVICE_SLOTS);
  screen_hosts_and_devices_devices();
  hosts_and_devices_long_filename();

  while (hd_subState == HD_DEVICES)
    hd_subState = input_hosts_and_devices_devices();
}

void hosts_and_devices_devices_set_mode(unsigned char m)
{
#if defined(BUILD_ATARI)
  bool mnt;
  char err_msg[64];
#elif defined(BUILD_APPLE2)
  bool mnt = false;
#endif

  memset(temp_filename, 0, sizeof(temp_filename));

  // Stow device slot temporarily
  memcpy(&temp_deviceSlot, &deviceSlots[selected_device_slot], sizeof(DeviceSlot));
  temp_deviceSlot.mode = m;
  fuji_get_device_filename(selected_device_slot, temp_filename);

  // unmount disk image
  fuji_unmount_disk_image(selected_device_slot);

  // copy device slot back in.
  memcpy(&deviceSlots[selected_device_slot], &temp_deviceSlot, sizeof(DeviceSlot));
  fuji_set_device_filename(selected_device_slot, selected_host_slot, m, temp_filename);

  fuji_put_device_slots(deviceSlots, NUM_DEVICE_SLOTS);

  // Make sure host slot is mounted or it will fail mounting disk
  fuji_mount_host_slot(deviceSlots[selected_device_slot].hostSlot);

  // Remount
#if defined(BUILD_ATARI)
  mnt = fuji_mount_disk_image(selected_device_slot, m);

  // Check for error
  if (!mnt)
  {
    unsigned int i;
    // Display error for a moment then redraw menu after
    strcpy(err_msg, "ERROR SETTING DISK MODE: ");
    screen_error(err_msg);
    for (i = 0; i < 6000; i++)
      // Do nothing to let the message display

    // likely failed on setting write mode, make it read only
    temp_deviceSlot.mode = MODE_READ;
    memcpy(&deviceSlots[selected_device_slot], &temp_deviceSlot, sizeof(DeviceSlot));
    fuji_put_device_slots(deviceSlots, NUM_DEVICE_SLOTS);
    screen_error(""); // clear the error msg
  }
  screen_hosts_and_devices_device_slots(DEVICES_START_Y, deviceSlots, deviceEnabled); // redraw the disks
#elif defined(BUILD_APPLE2)
  mnt = fuji_mount_disk_image(selected_device_slot, m);

  // Check for error
  if (!mnt)
  {
    unsigned int i;
    // Display error for a moment then redraw menu after
    screen_error("Error setting disk mode");
    for (i = 0; i < 4000; i++)
      mnt = true; // Do nothing to let the message display
    // likely failed on setting write mode, make it read only
    temp_deviceSlot.mode = MODE_READ;
    memcpy(&deviceSlots[selected_device_slot], &temp_deviceSlot, sizeof(DeviceSlot));
    fuji_put_device_slots(deviceSlots, NUM_DEVICE_SLOTS);
  }
  screen_hosts_and_devices_device_slots(11, deviceSlots, deviceEnabled); // redraw the disks
  //screen_hosts_and_devices_devices_selected(selected_device_slot); // Breaks disk order on screen??
  selected_device_slot = 0; // Go back to drive 0 instead
  hosts_and_devices_devices();
#elif defined(_CMOC_VERSION_)
    fuji_mount_disk_image(selected_device_slot, m);
    screen_hosts_and_devices_device_slots(1,deviceSlots,deviceEnabled);
    bar_jump(selected_device_slot);
#else
    fuji_mount_disk_image(selected_device_slot, m);
#endif
}

void hosts_and_devices_devices_enable_toggle(unsigned char ds)
{
  bool s = fuji_get_device_enabled_status(io_device_slot_to_device(ds));

  if (s == true)
    fuji_disable_device(fuji_device_slot_to_device(ds));
  else
    fuji_enable_device(fuji_device_slot_to_device(ds));

  deviceEnabled[ds] = fuji_get_device_enabled_status(io_device_slot_to_device(ds));

  fuji_update_devices_enabled(deviceEnabled, NUM_DEVICE_SLOTS);
  screen_hosts_and_devices_device_slots(11, deviceSlots, deviceEnabled);
  bar_update();
}

void hosts_and_devices_done(void)
{
  char i;
  // char *msg[40];
#ifdef _CMOC_VERSION_
  cls(1);
  printf("MOUNTING DISKS...\n\n");
#endif

#ifdef BUILD_APPLE2
  // Display Loading message since TNFS Disk II images can take some time
  char s;
  clrscr();
  cprintf("MOUNTING DISKS...\r\n\r\n");
#endif
#ifdef BUILD_PMD85
  char y = 1;
  char s = '0';
  clrscr();
  gotoxy(SCR_X0, SCR_Y0);
  textcolor(LIST_TITLE_COLOR); cprintf("MOUNTING DEVICES...");
#endif
  for (i = 0; i < NUM_DEVICE_SLOTS; i++) // 4 for apple for now, what about adam? 8 for atari?
  {
    if (deviceSlots[i].hostSlot != 0xFF)
    {
#ifdef _CMOC_VERSION_
      printf("%d:%s\n",i,deviceSlots[i].file);
#endif
#ifdef BUILD_APPLE2
      s = i + 1;
      cprintf("                DISK %d\r\n", s);
#endif
#ifdef BUILD_PMD85
      if (i == 2) s = 'T';
      else if (i == 3) s = 'M';
      else s++;
      y++;
      gotoxy(SCR_X0, SCR_Y0 + y);
      textcolor(LIST_VBAR_COLOR); cprintf("%c:", s);
      textcolor(TEXT_COLOR); cprintf("%s", deviceSlots[i].file);
#endif
      fuji_mount_host_slot(deviceSlots[i].hostSlot);
      fuji_mount_disk_image(i, deviceSlots[i].mode);
    }
  }

  state = DONE;
}

void hosts_and_devices(void)
{
  if (quick_boot == true)
    hd_subState = HD_DONE;
  else
    hd_subState = HD_HOSTS;

#ifdef BUILD_PMD85
    fuji_get_host_slots(hostSlots, NUM_HOST_SLOTS);
    fuji_get_device_slots(deviceSlots, NUM_DEVICE_SLOTS);
    fuji_update_devices_enabled(deviceEnabled, NUM_DEVICE_SLOTS);
    screen_hosts_and_devices(hostSlots, deviceSlots, deviceEnabled);
#endif

  while (state == HOSTS_AND_DEVICES)
  {
#ifndef BUILD_PMD85
    fuji_get_host_slots(&hostSlots[0], NUM_HOST_SLOTS);
    fuji_get_device_slots(deviceSlots, NUM_DEVICE_SLOTS);
    fuji_update_devices_enabled(deviceEnabled, NUM_DEVICE_SLOTS);
    screen_hosts_and_devices(&hostSlots[0], deviceSlots, deviceEnabled);
#endif

    switch (hd_subState)
    {
    case HD_HOSTS:
      hosts_and_devices_hosts();
      break;
    case HD_DEVICES:
      hosts_and_devices_devices();
      break;
    case HD_CLEAR_ALL_DEVICES:
      hosts_and_devices_devices_clear_all();
      break;
    case HD_DONE:
      state = DONE;
      break;
    }
  }

  if (state == DONE) {
    hosts_and_devices_done();
  }
}
