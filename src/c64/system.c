#include "../system.h"
#include "../globals.h"
#include "../constants.h"
#include <cbm.h>
#include <conio.h>
#include <peekpoke.h>
#include <stdlib.h>

#define KEYBUF     0x0277 // KERNAL keyboard buffer
#define KEYBUF_LEN 0x00C6

char response[256];
unsigned char system_create_type;

/**
 * Return to BASIC, which then loads and runs the first mounted drive:
 * the commands are left on screen and RETURN in the keyboard buffer.
 */
void system_boot(void)
{
  unsigned char i;

  clrscr();
  for (i = 0; i < NUM_DEVICE_SLOTS; i++)
  {
    if (deviceSlots[i].hostSlot == 0xFF)
      continue;

    // BASIC prints READY. on row 1 and reads row 2; LOAD's messages
    // and the next READY. take rows 3-6. Drive slot 1 is device 8.
    gotoxy(0, 2);
    cprintf("load\"*\",%d,1", 8 + i);
    gotoxy(0, 7);
    cprintf("run");
    gotoxy(0, 0);
    POKE(KEYBUF, CH_ENTER);
    POKE(KEYBUF + 1, CH_ENTER);
    POKE(KEYBUF_LEN, 2);
    break;
  }

  exit(0);
}

bool system_create_new(uint8_t, uint8_t, uint32_t, const char *)
{
	// TODO implement
	return false;
}

