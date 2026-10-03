#ifdef _CMOC_VERSION_

/**
 * Selection bar: the current row is shown by inverting its cells.
 */

#include "coco_screen.h"

static uint_fast8_t bar_y = 3, bar_m = 1, bar_i = 0, bar_oldi = 0;
static byte lit_row = 0xFF;
static byte span_x0 = 1, span_x1 = SCREEN_COLS - 2;

static void invert_row(byte y)
{
  byte x;

  for (x = span_x0; x <= span_x1; x++)
    txt_invert(x, y);
}

void bar_unlit(void)
{
  lit_row = 0xFF;
}

bool bar_cell_lit(byte x, byte y)
{
  return lit_row == y && x >= span_x0 && x <= span_x1;
}

void bar_draw(int y, bool clear)
{
  if (clear)
  {
    if (lit_row == (byte)y)
    {
      invert_row((byte)y);
      lit_row = 0xFF;
    }
    return;
  }

  if (lit_row == (byte)y)
    return;
  if (lit_row != 0xFF)
    invert_row(lit_row);
  invert_row((byte)y);
  lit_row = (byte)y;
}

void bar_clear(bool old)
{
  bar_draw(bar_y + (old ? bar_oldi : bar_i), true);
}

void bar_update(void)
{
  bar_clear(true);
  bar_draw(bar_y + bar_i, false);
}

/* c is the inset, in columns, from each screen edge. */
void bar_set(unsigned char y, unsigned char c, unsigned char m, unsigned char i)
{
  if (lit_row != 0xFF)
    bar_draw(lit_row, true);
  span_x0 = c;
  span_x1 = SCREEN_COLS - 1 - c;
  bar_y = y;
  bar_m = (m == 0 ? 0 : m - 1);
  bar_i = i;
  bar_oldi = bar_i;
  bar_update();
}

uint_fast8_t bar_get(void)
{
  return bar_i;
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

#endif
