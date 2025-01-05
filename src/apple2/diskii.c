#ifdef BUILD_APPLE2
#include "diskii.h"
#ifndef __ORCAC__
#include <peekpoke.h>
#include <apple2.h>
#endif
#include <string.h>



/* FIXME - fujinet-bus-apple2.h isn't reachable */
extern uint8_t sp_get_fuji_id();
extern int8_t sp_control(uint8_t dest, uint8_t ctrlcode);
extern int8_t sp_status(uint8_t dest, uint8_t statcode);
extern uint8_t sp_payload[];
extern uint8_t sp_fuji_id;

#define MAX_DISKII      2

#define IWM_CTRL_CLEAR_ENSEEN   0x08
#define IWM_STATUS_ENSEEN       0x08

#define DISKII_SEL_DRIVE1       0x0a
#define DISKII_SEL_DRIVE2       0x0b
#define DISKII_MOTOR_ON         0x09
#define DISKII_MOTOR_OFF        0x08

DiskII_SlotDrive diskii_slotdrive[MAX_DISKII];

static uint8_t find_diskii(uint8_t start_slot)
{
  uint16_t offset;
  uint8_t idx;


  for (idx = start_slot; idx; idx--) {
    offset = 0xC000 + (idx << 8);
    if (PEEK(offset + 1) == 0x20 && PEEK(offset + 3) == 0 && PEEK(offset + 5) == 3
        && PEEK(offset + 255) == 0)
      return idx;
  }

  return idx;
}

static void enable_diskii(uint8_t slot, uint8_t drive)
{
  uint16_t offset;

  if (get_ostype() == APPLE_IIIEM) // Satan Mode
  {
    if (drive == 1)
    {
      POKE(0xC0D4,0); // Set Internal drive1 on
      POKE(0xC0D2,0);
      POKE(0xC0D0,0);
    }
    else
    {
      POKE(0xC0D5,0); // Set External drive1 on
      POKE(0xC0D2,0);
      POKE(0xC0D1,0);
    }
  }

  offset = 0xC080 + (slot << 4);
  POKE(offset + DISKII_SEL_DRIVE1 + ((drive - 1) & 1), 0);
  POKE(offset + DISKII_MOTOR_ON, 0);
  POKE(offset + DISKII_MOTOR_OFF, 0);

  if (get_ostype() == APPLE_IIIEM) // Satan Mode
  {
    POKE(0xC0D5,0); // all drives off
    POKE(0xC0D2,0);
    POKE(0xC0D0,0);
  }
  
  return;
}

void diskii_find()
{
  uint8_t dev_id, slot, drive, status, seen, mask;
  int8_t err;


  dev_id = sp_get_fuji_id();

  // FIXME - suddenly sp_fuji_get_id() is returning 1 instead of 8 and sp_fuji_id is 0
  if (!sp_fuji_id)
    dev_id = sp_get_fuji_id();
  // FIXME - even after sp_fuji_id is set it's still returning 1
  dev_id = sp_fuji_id;

  memset(diskii_slotdrive, 0xff, sizeof(diskii_slotdrive));

  if (dev_id) {
    seen = 0;
    for (slot = 7; slot; slot--) {
      slot = find_diskii(slot);
      if (!slot)
        break;

      for (drive = 1; drive <= 2; drive++) {
        err = sp_control(dev_id, IWM_CTRL_CLEAR_ENSEEN);
        if (err) {
          // If err is set then might be old firmware and shouldn't hide Disk II slot
          diskii_slotdrive[drive - 1].slot = 0;
          diskii_slotdrive[drive - 1].drive = drive;
          continue;
        }

        enable_diskii(slot, drive);
        err = sp_status(dev_id, IWM_STATUS_ENSEEN);
        if (!err) {
	        status = sp_payload[0] & (~seen);

          // Make sure only a single bit is set
          mask = status & (status - 1);
          if (mask != 0)
            status ^= mask;

          if (!status)
            continue;

          seen |= status;
          status--;
          if (status < MAX_DISKII) {
            diskii_slotdrive[status].slot = slot;
            diskii_slotdrive[status].drive = drive;
            diskii_slotdrive[status].fuji = status + 1;
          }
        }
      }
    }
  }

  return;
}

#endif /* BUILD_APPLE2 */
