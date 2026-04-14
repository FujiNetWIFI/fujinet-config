/**
 * @file mount_and_boot.c
 * @brief MS-DOS mount-all and boot implementation.
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <dos.h>
#include <direct.h>
#include <i86.h>
#include <fujinet-fuji.h>
#include "../system.h"
#include "screen.h"

void mount_and_boot(void)
{
  fuji_mount_all();
  exit(0);
}

void mount_and_boot_lobby(void)
{
  unsigned int total;
  unsigned int drive_num;
  char lobby_drive;
  union REGS regs;

  screen_end();
  puts("Loading Lobby...");

  lobby_drive = system_find_drive_letter_for_slot(0);

  fuji_set_boot_mode(2);

  regs.h.ah = 0x0D;
  int86(0x21, &regs, &regs);

  if (lobby_drive != '\0') {
    drive_num = lobby_drive - 'A' + 1;
    _dos_setdrive(drive_num, &total);
  }

  chdir("\\");
  spawnlp(P_OVERLAY, "COMMAND.COM", "COMMAND.COM", "/C", "AUTOEXEC.BAT", NULL);
}
