#include "../system.h"
#include "sp.h"
#include "apple2_globals.h"
#include <conio.h>
#include <apple2.h>
#include <peekpoke.h> // For the insanity in io_boot()
#include <string.h>

static NewDisk newDisk;
uint8_t system_create_type;
char response[256];

void system_boot(void)
{
#ifdef __ORCAC__
  sp_done();
#ifndef BUILD_A2CDA
  WriteChar(0x8c);  // Clear screen
  WriteChar(0x92);  // Set 80 col
  WriteChar(0x86);  // Cursor on
  TextShutDown();
  exit(0);
#endif /* BUILD_A2CDA */

#else /* ! __ORCAC__ */
  char ostype;
  int i;

  ostype = get_ostype() & 0xF0;
  //clrscr();
  cprintf("\r\nRESTARTING...");

  // Wait for fujinet disk ii states to be ready
  for (i = 0; i < 2000; i++)
    {
      if (i % 250 == 0)
        {
          cprintf(".");
        }
    }

  if (ostype == APPLE_IIIEM)
    {
      asm("STA $C082");  // disable language card (Titan3+2)
      asm("LDA #$77");   // enable A3 primary rom
      asm("STA $FFDF");
      asm("JMP $F4EE");  // jmp to A3 reset entry
    }
  else  // Massive brute force hack that takes advantage of MMU quirk. Thank you xot.
    {
      POKE(0xC00E,0); // CLRALTCHAR

      // Make the simulated 6502 RESET result in a cold start.
      // INC $03F4
      POKE(0x100,0xEE);
      POKE(0x101,0xF4);
      POKE(0x102,0x03);

      // Make sure to not get disturbed.
      // SEI
      POKE(0x103,0x78);

      // Disable Language Card (which is enabled for all cc65 programs).
      // LDA $C082
      POKE(0x104,0xAD);
      POKE(0x105,0x82);
      POKE(0x106,0xC0);

      // Simulate a 6502 RESET, additionally do it from the stack page to make the MMU
      // see the 6502 memory access pattern which is characteristic for a 6502 RESET.
      // JMP ($FFFC)
      POKE(0x107,0x6C);
      POKE(0x108,0xFC);
      POKE(0x109,0xFF);

      asm("JMP $0100");
    }
#endif /* __ORCAC__ */
}

void system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                       unsigned long selected_size, const char *path)
{
  newDisk.createType = system_create_type;
  newDisk.deviceSlot = selected_device_slot;
  newDisk.hostSlot = selected_host_slot;
  newDisk.numBlocks = selected_size;
  strcpy(&newDisk.filename[0], path);
  fuji_create_new(&newDisk);
  return;
}

void system_list_devs(void)
{
    sp_list_devs();
    state = HOSTS_AND_DEVICES;
}

