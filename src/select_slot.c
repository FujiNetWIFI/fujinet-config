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
#include "pause.h"
#ifdef BUILD_MSXROM
#include "msx/device_slots.h"
#endif /* BUILD_MSXROM */
#if defined(BUILD_COCO) || defined(BUILD_DRAGON)
#include "coco/strendswith.h"
#endif /* BUILD_COCO || BUILD_DRAGON */

char mode=0;

bool create=false;
SSSubState ss_subState;

#if defined(BUILD_COCO) || defined(BUILD_DRAGON)
// system_slot0_is_rom() (coco/system.c) only ever looks at device slot 0, so
// a .CCC/.ROM always goes there, skipping the normal slot picker -- there's
// no disk device for a cart ROM to occupy.
static bool selected_rom_image = false;

// Read the name of the currently selected directory entry and report
// whether it is a ROM cartridge image.
static bool selected_file_is_rom_image(void)
{
  fuji_open_directory_filter(selected_host_slot,path,filter);
  fuji_set_directory_position(pos);
  fuji_read_directory(255-(unsigned char)strlen(path), 0, response);
  fuji_close_directory();

  return strendswith(response, ".ccc") || strendswith(response, ".CCC") ||
         strendswith(response, ".rom") || strendswith(response, ".ROM");
}
#endif /* BUILD_COCO || BUILD_DRAGON */

#ifdef BUILD_ATARI
// Device slot 8 (index 7) is reserved for the cassette (C:) device.
#define CASSETTE_DEVICE_SLOT 7

// Read the name of the currently selected directory entry and report
// whether it is a cassette (.cas) image.
static bool selected_file_is_cassette(void)
{
  fuji_open_directory_filter(selected_host_slot,path,filter);
  fuji_set_directory_position(pos);
  fuji_read_directory(255-(unsigned char)strlen(path), 0, response);
  fuji_close_directory();

  return (strstr(response, ".cas")!=NULL) || (strstr(response, ".CAS")!=NULL);
}
#endif

void select_slot_init()
{
  if (quick_boot==true)
    {
      mode=0;
      selected_device_slot=0;
      ss_subState=SS_DONE;
    }
#ifdef BUILD_ATARI
  else if (create==false && selected_file_is_cassette())
    {
      // A .cas image must live in device slot 8 and be mounted read-only
      // so the firmware routes it to the C: cassette device. Force the
      // slot and mode here and skip the slot picker so that selecting a
      // cassette always behaves consistently.
      selected_device_slot=CASSETTE_DEVICE_SLOT;
      mode=MODE_READ;
      ss_subState=SS_DONE;
    }
#endif
#if defined(BUILD_COCO) || defined(BUILD_DRAGON)
  else if (create==false && (selected_rom_image = selected_file_is_rom_image()))
    {
      selected_device_slot=0;
      mode=MODE_READ;
      ss_subState=SS_DONE;
    }
#endif
  else
    {
#if defined(BUILD_COCO) || defined(BUILD_DRAGON)
      selected_rom_image = false;
#endif
      ss_subState=SS_DISPLAY;
    }
}

void select_slot_display()
{
  if (create==true)
    {
#if defined(BUILD_COCO) || defined(BUILD_H89)
      char dispPath[11 + DIR_MAX_LEN + 1];
#else
      char dispPath[42];
#endif
      memset(dispPath,0,sizeof(dispPath));
      strncpy(&dispPath[11],path,DIR_MAX_LEN);
#ifdef BUILD_MSXROM
      msx_set_mount_is_rom(false); // a newly created image is always a disk
#endif
      screen_select_slot(dispPath);
    }
  else
    {
      fuji_open_directory_filter(selected_host_slot,path,filter);

      fuji_set_directory_position(pos);

      if (slots_dirty)
        {
          fuji_get_device_slots(&deviceSlots[0], NUM_DEVICE_SLOTS);
          slots_dirty = false;
        }

#ifdef BUILD_MSXROM
      /* The slot picker needs to know whether a ROM is on its way in, so
         read the raw entry name before the formatted one below and rewind
         the directory for that second read. */
      fuji_read_directory(255-(unsigned char)strlen(path), 0, response);
      msx_set_mount_is_rom(msx_device_slot_is_rom(response));
      fuji_set_directory_position(pos);
#endif

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
  slots_dirty = true;
  fuji_unmount_disk_image(ds);
#ifdef BUILD_COCO
  if (ds == 0)
    config_disk_gone = true;
#endif /* BUILD_COCO */
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

  // An abort leaves SELECT_SLOT; the next pick must mount, not create
  if (state != SELECT_SLOT)
    create = false;
}

void select_slot_done()
{
  char filename[256];

#ifdef BUILD_APPLE2
  bool mnt = false;
  // int i;
#endif

  // Every path below can change the device slot table on the FujiNet - even a
  // refused NEW DISK leaves its name in the slot - and the firmware also owns
  // the MODE_MOUNTED bit, so our copy has to be re-read next time it is needed.
  slots_dirty = true;

  memset(filename,0,sizeof(filename));

  if (create==true)
  {
    create=false; // we're done with this until next time.
    screen_select_file_new_creating();
    if (!system_create_new(selected_host_slot,selected_device_slot,selected_size,path))
    {
      screen_error("  COULD NOT CREATE DISK.");
#ifdef BUILD_APPLE2
      pause(255); // the Apple II's pause() runs about 4x short
#else
      pause(150);
#endif
      goto not_created;
    }
#if defined(BUILD_COCO) || defined(BUILD_H89)
    memcpy(deviceSlots[selected_device_slot].file,path,FILE_MAXLEN);
#else
    memcpy(deviceSlots[selected_device_slot].file,path,DIR_MAX_LEN);
#endif
    deviceSlots[selected_device_slot].mode=2;
    deviceSlots[selected_device_slot].hostSlot=selected_host_slot;

#if !defined(BUILD_ATARI) && !defined(BUILD_APPLE2) && !defined(__CBM__)
    fuji_put_device_slots(&deviceSlots[0], NUM_DEVICE_SLOTS);
#endif
    // A new disk is for writing on, whichever key picked the slot
    fuji_set_device_filename(MODE_WRITE, selected_host_slot, selected_device_slot, path);

#ifdef BUILD_ADAM
    // An EOS directory holds the disk's size in 16 bits
    if (selected_size <= 0xFFFF)
    {
      screen_select_slot_build_eos_directory();
      if (input_select_slot_build_eos_directory())
      {
        fuji_mount_disk_image(selected_device_slot,2); // R/W
        screen_select_slot_build_eos_directory_label();
        input_select_slot_build_eos_directory_label(filename);
        screen_select_slot_build_eos_directory_creating();
        if (!system_build_directory(selected_device_slot,selected_size,filename))
        {
          screen_select_slot_build_eos_directory_failed();
          input_select_slot_build_eos_directory_failed();
        }
      }
    }

    state=HOSTS_AND_DEVICES;
    goto adam_bypass;
#endif
  }
  else
  {
    strcat(filename,path);

    fuji_open_directory_filter(selected_host_slot,path,filter);

    fuji_set_directory_position(pos);

    fuji_read_directory(255-(unsigned char)strlen(path), 0, response);
    strcat(filename, response);

#ifdef BUILD_MSDOS
    pause(6);
#endif

#ifdef BUILD_ADAM
    /* The AdamNet SET DEVICE FILENAME (0xE2) wire format carries no
       host slot or mode; the firmware reuses whatever the slot last
       held. Write the updated slot table first so the 0xE2 below is
       applied against the new host slot and mode. */
    deviceSlots[selected_device_slot].hostSlot=selected_host_slot;
    deviceSlots[selected_device_slot].mode=mode;
    memset(deviceSlots[selected_device_slot].file,0,FILE_MAXLEN);
    strncpy((char *)deviceSlots[selected_device_slot].file,response,FILE_MAXLEN-1);
    fuji_put_device_slots(&deviceSlots[0], NUM_DEVICE_SLOTS);
#endif

    fuji_set_device_filename(mode, selected_host_slot, selected_device_slot, filename);

#ifdef OBSOLETE
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
#endif // OBSOLETE

    fuji_close_directory();

#if defined(BUILD_COCO) || defined(BUILD_DRAGON)
    if (selected_rom_image && system_confirm_and_boot_user_rom(filename))
      return; // Firmware is resetting into the cart -- nothing left to do here.
#endif
  }

 not_created:
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
