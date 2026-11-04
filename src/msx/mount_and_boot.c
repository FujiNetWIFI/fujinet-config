#include "../mount_and_boot.h"
#include "../system.h"
#include "../globals.h"
#include "../constants.h"

void boot_to_basic(void)
{
  // TODO: unify this with fuji_enable_user_rom()

  // we need to flip the switch to the other rom while not running from rom.
  // so we put this piece of code in ram at $C000 then we jump to it
  // to switch and reboot into new rom.
  // TODO: parameterize the IO_CONTROL port address
  // C000                          .ORG   C000H
  // C000   21 FF BF               LD   hl,$bfff
  // C003   36 04                  LD   (hl),$04
  // C005   C3 00 00               JP   0

  unsigned char *code = (unsigned char *)0xC000;
  code[0] = 0x21; code[1] = 0xFF; code[2] = 0xBF;  // LD hl,$bfff
  code[3] = 0x36; code[4] = 0x04;                  // LD (hl),$04
  code[5] = 0xC3; code[6] = 0x00; code[7] = 0x00;  // JP 0

  __asm
    jp 0xC000
  __endasm;
  return;
}

void mount_and_boot_lobby(void)
{
  screen_mount_and_boot();
  fuji_set_boot_mode(FUJI_BOOT_MODE_LOBBY);
  system_boot();
}

void mount_and_boot(void)
{
  screen_mount_and_boot();

  if (!fuji_get_device_slots(deviceSlots, FUJI_MAX_DISKS))
  {
    screen_error("ERROR READING DEVICE SLOTS");
    die();
  }

  if (!fuji_get_host_slots(hostSlots, FUJI_MAX_HOSTS))
  {
    screen_error("ERROR READING HOST SLOTS");
    die();
  }

  if (!fuji_mount_all())
  {
    screen_error("ERROR MOUNTING ALL");
    state = HOSTS_AND_DEVICES;
  }
  else
  {
    fuji_set_boot_config(0);
    system_boot();
  }
}
