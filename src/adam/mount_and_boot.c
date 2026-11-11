#include "../mount_and_boot.h"
#include "../screen.h"
#include "../system.h"
#include "../globals.h"
#include <smartkeys.h>

void mount_and_boot_lobby(void)
{
  if (!screen_mount_and_boot_lobby())
    return;

  smartkeys_sound_play(SOUND_CONFIRM);
  smartkeys_display(NULL,NULL,NULL,NULL,NULL,NULL);
  smartkeys_status("  BOOTING LOBBY...");
  fuji_set_boot_mode(FUJI_BOOT_MODE_LOBBY);
  system_boot();
}
