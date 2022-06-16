#ifdef BUILD_ATARI

/**
 * FujiNet Configurator
 *
 * Functions to display a selection bar
 */

#include <atari.h>
#include <string.h>
#include "bar.h"

#define BAR_PMBASE	0x7c00
unsigned char* bar_pmbase=(unsigned char *)BAR_PMBASE;
static unsigned char bar_y=1, bar_c=1, bar_m=1, bar_i=0, bar_oldi=0, bar_co=0x13;

/**
 * Clear bar from screen
 */
void bar_clear(bool old)
{
  memset(bar_pmbase,0,1024);
}

/**
 * Set bar color
 */
void bar_set_color(unsigned char c)
{
  OS.pcolr0=OS.pcolr1=OS.pcolr2=OS.pcolr3=c;
}

/**
 * Get current bar position
 * @return bar index
 */
unsigned char bar_get()
{
    return bar_y;
}

void bar_up()
{
  bar_y--;

  bar_show(bar_y);
}

void bar_down()
{
  bar_y++;

  bar_show(bar_y);
}


/**
 * Update bar display
 */
void bar_update(void)
{
  
  bar_clear(true);
  
  // Fill bar color in new row
  //msx_vfill(ADDR_FILE_LIST + ROW(bar_y+bar_i),bar_co,256);
}

/**
 * Show bar at Y position
 */
void fastcall bar_show(unsigned char y)
{

  bar_y = y;

  asm ("asl");
  asm ("asl");
  asm ("adc #%b",16);
 
  asm ("pha");
  bar_clear(false);
  asm ("pla");
  asm ("tax");
  asm ("ldy #%b",4);
  asm ("lda #%b",-1);
lab:
  // First the missiles (rightmost portion of bar)
  asm ("sta %w,x",BAR_PMBASE+384);
  // Then the players (p0)
  asm ("sta %w,x",BAR_PMBASE+512);
  // P1
  asm ("sta %w,x",BAR_PMBASE+640);
  // P2
  asm ("sta %w,x",BAR_PMBASE+768);
  // P3
  asm ("sta %w,x",BAR_PMBASE+896);
  asm ("inx");
  asm ("dey");
  asm ("bne %g",lab);

}


#endif