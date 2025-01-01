/**
 * FujiNet for #Adam configuration program
 *
 * Hosts and Devices
 */

#include "debug.h"

#ifdef _CMOC_VERSION_
#include <cmoc.h>
#include "coco/stdbool.h"
#include "coco/globals.h"
#include "coco/io.h"
#include "coco/screen.h"
#include "coco/input.h"
#include "coco/bar.h"
#else
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#endif /* _CMOC_VERSION_ */
#include "hosts_and_devices.h"
#include "die.h"
#include "typedefs.h"
#include "fuji_typedefs.h"

#ifdef BUILD_ADAM
#include "adam/globals.h"
#include "adam/io.h"
#include "adam/screen.h"
#include "adam/input.h"
#include "adam/bar.h"
#endif /* BUILD_ADAM */

#ifdef BUILD_APPLE2
#include "apple2/globals.h"
#include "apple2/io.h"
#include "apple2/screen.h"
#include "apple2/input.h"
#include "apple2/bar.h"
#ifdef __ORCAC__
#include <coniogs.h>
#else
#include <conio.h>
#endif
// #include <stdio.h> // for debugging using sprintf

extern uint8_t sp_error;
#endif /* BUILD_APPLE2 */

#ifdef BUILD_ATARI
#include "atari/globals.h"
#include "atari/io.h"
#include "atari/screen.h"
#include "atari/input.h"
#include "atari/bar.h"
#endif /* BUILD_ATARI */

#ifdef BUILD_C64
#include "c64/globals.h"
#include "c64/io.h"
#include "c64/screen.h"
#include "c64/input.h"
#include "c64/bar.h"
#endif /* BUILD_C64 */

#ifdef BUILD_PC8801
#include "pc8801/globals.h"
#include "pc8801/io.h"
#include "pc8801/screen.h"
#include "pc8801/input.h"
#include "pc8801/bar.h"
#endif /* BUILD_PC8801 */

#ifdef BUILD_PC6001
#include "pc6001/globals.h"
#include "pc6001/io.h"
#include "pc6001/screen.h"
#include "pc6001/input.h"
#include "pc6001/bar.h"
#endif /* BUILD_PC6001 */

#ifdef BUILD_PMD85
#include "pmd85/globals.h"
#include "pmd85/io.h"
#include "pmd85/screen.h"
#include "pmd85/input.h"
#include "pmd85/bar.h"
#include "pmd85/colors.h"
#include <conio.h>
#endif /* BUILD_PMD85 */

#ifdef BUILD_RC2014
#include "rc2014/globals.h"
#include "rc2014/io.h"
#include "rc2014/screen.h"
#include "rc2014/input.h"
#include "rc2014/bar.h"
#endif /* BUILD_RC2014 */

HDSubState hd_subState=HD_HOSTS;
DeviceSlot deviceSlots[8];
DeviceSlot temp_deviceSlot;
bool deviceEnabled[8];
HostSlot hostSlots[8];
char selected_host_slot = 0;
char selected_device_slot = 0;
char selected_host_name[32];
char temp_filename[256];

extern bool quick_boot;

#ifdef _CMOC_VERSION_
void hosts_and_devices_edit_host_slot(int i)
#else
void hosts_and_devices_edit_host_slot(unsigned char i)
#endif
{
#ifdef _CMOC_VERSION_
  unsigned int o;
#else
  unsigned char o;
#endif
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
    for ( o = 0; o<NUM_DEVICE_SLOTS; o++)
    {
      if ( deviceSlots[o].hostSlot == i )
      {
        hosts_and_devices_eject(o);
      }
    }
  } 

  io_put_host_slots(&hostSlots[0]);
  screen_hosts_and_devices_hosts();
}

void hosts_and_devices_hosts(void)
{
  io_update_devices_enabled(&deviceEnabled[0]);

  screen_hosts_and_devices_hosts();

  while (hd_subState == HD_HOSTS)
    hd_subState = input_hosts_and_devices_hosts();
}

void hosts_and_devices_long_filename(void)
{
  const char *f = io_get_device_filename(selected_device_slot);

  screen_hosts_and_devices_long_filename(f);
}

void hosts_and_devices_eject(unsigned char ds)
{
  io_umount_disk_image(ds);
  memset(deviceSlots[ds].file, 0, FILE_MAXLEN);
  deviceSlots[ds].hostSlot = 0xFF;
  io_put_device_slots(&deviceSlots[0]);
  io_get_device_slots(&deviceSlots[0]);
  screen_hosts_and_devices_eject(ds);
  hosts_and_devices_long_filename();
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

  io_update_devices_enabled(&deviceEnabled[0]);
  screen_hosts_and_devices_devices();
  hosts_and_devices_long_filename();

  while (hd_subState == HD_DEVICES)
    hd_subState = input_hosts_and_devices_devices();
}

void hosts_and_devices_devices_set_mode(unsigned char m)
{
  int i;
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
  memcpy(temp_filename, io_get_device_filename(selected_device_slot), sizeof(temp_filename));

  // unmount disk image
  io_umount_disk_image(selected_device_slot);

  // copy device slot back in.
  memcpy(&deviceSlots[selected_device_slot], &temp_deviceSlot, sizeof(DeviceSlot));
#if defined(BUILD_ATARI) || defined(BUILD_APPLE2) || defined(__CBM__)
  io_set_device_filename(selected_device_slot, selected_host_slot, m, temp_filename);
#else
  // i promise to implement in all cases.
  io_set_device_filename(selected_device_slot, temp_filename);
#endif

  io_put_device_slots(&deviceSlots[0]);

  // Make sure host slot is mounted or it will fail mounting disk
  io_mount_host_slot(deviceSlots[selected_device_slot].hostSlot);

  // Remount
#if defined(BUILD_ATARI)
  mnt = io_mount_disk_image(selected_device_slot, m);

  // Check for error
  if (!mnt)
  {
    // Display error for a moment then redraw menu after
    strcpy(err_msg, "ERROR SETTING DISK MODE: ");
    screen_error(err_msg);
    for (i = 0; i < 6000; i++)
      // Do nothing to let the message display

    // likely failed on setting write mode, make it read only
    temp_deviceSlot.mode = MODE_READ;
    memcpy(&deviceSlots[selected_device_slot], &temp_deviceSlot, sizeof(DeviceSlot));
    io_put_device_slots(&deviceSlots[0]);
    screen_error(""); // clear the error msg
  }
  screen_hosts_and_devices_device_slots(DEVICES_START_Y, &deviceSlots[0], &deviceEnabled[0]); // redraw the disks
#elif defined(BUILD_APPLE2)
  mnt = io_mount_disk_image(selected_device_slot, m);

  // Check for error
  if (!mnt)
  {
    // Display error for a moment then redraw menu after
    screen_error("Error setting disk mode");
    for (i = 0; i < 4000; i++)
      mnt = true; // Do nothing to let the message display
    // likely failed on setting write mode, make it read only
    temp_deviceSlot.mode = MODE_READ;
    memcpy(&deviceSlots[selected_device_slot], &temp_deviceSlot, sizeof(DeviceSlot));
    io_put_device_slots(&deviceSlots[0]);
  }
  screen_hosts_and_devices_device_slots(11, &deviceSlots[0], &deviceEnabled[0]); // redraw the disks
  //screen_hosts_and_devices_devices_selected(selected_device_slot); // Breaks disk order on screen??
  selected_device_slot = 0; // Go back to drive 0 instead
  hosts_and_devices_devices();
#else
    io_mount_disk_image(selected_device_slot, m);
#endif
}

void hosts_and_devices_devices_enable_toggle(unsigned char ds)
{
  bool s = io_get_device_enabled_status(io_device_slot_to_device(ds));

  if (s == true)
    io_disable_device(io_device_slot_to_device(ds));
  else
    io_enable_device(io_device_slot_to_device(ds));

  deviceEnabled[ds] = io_get_device_enabled_status(io_device_slot_to_device(ds));

  io_update_devices_enabled(&deviceEnabled[0]);
  screen_hosts_and_devices_device_slots(11, &deviceSlots[0], &deviceEnabled[0]);
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
      io_mount_host_slot(deviceSlots[i].hostSlot);
      io_mount_disk_image(i, deviceSlots[i].mode);
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
    io_get_host_slots(&hostSlots[0]);
    io_get_device_slots(&deviceSlots[0]);
    io_update_devices_enabled(&deviceEnabled[0]);
    screen_hosts_and_devices(&hostSlots[0], &deviceSlots[0], &deviceEnabled[0]);
#endif

  while (state == HOSTS_AND_DEVICES)
  {
#ifndef BUILD_PMD85
    io_get_host_slots(&hostSlots[0]);
    io_get_device_slots(&deviceSlots[0]);
    io_update_devices_enabled(&deviceEnabled[0]);
    screen_hosts_and_devices(&hostSlots[0], &deviceSlots[0], &deviceEnabled[0]);
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
