/**
 * FujiNet for #Adam configuration program
 *
 * Hosts and Devices
 */

#include <string.h>
#include "hosts_and_devices.h"
#include "die.h"
#include "typedefs.h"

#ifdef BUILD_ADAM
#include "adam/globals.h"
#include "adam/fuji_typedefs.h"
#include "adam/io.h"
#include "adam/screen.h"
#include "adam/input.h"
#include "adam/bar.h"
#endif /* BUILD_ADAM */

#ifdef BUILD_APPLE2
#include "apple2/globals.h"
#include "apple2/fuji_typedefs.h"
#include "apple2/io.h"
#include "apple2/screen.h"
#include "apple2/input.h"
#include "apple2/bar.h"
// #include <stdio.h> // for debugging using sprintf
#endif /* BUILD_APPLE2 */

#ifdef BUILD_ATARI
#include "atari/globals.h"
#include "atari/fuji_typedefs.h"
#include "atari/io.h"
#include "atari/screen.h"
#include "atari/input.h"
#include "atari/bar.h"
#endif /* BUILD_ATARI */

#ifdef BUILD_C64
#include "c64/globals.h"
#include "c64/fuji_typedefs.h"
#include "c64/io.h"
#include "c64/screen.h"
#include "c64/input.h"
#include "c64/bar.h"
#endif /* BUILD_C64 */

#ifdef BUILD_PC8801
#include "pc8801/globals.h"
#include "pc8801/fuji_typedefs.h"
#include "pc8801/io.h"
#include "pc8801/screen.h"
#include "pc8801/input.h"
#include "pc8801/bar.h"
#endif /* BUILD_PC8801 */

#ifdef BUILD_PC6001
#include "pc6001/globals.h"
#include "pc6001/fuji_typedefs.h"
#include "pc6001/io.h"
#include "pc6001/screen.h"
#include "pc6001/input.h"
#include "pc6001/bar.h"
#endif /* BUILD_PC6001 */

HDSubState hd_subState;
DeviceSlot deviceSlots[8];
DeviceSlot temp_deviceSlot;
bool deviceEnabled[8];
HostSlot hostSlots[8];
char selected_host_slot;
char selected_device_slot;
char selected_host_name[32];
char temp_filename[256];

extern bool quick_boot;

void hosts_and_devices_edit_host_slot(unsigned char i)
{
  unsigned char o;

  if (strlen((const char *)hostSlots[i]) == 0)
  {
    screen_hosts_and_devices_clear_host_slot(i);
    o = 0;
  }
  else
    o = strlen((const char *)hostSlots[i]);

  screen_hosts_and_devices_edit_host_slot(i);
  input_line_hosts_and_devices_host_slot(i, o, (char *)hostSlots[i]);

  if (strlen((const char*)hostSlots[i]) == 0)
    screen_hosts_and_devices_host_slot_empty(i);

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
  char *f = io_get_device_filename(selected_device_slot);

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
  memset(temp_filename, 0, sizeof(temp_filename));

  // Stow device slot temporarily
  memcpy(&temp_deviceSlot, &deviceSlots[selected_device_slot], sizeof(DeviceSlot));
  temp_deviceSlot.mode = m;
  memcpy(temp_filename, io_get_device_filename(selected_device_slot), sizeof(temp_filename));

  // unmount disk image
  io_umount_disk_image(selected_device_slot);

  // copy device slot back in.
  memcpy(&deviceSlots[selected_device_slot], &temp_deviceSlot, sizeof(DeviceSlot));
  io_set_device_filename(selected_device_slot, temp_filename);

  io_put_device_slots(&deviceSlots[0]);

  // Remount
  io_mount_disk_image(selected_device_slot, m);
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

  for (i = 0; i < NUM_DEVICE_SLOTS; i++) // 4 for apple for now, what about adam? 8 for atari?
  {
    if (deviceSlots[i].hostSlot != 0xFF)
    {
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

  io_get_host_slots(&hostSlots[0]);
  io_get_device_slots(&deviceSlots[0]);
  io_update_devices_enabled(&deviceEnabled[0]);
  screen_hosts_and_devices(&hostSlots[0], &deviceSlots[0], &deviceEnabled[0]);

  while (state == HOSTS_AND_DEVICES)
  {
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
  if (state == DONE)
    hosts_and_devices_done();
}
