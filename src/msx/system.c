#include <conio.h>
#include "../system.h"

#define RG9SAV 0xFFE8

// default value
uint8_t system_fps = 60;

char response[256];

void system_boot(void)
{
  // we need to flip the switch to the other rom while not running from rom.
  // so we put this piece of code in ram at $C000 then we jump to it
  // to switch and reboot into new rom.
  // TODO: parameterize the IO_CONTROL port address
  // C000                          .ORG   C000H   
  // C000   21 FF BF               LD   hl,$bfff   
  // C003   36 81                  LD   (hl),$81   
  // C005   C3 00 00               JP   0   

  unsigned char *code = (unsigned char *)0xC000;
  code[0] = 0x21; code[1] = 0xFF; code[2] = 0xBF;  // LD hl,$bfff
  code[3] = 0x36; code[4] = 0x81;                  // LD (hl),$81
  code[5] = 0xC3; code[6] = 0x00; code[7] = 0x00;  // JP 0

  __asm
    jp 0xC000
  __endasm;
  return;
}

void system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                       uint32_t selected_size, const char *path)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    // last_rc = fujinet_create_new(selected_host_slot, selected_device_slot, selected_size, path);
#endif
}

void system_set_fps()
{
  unsigned char *rg9sav_addr = RG9SAV;
  system_fps = *rg9sav_addr & 0b10 ? 50 : 60;
}
