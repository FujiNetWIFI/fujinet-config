#ifdef BUILD_APPLE2
/**
 * FujiNet CONFIG for #Apple2
 *
 * SmartPort MLI Routines
 */

#include "sp.h"

unsigned char sp_payload[1024];
unsigned int sp_count;

static unsigned char sp_cmdlist[10];
static unsigned char sp_cmdlist_low, sp_cmdlist_high;
static unsigned char sp_dest, sp_err, sp_rtn_low, sp_rtn_high;

unsigned char sp_status(unsigned char statcode)
{
  // build the command list
  sp_cmdlist[0] = SP_CMD_STATUS;
  sp_cmdlist[1] = sp_dest; // set before calling sp_status();
  sp_cmdlist[2] = (unsigned char)((unsigned)&sp_payload & 0x00FF);
  sp_cmdlist[3] = (unsigned char)((unsigned)&sp_payload >> 8) & 0xFF;
  sp_cmdlist[4] = statcode;

  sp_cmdlist_low = (unsigned char)((unsigned)&sp_cmdlist & 0x00FF);
  sp_cmdlist_high = (unsigned char)((unsigned)&sp_cmdlist >> 8) & 0xFF;

  // store cmd list
  __asm__ volatile ("lda #$00");
  __asm__ volatile ("sta %g", spCmd); // store status command #
  __asm__ volatile ("lda %v", sp_cmdlist_low);
  __asm__ volatile ("sta %g", spCmdListLow); // store status command #
  __asm__ volatile ("lda %v", sp_cmdlist_high);
  __asm__ volatile ("sta %g", spCmdListHigh); // store status command #
    
  __asm__ volatile ("jsr $C50D");
spCmd:
  __asm__ volatile ("nop");
spCmdListLow:
  __asm__ volatile ("nop");
spCmdListHigh:
  __asm__ volatile ("nop");
  __asm__ volatile ("stx %v", sp_rtn_low);
  __asm__ volatile ("sty %v", sp_rtn_high);
  __asm__ volatile ("sta %v", sp_err);
  
  sp_count = ((unsigned)sp_rtn_high << 8) | (unsigned)sp_rtn_low;

  return sp_err;
}

#endif /* BUILD_APPLE2 */