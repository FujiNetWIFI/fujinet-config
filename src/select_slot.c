/**
 * Select a Destination Device Slot
 */

#include "select_slot.h"
#include "globals.h"
#include "constants.h"
#include "compat.h"
#include "screen.h"
#include "input.h"
#include "system.h"

char mode=0;

bool create=false;
SSSubState ss_subState;
char response[256];

void select_slot_init()
{
  if (quick_boot==true)
    {
      mode=0;
      selected_device_slot=0;
      ss_subState=SS_DONE;
    }
  else
    ss_subState=SS_DISPLAY;
}

void select_slot_display()
{
  if (create==true)
    {
      char dispPath[42];
      memset(dispPath,0,42);
      strncpy(&dispPath[11],path,DIR_MAX_LEN);
      screen_select_slot(dispPath);
    }
  else
    {
      fuji_open_directory2(selected_host_slot,path,filter);

      fuji_set_directory_position(pos);

      fuji_get_device_slots(&deviceSlots[0], NUM_DEVICE_SLOTS);

#ifdef BUILD_PMD85
      fuji_read_directory(120, 0x80, response); // up to 3 lines of 40 chars each
#else
      fuji_read_directory(49, 0x80, response);
#endif
      screen_select_slot(response);

      fuji_close_directory();
    }

  ss_subState=SS_CHOOSE;
}

void select_slot_eject(unsigned char ds)
{
  fuji_unmount_disk_image(ds);
  memset(deviceSlots[ds].file,0,FILE_MAXLEN);
  deviceSlots[ds].hostSlot=0xFF;
  fuji_put_device_slots(&deviceSlots[0], NUM_DEVICE_SLOTS);
  screen_select_slot_eject(ds);
}

void select_slot_choose()
{
  screen_select_slot_choose();

  while (ss_subState==SS_CHOOSE)
    ss_subState=input_select_slot_choose();
}

void select_slot_done()
{
  char filename[256];

#ifdef BUILD_APPLE2
  bool mnt = false;
  // int i;
#endif

  memset(filename,0,sizeof(filename));

  if (create==true)
  {
    create=false; // we're done with this until next time.
    screen_select_file_new_creating();
    system_create_new(selected_host_slot,selected_device_slot,selected_size,path);
    memcpy(deviceSlots[selected_device_slot].file,path,DIR_MAX_LEN);
    deviceSlots[selected_device_slot].mode=2;
    deviceSlots[selected_device_slot].hostSlot=selected_host_slot;

#if defined(BUILD_ATARI) || defined(BUILD_APPLE2) || defined(__CBM__)
    // why hardcoded to 2 here?
    fuji_set_device_filename(2, selected_host_slot, selected_device_slot, path);
#else
    fuji_put_device_slots(&deviceSlots[0], NUM_DEVICE_SLOTS);
    fuji_set_device_filename(mode, selected_host_slot, selected_device_slot, path);
#endif

#ifdef BUILD_ADAM
    screen_select_slot_build_eos_directory();
    if (input_select_slot_build_eos_directory())
    {
      io_mount_disk_image(selected_device_slot,2); // R/W
      screen_select_slot_build_eos_directory_label();
      input_select_slot_build_eos_directory_label(filename);
      screen_select_slot_build_eos_directory_creating();
      io_build_directory(selected_device_slot,selected_size,filename);
    }

    state=HOSTS_AND_DEVICES;
    goto adam_bypass;
#endif
  }
  else
  {
    strcat(filename,path);

    fuji_open_directory2(selected_host_slot,path,filter);

    fuji_set_directory_position(pos);

    fuji_read_directory(255-(unsigned char)strlen(path), 0, response);
    strcat(filename, response);

    fuji_set_device_filename(mode, selected_host_slot, selected_device_slot, filename);

    fuji_set_directory_position(pos);

    fuji_read_directory(DIR_MAX_LEN, 0, response);
    memcpy(deviceSlots[selected_device_slot].file, response, DIR_MAX_LEN);
    deviceSlots[selected_device_slot].mode=mode;
    deviceSlots[selected_device_slot].hostSlot=selected_host_slot;

#ifndef BUILD_ATARI
    fuji_put_device_slots(&deviceSlots[0], NUM_DEVICE_SLOTS);
#endif

#ifdef BUILD_APPLE2
    // Try to mount the disk and error on failure
    /* Disabled for now because it ends up mounting now and during
       mount and boot phase which could mean extra looong time to
       wait. Maybe add a new fuji command to check if disk is
       already mounted for use during mount and boot?
    mnt = io_mount_disk_image(selected_device_slot, mode);

    if (mnt)
    {
      // Display error for a moment
      screen_error("ERROR MOUNTING DISK");
      for (i = 0; i < 4000; i++)
        mnt = true; // Do nothing to let the message display
      io_umount_disk_image(selected_device_slot);
      memset(deviceSlots[selected_device_slot].file,0,FILE_MAXLEN);
      deviceSlots[selected_device_slot].hostSlot=0xFF;
      io_put_device_slots(&deviceSlots[0]);
    }*/
#endif
    fuji_close_directory();
  }

  if (!quick_boot)
    {
#ifdef BUILD_PMD85
      // go back to hosts and devices
      state=HOSTS_AND_DEVICES;
#else
      state=SELECT_FILE;
      backToFiles = true;
#endif
    }
  else
    {
      state=HOSTS_AND_DEVICES;
    }

#ifdef BUILD_ADAM
 adam_bypass:
#endif
  return;
}

void select_slot(void)
{
  ss_subState=SS_INIT;

  while (state==SELECT_SLOT)
  {
    switch(ss_subState)
    {
    case SS_INIT:
      select_slot_init();
      break;
    case SS_DISPLAY:
      select_slot_display();
      break;
    case SS_CHOOSE:
      select_slot_choose();
      break;
    case SS_DONE:
      select_slot_done();
      break;
    case SS_ABORT:
      break;
    }
  }
}
