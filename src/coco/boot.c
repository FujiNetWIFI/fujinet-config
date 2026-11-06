#include "../system.h"
#include "../pause.h"

void system_boot(void)
{
  static const char msg[] = "\rBOOTING USER ROM...\r";

  if (system_slot0_is_rom()) {
    // putstr, not printf: keeps the printf library out of apps that never print.
    putstr(msg, sizeof(msg) - 1);
    pause(120);
    // Tries a live, reset-free jump over One ROM's DriveWire bridge first;
    // falls back to fuji_enable_user_rom()'s plain hardware trigger (a real
    // reset) if no One ROM companion answers - see onerom_boot.c.
    system_enable_user_rom();
  }
  else
  {
    pause(120);
  }

  // On a Rev0000 FujiNet the ROM boot never happens, so cold start to boot normally.
  coldStart();
  exit(0);
}
