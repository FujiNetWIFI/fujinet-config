/**
 * FujiNet for #Adam configuration program
 *
 * Select a Destination Device Slot
 */

#ifdef _CMOC_VERSION_
#include <cmoc.h>
#include "coco/screen.h"
#include "coco/input.h"
#include "coco/globals.h"
#include "coco/io.h"
#include "coco/bar.h"
#define DIR_MAX_LEN 31
#else
#include <stdlib.h>
#include <string.h>
#endif /* _CMOC_VERSION_ */

#include "select_slot.h"

#ifdef BUILD_ADAM
#include <eos.h>
#include <conio.h>
#include "adam/screen.h"
#include "adam/input.h"
#include "adam/globals.h"
#include "adam/io.h"
#include "adam/bar.h"
#define DIR_MAX_LEN 31
#endif /* BUILD_ADAM */

#ifdef BUILD_APPLE2
#include "apple2/screen.h"
#include "apple2/input.h"
#include "apple2/globals.h"
#include "apple2/io.h"
#include "apple2/bar.h"
#define DIR_MAX_LEN 31
#endif /* BUILD_APPLE2 */

#ifdef BUILD_ATARI
#include "atari/screen.h"
#include "atari/input.h"
#include "atari/globals.h"
#include "atari/io.h"
#include "atari/bar.h"
#define DIR_MAX_LEN 36
#endif

#ifdef BUILD_C64
#include "c64/screen.h"
#include "c64/input.h"
#include "c64/globals.h"
#include "c64/io.h"
#include "c64/bar.h"
#define DIR_MAX_LEN 36
#endif /* BUILD_C64 */

#ifdef BUILD_PC8801
#include "pc8801/screen.h"
#include "pc8801/input.h"
#include "pc8801/globals.h"
#include "pc8801/io.h"
#include "pc8801/bar.h"
#endif /* BUILD_PC8801 */

#ifdef BUILD_PC6001
#include "pc6001/screen.h"
#include "pc6001/input.h"
#include "pc6001/globals.h"
#include "pc6001/io.h"
#include "pc6001/bar.h"
#endif /* BUILD_PC6001 */

#ifdef BUILD_PMD85
#include "pmd85/screen.h"
#include "pmd85/input.h"
#include "pmd85/globals.h"
#include "pmd85/io.h"
#include "pmd85/bar.h"
#define DIR_MAX_LEN 36
#endif /* BUILD_PMD85 */

#ifdef BUILD_RC2014
#include "rc2014/screen.h"
#include "rc2014/input.h"
#include "rc2014/globals.h"
#include "rc2014/io.h"
#include "rc2014/bar.h"
#define DIR_MAX_LEN 31
#endif /* BUILD_RC2014 */

extern DeviceSlot deviceSlots[8];
extern bool quick_boot;

char mode=0;

bool create=false;
SSSubState ss_subState;

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
      io_open_directory(selected_host_slot,path,filter);

      io_set_directory_position(pos);

      io_get_device_slots(&deviceSlots[0]);

#ifdef BUILD_PMD85
      screen_select_slot(io_read_directory(120,0x80)); // up to 3 lines of 40 chars each
#else
      screen_select_slot(io_read_directory(49,0x80));
#endif

      io_close_directory();
    }

  ss_subState=SS_CHOOSE;
}

void select_slot_eject(unsigned char ds)
{
  io_umount_disk_image(ds);
  memset(deviceSlots[ds].file,0,FILE_MAXLEN);
  deviceSlots[ds].hostSlot=0xFF;
  io_put_device_slots(&deviceSlots[0]);
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
    io_create_new(selected_host_slot,selected_device_slot,selected_size,path);
    memcpy(deviceSlots[selected_device_slot].file,path,DIR_MAX_LEN);
    deviceSlots[selected_device_slot].mode=2;
    deviceSlots[selected_device_slot].hostSlot=selected_host_slot;

#if defined(BUILD_ATARI) || defined(BUILD_APPLE2) || defined(__CBM__)
    // why hardcoded to 2 here?
    io_set_device_filename(selected_device_slot, selected_host_slot, 2, path);
#else
    io_put_device_slots(&deviceSlots[0]);
    io_set_device_filename(selected_device_slot, path);
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

    io_open_directory(selected_host_slot,path,filter);

    io_set_directory_position(pos);

    strcat(filename,io_read_directory(255-(unsigned char)strlen(path),0));

#if defined(BUILD_ATARI) || defined(BUILD_APPLE2) || defined(__CBM__)
    io_set_device_filename(selected_device_slot, selected_host_slot, mode, filename);
#else
    io_set_device_filename(selected_device_slot,filename);
#endif

    io_set_directory_position(pos);

    memcpy(deviceSlots[selected_device_slot].file,io_read_directory(DIR_MAX_LEN,0),DIR_MAX_LEN);
    deviceSlots[selected_device_slot].mode=mode;
    deviceSlots[selected_device_slot].hostSlot=selected_host_slot;

#ifndef BUILD_ATARI
    io_put_device_slots(&deviceSlots[0]);
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
    io_close_directory();
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
