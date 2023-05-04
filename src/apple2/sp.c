#ifdef BUILD_APPLE2
/**
 * FujiNet CONFIG for #Apple2
 *
 * SmartPort MLI Routines
 */

#ifdef __INTELLISENSE__
// 18, expect closing parenthses - needed to use cc65 inline asm command with agruments.
  #pragma diag_suppress 18
#endif


#include "sp.h"
#include "screen.h"
#include <conio.h>
#include <apple2.h>
#include <peekpoke.h>

#ifdef __ORCAC__
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <misctool.h>
#endif

#define SP_CMD_STATUS 0
#define SP_CMD_CONTROL 4
#define SP_STATUS_PARAM_COUNT 3
#define SP_CONTROL_PARAM_COUNT 3

// extern globals:
#ifdef __ORCAC__
uint8_t *sp_payload;
#else
uint8_t sp_payload[1024];
#endif
uint16_t sp_count;
uint8_t sp_dest;
uint16_t sp_dispatch;
uint8_t sp_error;

#ifdef __ORCAC__
static int myId;
static Handle sp_handle;
static uint8_t *sp_cmdlist;
static uint8_t *sp_instr;
static FWRec fwRec;
#else
static uint8_t sp_cmdlist[10];
static uint8_t sp_err, sp_rtn_low, sp_rtn_high;
#endif
static uint8_t sp_cmdlist_low, sp_cmdlist_high;
static uint8_t sp_dispatch_low, sp_dispatch_high;

int8_t sp_status(uint8_t dest, uint8_t statcode)
{
  sp_error = 0;
  // build the command list
  sp_cmdlist[0] = SP_STATUS_PARAM_COUNT;
  sp_cmdlist[1] = dest; // set before calling sp_status();
#ifdef __ORCAC__
  sp_cmdlist[2] = (uint8_t)((uint16_t)sp_payload & 0x00FF);
  sp_cmdlist[3] = (uint8_t)((uint16_t)sp_payload >> 8) & 0xFF;
#else
  sp_cmdlist[2] = (uint8_t)((uint16_t)&sp_payload & 0x00FF);
  sp_cmdlist[3] = (uint8_t)((uint16_t)&sp_payload >> 8) & 0xFF;
#endif
  sp_cmdlist[4] = statcode;

#ifdef __ORCAC__
  sp_cmdlist_low = (uint8_t)((uint16_t)sp_cmdlist & 0x00FF);
  sp_cmdlist_high = (uint8_t)((uint16_t)sp_cmdlist >> 8) & 0xFF;
#else
  sp_cmdlist_low = (uint8_t)((uint16_t)&sp_cmdlist & 0x00FF);
  sp_cmdlist_high = (uint8_t)((uint16_t)&sp_cmdlist >> 8) & 0xFF;
#endif
  sp_dispatch_low = (uint8_t)((uint16_t)sp_dispatch & 0x00FF);
  sp_dispatch_high = (uint8_t)((uint16_t)sp_dispatch >> 8) & 0xFF;

#ifdef __ORCAC__
  // Make firmware call
  sp_instr[0] = 0x20;	// JSR
  sp_instr[1] = sp_dispatch_low;
  sp_instr[2] = sp_dispatch_high;
  sp_instr[3] = SP_CMD_STATUS;
  sp_instr[4] = sp_cmdlist_low;
  sp_instr[5] = sp_cmdlist_high;
  sp_instr[6] = 0x60;	// RTS

  // Call in emulation mode with FWEntry
  fwRec = FWEntry(0, 0, 0, (Word)sp_instr);
  sp_count = (fwRec.yRegExit << 8) | fwRec.xRegExit;
  sp_error = fwRec.aRegExit;
  return sp_error;
#else
  // store cmd list
  __asm__ volatile ("lda #%b", SP_CMD_STATUS);
  __asm__ volatile ("sta %g", spCmd); // store status command #
  __asm__ volatile ("lda %v", sp_cmdlist_low);
  __asm__ volatile ("sta %g", spCmdListLow); // store status command #
  __asm__ volatile ("lda %v", sp_cmdlist_high);
  __asm__ volatile ("sta %g", spCmdListHigh); // store status command #
  __asm__ volatile ("lda #%b", 0x20); // jsr opcode
  __asm__ volatile ("sta %g", spJmp);
  __asm__ volatile ("lda %v", sp_dispatch_low); // sp entry lsb
  __asm__ volatile ("sta %g", spDispatchLow);
  __asm__ volatile ("lda %v", sp_dispatch_high); // sp entry msb
  __asm__ volatile ("sta %g", spDispatchHigh);
spJmp:
  __asm__ volatile ("nop");
spDispatchLow:
  __asm__ volatile ("nop");
spDispatchHigh:
  __asm__ volatile ("nop");
spCmd:
  __asm__ volatile ("nop");
spCmdListLow:
  __asm__ volatile ("nop");
spCmdListHigh:
  __asm__ volatile ("nop");
  __asm__ volatile ("stx %v", sp_rtn_low);
  __asm__ volatile ("sty %v", sp_rtn_high);
  __asm__ volatile ("sta %v", sp_err);

  sp_count = ((uint16_t)sp_rtn_high << 8) | (uint16_t)sp_rtn_low;
  sp_error = sp_err;
  return sp_err;
#endif
}

int8_t sp_control(uint8_t dest, uint8_t ctrlcode)
{
  sp_error = 0;
  // sp_dest = 5; // need to search
  // build the command list
  sp_cmdlist[0] = SP_CONTROL_PARAM_COUNT;
  sp_cmdlist[1] = dest; // set before calling sp_status();
#ifdef __ORCAC__
  sp_cmdlist[2] = (uint8_t)((uint16_t)sp_payload & 0x00FF);
  sp_cmdlist[3] = (uint8_t)((uint16_t)sp_payload >> 8) & 0xFF;
#else
  sp_cmdlist[2] = (uint8_t)((uint16_t)&sp_payload & 0x00FF);
  sp_cmdlist[3] = (uint8_t)((uint16_t)&sp_payload >> 8) & 0xFF;
#endif
  sp_cmdlist[4] = ctrlcode;

#ifdef __ORCAC__
  sp_cmdlist_low = (uint8_t)((uint16_t)sp_cmdlist & 0x00FF);
  sp_cmdlist_high = (uint8_t)((uint16_t)sp_cmdlist >> 8) & 0xFF;
#else
  sp_cmdlist_low = (uint8_t)((uint16_t)&sp_cmdlist & 0x00FF);
  sp_cmdlist_high = (uint8_t)((uint16_t)&sp_cmdlist >> 8) & 0xFF;
#endif
  sp_dispatch_low = (uint8_t)((uint16_t)sp_dispatch & 0x00FF);
  sp_dispatch_high = (uint8_t)((uint16_t)sp_dispatch >> 8) & 0xFF;

#ifdef __ORCAC__
  // Make firmware call
  sp_instr[0] = 0x20;	// JSR
  sp_instr[1] = sp_dispatch_low;
  sp_instr[2] = sp_dispatch_high;
  sp_instr[3] = SP_CMD_CONTROL;
  sp_instr[4] = sp_cmdlist_low;
  sp_instr[5] = sp_cmdlist_high;
  sp_instr[6] = 0x60;	// RTS

  // Call in emulation mode with FWEntry
  fwRec = FWEntry(0, 0, 0, (Word)sp_instr);
  sp_count = (fwRec.yRegExit << 8) | fwRec.xRegExit;
  sp_error = fwRec.aRegExit; 
  return sp_error;                         
#else
  // store cmd list
  __asm__ volatile ("lda #%b", SP_CMD_CONTROL);
  __asm__ volatile ("sta %g", spCmd); // store status command #
  __asm__ volatile ("lda %v", sp_cmdlist_low);
  __asm__ volatile ("sta %g", spCmdListLow); // store status command #
  __asm__ volatile ("lda %v", sp_cmdlist_high);
  __asm__ volatile ("sta %g", spCmdListHigh); // store status command #
  __asm__ volatile ("lda #%b", 0x20); // jsr opcode
  __asm__ volatile ("sta %g", spJmp);
  __asm__ volatile ("lda %v", sp_dispatch_low); // sp entry lsb
  __asm__ volatile ("sta %g", spDispatchLow);
  __asm__ volatile ("lda %v", sp_dispatch_high); // sp entry msb
  __asm__ volatile ("sta %g", spDispatchHigh);
spJmp:
  __asm__ volatile ("nop");
spDispatchLow:
  __asm__ volatile ("nop");
spDispatchHigh:
  __asm__ volatile ("nop");
spCmd:
  __asm__ volatile ("nop");
spCmdListLow:
  __asm__ volatile ("nop");
spCmdListHigh:
  __asm__ volatile ("nop");
  __asm__ volatile ("sta %v", sp_err);
  sp_error = sp_err;
  return sp_err;
#endif
}

int8_t sp_find_fuji(void)
{
  // const char fuji[9] = "THE_FUJI";
  const char fuji[14] = "FUJINET_DISK_0";
  const uint8_t fuji_len = sizeof(fuji);
  int8_t err, num, i, j;

  err = sp_status(0x00, 0x00); // get number of devices
	if (err)
    return -err;
  num = sp_payload[0];
	num++;
	for (i = 1; i < num; i++)
	{
    //do
      err = sp_status(i, 0x03); // get DIB
    //while (err);
    if (sp_payload[4] == fuji_len)
    {
      for (j = 0; j < fuji_len; j++)
        if (fuji[j]!=sp_payload[5+j])
          return 0;
      sp_dest = i; // store the fuji unit #
      return i;
    }
	}
  return 0;
}

/* Theoretical maximum # of SmartPort devices is 127
 * Call out to each device and if they respond (err = 0),
 * then display it's name
 */
void sp_list_devs(void)
{
  int8_t err, num = 127, i, j;

  revers(1);
  cprintf(" SMARTPORT DEVICE LIST \r\n\r\n");
  revers(0);

  for (i = 1; i < num; i++)
  {
    err = sp_status(i, 0x03);
    if (err == 0){
      cprintf("UNIT #%d NAME: ", i);
      for (j = 0; j < sp_payload[4]; j++)
        cputc(sp_payload[5 + j]);
      cputs("\r\n");
    }
  }
  revers(1);
  cprintf("\r\n PRESS ANY KEY TO CONTINUE \r\n");
  revers(0);
  cgetc();
}

/**
 * Apple // SmartPort routines for CC65
 *
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 *
 */

/**
 * Check for SmartPort presence
 * @return slot #, or 0xFF if not present.
 */
uint8_t sp_find_slot(void)
{
  uint8_t s=0;

  for (s=7; s > 0; s--)
    {
      uint16_t a = 0xc000 + (s * 0x100);
      if ((PEEK(a+1) == 0x20) &&
          (PEEK(a+3) == 0x00) &&
          (PEEK(a+5) == 0x03) &&
          (PEEK(a+7) == 0x00))
        return s;
    }

  // Did not find.
  return 0;
}

/**
 * Return dispatch address for Smartport slot.
 * @param s Slot # (1-7)
 * @return smartport dispatch address
 */
uint16_t sp_dispatch_address(uint8_t slot)
{
  uint16_t a = (slot * 0x100) + 0xC000;
  uint8_t j = PEEK(a+0xFF);

  return a + j + 3;
}

#ifdef __ORCAC__
void sp_check_handle(void)
{
  if (sp_handle == NULL)
  {
    screen_error("COULD NOT ALLOCATE BUFFER!");
    revers(1);
    cprintf("\r\n PRESS ANY KEY TO EXIT \r\n");
    revers(0);
    cgetc();
    exit(0);
  };
}

void sp_get_buffer(void)
{
  if (PEEK(0xe100bc) == 0)  // Running under ProDOS 8
    {
      sp_handle = NewHandle(0x500, myId, 0x0000, 0L);
      sp_check_handle();
      sp_payload = (uint8_t *)0x800;
      sp_cmdlist = sp_payload + 1024;
      sp_instr = sp_payload + 1034;
      memcpy(*sp_handle, sp_payload, 0x500);
    }
  else  // Under GS/OS or ProDOS 16
    {
      sp_handle = NewHandle(0x500, myId, 0xC011, 0L);
      sp_check_handle();
      sp_payload = (uint8_t *)*sp_handle;
      sp_cmdlist = (uint8_t *)*sp_handle + 1024;
      sp_instr = (uint8_t *)*sp_handle + 1034;
    }
}
#endif

void sp_init(void)
{
  uint8_t slot, f;
#ifdef __ORCAC__
  MTStartUp();
  myId = MMStartUp();
  sp_get_buffer();
  slot = 5;
#else
  slot = (PEEK(0x43) & 0x70) >> 4; // determine slot used for booting, check unit number
#endif
  if (slot)
    sp_dispatch = sp_dispatch_address(slot);
  else
    screen_error("No SmartPort Firmware Found!");
  sp_list_devs();
  f = sp_find_fuji();
  if (f < 1)
    screen_error("FujiNet Not Found!");
}

#ifdef __ORCAC__
void sp_done(void)
{
  if (PEEK(0xe100bc) == 0)  // Running under ProDOS 8
    memcpy(sp_payload, *sp_handle, 0x500);
  DisposeHandle(sp_handle);
  MMShutDown(myId);
  MTShutDown();
}
#endif

#endif /* BUILD_APPLE2 */