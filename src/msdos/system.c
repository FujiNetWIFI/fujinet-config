/**
 * @file system.c
 * @brief MS-DOS system-level utilities: boot, disk image creation, and drive-letter mapping.
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 */

#include "../system.h"

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <i86.h>
#include <stdbool.h>
#include <string.h>
#include "screen.h"
#include <fujinet-fuji.h>

char response[256];
char deviceDriveLetters[8];
bool install_tsr = false;

extern unsigned short custom_numSectors;
extern unsigned short custom_sectorSize;

/* AppKey identity for persisting the "install TSR" setting.
   Creator/App IDs are the registered FujiNet Config values. */
#define AK_CREATOR_ID 0x0001
#define AK_APP_ID     0x02
#define AK_KEY_TSR    0

/**
 * @brief Load the persisted "install TSR" setting from its appkey.
 *        A failed read (e.g. no SD card in the FujiNet) leaves
 *        install_tsr at its default of false.
 *
 *        buf/count are static so their addresses are near pointers;
 *        this build uses -zu (SS != DS) and auto locals would yield
 *        far pointers that the fujinet-lib calls truncate.
 */
void system_load_tsr_setting(void)
{
    static uint8_t  buf[MAX_APPKEY_LEN + 2];
    static uint16_t count;

    count = 0;
    fuji_set_appkey_details(AK_CREATOR_ID, AK_APP_ID, DEFAULT);
    if (fuji_read_appkey(AK_KEY_TSR, &count, buf) && count >= 1)
        install_tsr = (buf[0] != 0);
}

/**
 * @brief Persist the current "install TSR" setting to its appkey.
 *        Writes the single bool byte directly — a 0-byte write is
 *        silently ignored over RS-232, so we always send one byte.
 */
void system_save_tsr_setting(void)
{
    fuji_set_appkey_details(AK_CREATOR_ID, AK_APP_ID, DEFAULT);
    fuji_write_appkey(AK_KEY_TSR, 1, (uint8_t *)&install_tsr);
}

/**
 * @brief If the user armed the TSR option, install CFGTSR.EXE now.
 *        Must run BEFORE fuji_mount_all() swaps the config disk out from
 *        under us. Uses P_WAIT so control returns here after CFGTSR has
 *        called INT 21h AH=31h to stay resident.
 *
 *        CFGTSR.EXE lives on the config disk image, which is mapped to
 *        device slot 0. config.exe may have been launched from a different
 *        drive, so prepend slot 0's drive letter to the path.
 */
void install_tsr_now(void)
{
    static char tsr_path[24];

    if (!install_tsr)
        return;

    screen_end();

    sprintf(tsr_path, "%c:\\CFGTSR.EXE", deviceDriveLetters[0]);

    spawnlp(P_WAIT, tsr_path, tsr_path, "/I", NULL);
    install_tsr = false;
}

/**
 * @brief Reset the video mode and exit the configurator, triggering a soft boot.
 */
void system_boot(void)
{
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
 * @brief Build the cached drive-letter table for all 8 FujiNet device slots.
 *
 *        One pass over DOS drives C:-Z: fills every entry, because
 *        find_drive_letter() hands back the device slot the drive belongs to.
 *        The previous shape asked that question once per slot and so paid for
 *        the whole C:-Z: walk eight times over - 192 IOCTL calls where 24 do.
 *
 *        The result is cached for the life of the program. FUJINET.SYS declares
 *        num_units at CONFIG.SYS time and DOS hands out its drive letters right
 *        then; mounting, ejecting or changing a slot's mode never moves a unit
 *        to a different letter, so there is nothing to invalidate.
 */
void system_refresh_drive_letters(void)
{
    static bool drive_letters_valid = false;
    int drive;
    int unit;

    if (drive_letters_valid)
        return;

    memset((void *)deviceDriveLetters, 0, sizeof(deviceDriveLetters));

    for (drive = 3; drive <= 26; drive++)
    {
        unit = find_drive_letter(drive);
        if (unit >= 0 && unit < 8)
            deviceDriveLetters[unit] = (char)('A' + drive - 1);
    }

    drive_letters_valid = true;
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

