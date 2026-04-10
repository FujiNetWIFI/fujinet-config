/**
 * @file mount_and_boot.c
 * @brief MS-DOS mount-all and boot implementation.
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 */

#include <stdlib.h>
#include <process.h>
#include <dos.h>
#include <direct.h>
#include <fujinet-fuji.h>
#include "screen.h"
#include "../system.h"

/**
 * @brief Mount all FujiNet device slots and exit the configurator.
 */
void mount_and_boot(void)
{
  fuji_mount_all();
  exit(0);
}

/**
 * @brief Set boot mode to lobby and reboot into it.
 */
void mount_and_boot_lobby(void)
{
  unsigned int total;
  char drive_letter;
  int drive_num;

  screen_end();
  fuji_set_boot_mode(2);

  drive_letter = system_find_drive_letter_for_slot(0);
  if (drive_letter == '\0')
    return;

  drive_num = drive_letter - 'A' + 1;
  _dos_setdrive((unsigned)drive_num, &total);
  chdir("\\");
  spawnlp(P_OVERLAY, "COMMAND.COM", "COMMAND.COM", "/C", "AUTOEXEC.BAT", NULL);
}
