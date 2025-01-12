#ifdef BUILD_CONFIG86

/**
 * Functions to display a selection bar
 */

#include <dos.h>
#include <stdbool.h>
#include "bar.h"
#include "screen.h"

extern unsigned char _screen_cols, _screen_mode, _screen_x, _screen_y, _screen_color;

/**
 * static local variables for bar y, max, and index.
 */
static int bar_y=3, bar_c=1, bar_m=1, bar_i=0, bar_oldi=0;

/**
 * Set up bar and start display on row
 * @param y Y column for bar display
 * @param c column size in characters
 * @param m number of items
 * @param i item index to start display
 */
void bar_set(unsigned char y, unsigned char c, unsigned char m, unsigned char i)
{
  bar_y = y;
  bar_c = c;
  bar_m = (m == 0 ? 0 : m-1);
  bar_i = i;
  bar_oldi = bar_i;
  bar_update();
}

void bar_clear(bool old)
{
    union REGS r;
    unsigned char col=0;

    for (col=0;col<_screen_cols;col++)
    {
        /* Position cursor */
        r.h.ah = 0x02;
        r.h.bh = 0;
        r.h.dl = col;
        r.h.dh = bar_y;

        if (old)
            r.h.dh += bar_oldi;
        else
            r.h.dh += bar_i;
        
        int86(0x10,&r,0);
        
        /* Get character */
        r.h.ah = 0x08;
        r.h.bh = 0;
        int86(0x10,&r,&r);

        /* Put character back with normal color */
        r.h.ah = 0x09;
        r.h.bh = 0;
        r.h.bl = 0x07;
        r.x.cx = 1;
        int86(0x10,&r,0);
    }
}

/**
 * Draw bar at y
 */
void bar_draw(int y, bool clear)
{
    union REGS r;
    unsigned char col=0;

    for (col=0;col<_screen_cols;col++)
    {
        /* Position cursor */
        r.h.ah = 0x02;
        r.h.bh = 0;
        r.h.dl = col;
        r.h.dh = y;
        int86(0x10,&r,0);
        
        /* Get character */
        r.h.ah = 0x08;
        r.h.bh = 0;
        int86(0x10,&r,&r);

        /* Put character back with bar color */
        r.h.ah = 0x09;
        r.h.bh = 0;
        r.h.bl = 0x70;
        r.x.cx = 1;
        int86(0x10,&r,0);
    }
    
}

/**
 * Get current bar position
 * @return bar index
 */
int bar_get()
{
  return bar_i;
}

/**
 * Move bar upward until index 0
 */
void bar_up()
{
    bar_oldi=bar_i;
  
    if (bar_i > 0)
    {
        bar_i--;
        bar_update();
    }
}

/**
 * Move bar downward until index m
 */
void bar_down()
{
    bar_oldi=bar_i;

    if (bar_i < bar_m)
    {
        bar_i++;
        bar_update();
    }
}

/**
 * @brief jump to bar position
 */
void bar_jump(int i)
{
    bar_oldi = bar_i;
    bar_i = i;
    bar_update();
}

/**
 * Update bar display
 */
void bar_update(void)
{  
    bar_clear(true);
    bar_draw(bar_y+bar_i,false);
}

#endif
