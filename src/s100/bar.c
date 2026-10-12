#ifdef BUILD_S100
/**
 * Bar routines: the selected row is redrawn from the screen shadow in reverse video.
 */

#include "../screen.h"
#include "s100.h"

static uint8_t bar_y = 3, bar_c = 1, bar_m = 0, bar_i = 0, bar_oldi = 0;
/* The row drawn reversed, and how much of it */
static uint8_t hl_row = 0xFF, hl_len;

/* The screen was cleared: nothing is reversed any more */
void bar_forget(void)
{
    hl_row = 0xFF;
}

/* Only the text of the row is reversed, which keeps a bar move short on a slow line */
void bar_draw(int y, bool clear)
{
    const char *p = &s100_shadow[y * SCR_COLS + bar_c];
    uint8_t n = SCR_COLS - 2 * bar_c;

    while (n && p[n - 1] == ' ')
        n--;
    if (y == hl_row)
    {
        if (n < hl_len)
            n = hl_len;
        hl_row = 0xFF;
    }
    if (!clear)
    {
        hl_row = y;
        hl_len = n;
    }

    s100_goto(bar_c, y);
    if (!clear)
        s100_reverse(true);
    while (n--)
        con_putc(*p++);
    if (!clear)
        s100_reverse(false);
}

void bar_clear(bool oldRow)
{
    bar_draw(bar_y + (oldRow ? bar_oldi : bar_i), true);
}

void bar_update(void)
{
    if (hl_row != 0xFF)
        bar_draw(hl_row, true);
    bar_draw(bar_y + bar_i, false);
}

void bar_set(uint8_t y, uint8_t c, uint8_t m, uint8_t i)
{
    bar_y = y;
    bar_c = c;
    bar_m = m ? m - 1 : 0;
    bar_oldi = bar_i;
    bar_i = i;
    bar_update();
}

void bar_up(void)
{
    bar_oldi = bar_i;
    if (bar_i > 0)
    {
        bar_i--;
        bar_update();
    }
}

void bar_down(void)
{
    bar_oldi = bar_i;
    if (bar_i < bar_m)
    {
        bar_i++;
        bar_update();
    }
}

void bar_jump(uint_fast8_t i)
{
    bar_oldi = bar_i;
    bar_i = i;
    bar_update();
}

uint_fast8_t bar_get(void)
{
    return bar_i;
}

#endif /* BUILD_S100 */
