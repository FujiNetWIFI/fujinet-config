/**
 * @file system.c
 * @brief MS-DOS system-level utilities: boot, disk image creation, and drive-letter mapping.
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 */

#include "../system.h"

#include <stdlib.h>
#include <i86.h>
#include <string.h>
#include "screen.h"
#include <fujinet-fuji.h>

char response[256];
char deviceDriveLetters[8];

extern unsigned short custom_numSectors;
extern unsigned short custom_sectorSize;

/**
 * @brief Reset the video mode and exit the configurator, triggering a soft boot.
 */
void system_boot(void)
{
    static union REGS r;
    r.h.ah = 0x00;
    r.h.al = screen_mode;
    int86(0x10, (union REGS *)&r, (union REGS *)&r);
    exit(0);
}

/**
 * @brief Create a new floppy disk image on the FujiNet host.
 *
 * Maps the disk-type code in selected_size to the appropriate MS-DOS floppy
 * geometry (numSectors + sectorSize = 512) and sends a NEW_DISK command.
 *
 * @param selected_host_slot    Host slot index (0-based).
 * @param selected_device_slot  Device slot index (0-based).
 * @param selected_size         Disk type code: 360=360K, 720=720K,
 *                              1200=1.2MB, 1440=1.44MB, 999=custom.
 * @param path                  Full path/filename for the new image.
 */
void system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                       uint32_t selected_size, const char *path)
{
    static NewDisk newDisk;

    switch (selected_size)
    {
    case 360:
        newDisk.numSectors = 720;
        newDisk.sectorSize = 512;
        break;
    case 720:
        newDisk.numSectors = 1440;
        newDisk.sectorSize = 512;
        break;
    case 1200:
        newDisk.numSectors = 2400;
        newDisk.sectorSize = 512;
        break;
    case 1440:
        newDisk.numSectors = 2880;
        newDisk.sectorSize = 512;
        break;
    case 999:
        newDisk.numSectors = custom_numSectors;
        newDisk.sectorSize = custom_sectorSize;
        break;
    default:
        return;
    }

    newDisk.hostSlot   = selected_host_slot;
    newDisk.deviceSlot = selected_device_slot;
    strncpy(newDisk.filename, path, sizeof(newDisk.filename) - 1);
    newDisk.filename[sizeof(newDisk.filename) - 1] = '\0';

    fuji_create_new(&newDisk);
}

/**
 * @brief Refresh the cached drive-letter table for all 8 FujiNet device slots.
 */
void system_refresh_drive_letters(void)
{
    uint8_t slot;
    for (slot = 0; slot < 8; slot++)
        deviceDriveLetters[slot] = system_find_drive_letter_for_slot(slot);
}

/**
 * @brief Query a DOS drive number to determine whether it is a FujiNet
 *        virtual drive and, if so, which device slot it maps to.
 *
 * Issues an IOCTL "Receive Control Data" call (INT 21h AH=44h AL=04h) for
 * the given drive and inspects the returned fuji_ioctl_query block.
 *
 * @param drive  DOS drive number to query (1=A, 2=B, 3=C, ...).
 * @return       The FujiNet device slot index (0-based) on a match,
 *               or -1 if the drive is not a FujiNet drive.
 */
int find_drive_letter(int drive)
{
  static union REGS regs;
  static struct SREGS sregs;
  static fuji_ioctl_query query;

  memset((void *)&query, 0, sizeof(query));

  regs.h.ah = 0x44;       /* IOCTL Function */
  regs.h.al = 0x04;       /* Receive Control Data (Block) */
  regs.h.bl = (unsigned char)drive;
  regs.w.cx = sizeof(query);
  regs.x.dx = FP_OFF(&query);
  sregs.ds  = FP_SEG(&query);

  int86x(0x21, (union REGS *)&regs, (union REGS *)&regs, (struct SREGS *)&sregs);

  /* If Carry Flag is NOT set, the call succeeded */
  if (!(regs.x.cflag & INTR_CF)) {
    if (memcmp((void *)query.signature, FUJI_SIGNATURE, 4) == 0) {
      return query.unit; /* Match found! */
    }
  }

  return -1; /* Not found */
}


/**
 * @brief Find the DOS drive letter assigned to a given FujiNet device slot.
 *
 * Iterates drives C: through Z: (DOS drive numbers 3-26), calling
 * find_drive_letter() for each, and returns the first matching drive letter.
 *
 * @param device_slot  FujiNet device slot index (0-based, 0-7).
 * @return             Uppercase drive letter ('C'-'Z') if a match is found,
 *                     or '\0' if no mounted drive maps to that slot.
 */
char system_find_drive_letter_for_slot(uint8_t device_slot)
{
    int drive;
    char drive_letter = '\0';

    for (drive = 3; drive <= 26; drive++)
    {
        if (find_drive_letter(drive) == (int) device_slot)
        {
            drive_letter = 'A' + drive - 1;
            break;
        }
    }

    return drive_letter;
}
