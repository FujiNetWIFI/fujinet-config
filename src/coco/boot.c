#include "../system.h"
#include "../pause.h"
#include "../globals.h"

// Literals, not static const arrays: CMOC also keeps an unused copy of an array's initializer
#define BOOT_MSG "\rBOOTING USER ROM...\r"
#define FAILED_MSG "\rCOULD NOT LOAD THE ROM\r"

void system_boot(void)
{
  bool rom = system_slot0_is_rom();

  if (rom) {
    // putstr, not printf: keeps the printf library out of apps that never print.
    if (deviceSlots[0].hostSlot == 0xFF) {
      putstr(FAILED_MSG, sizeof(FAILED_MSG) - 1);
      rom = false;
    }
    else
      putstr(BOOT_MSG, sizeof(BOOT_MSG) - 1);
  }
  fuji_sleep_ms(2000);
  if (rom)
    fuji_enable_user_rom();

  // On a Rev0000 FujiNet the ROM boot never happens, so cold start to boot normally.
  coldStart();
  exit(0);
}
