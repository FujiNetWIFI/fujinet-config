#ifdef BUILD_TRS80M1

/**
 * @file vid.c
 * @brief The Model I's 64x16 screen, drawn by cells and sent by difference.
 *
 * Two copies of the screen: want, which the screens draw into, and have,
 * what video memory at 3C00h holds. Video memory is read back without bit 6,
 * so it is never compared against directly. The bar and the cursor are laid
 * over want as it is sent.
 */

#include <string.h>
#include "vid.h"

#define VRAM ((volatile uint8_t *)0x3C00)

static uint8_t want[VID_COLS * VID_ROWS];
static uint8_t have[VID_COLS * VID_ROWS];
static uint8_t dirty[VID_ROWS];

static uint8_t bar_row = VID_NONE, bar_x0;
static uint8_t cursor_x = VID_NONE, cursor_y;

void vid_init(void)
{
  memset((void *)VRAM, ' ', sizeof(want));
  memset(want, ' ', sizeof(want));
  memset(have, ' ', sizeof(have));
  memset(dirty, 0, sizeof(dirty));
  bar_row = cursor_x = VID_NONE;
}

void vid_begin(void)
{
  memset(want, ' ', sizeof(want));
  memset(dirty, 1, sizeof(dirty));
  vid_bar(VID_NONE, 0);
  vid_cursor(VID_NONE, 0);
}

void vid_put(uint8_t col, uint8_t row, uint8_t code)
{
  if (col >= VID_COLS || row >= VID_ROWS)
    return;
  want[(uint16_t)row * VID_COLS + col] = code;
  dirty[row] = 1;
}

void vid_fill(uint8_t col, uint8_t row, uint8_t width, uint8_t code)
{
  if (row >= VID_ROWS || col >= VID_COLS)
    return;
  if (width > VID_COLS - col)
    width = VID_COLS - col;
  memset(&want[(uint16_t)row * VID_COLS + col], code, width);
  dirty[row] = 1;
}

/* No lower case on a stock Model I: letters go up, and the few ASCII
   characters whose codes show as arrows or nothing are swapped for a lookalike */
static uint8_t to_cell(uint8_t code)
{
  if (code >= 'a' && code <= 'z')
    return code - 0x20;
  switch (code)
  {
  case '[':
  case '{':
    return '(';
  case ']':
  case '}':
    return ')';
  case '\\':
    return '/';
  case '_':
  case '~':
    return '-';
  case '`':
    return '\'';
  case '|':
    return '!';
  case '^':
    return '+';
  }
  if (code < 0x20 || code > 0x7E)
    return '?';
  return code;
}

uint8_t vid_text(uint8_t col, uint8_t row, const char *text, uint8_t width, bool pad)
{
  static uint8_t *dest;

  if (row >= VID_ROWS || col >= VID_COLS)
    return col;
  if (width > VID_COLS - col)
    width = VID_COLS - col;

  dest = &want[(uint16_t)row * VID_COLS + col];
  dirty[row] = 1;

  while (width && *text)
  {
    *dest++ = to_cell((uint8_t)*text++);
    col++;
    width--;
  }

  if (pad)
    while (width--)
    {
      *dest++ = ' ';
      col++;
    }

  return col;
}

void vid_bar(uint8_t row, uint8_t col_first)
{
  if (bar_row < VID_ROWS)
    dirty[bar_row] = 1;
  bar_row = row;
  bar_x0 = col_first;
  if (bar_row < VID_ROWS)
    dirty[bar_row] = 1;
}

void vid_cursor(uint8_t col, uint8_t row)
{
  if (cursor_x != VID_NONE)
    dirty[cursor_y] = 1;
  cursor_x = col;
  cursor_y = row;
  if (cursor_x != VID_NONE)
    dirty[cursor_y] = 1;
}

void vid_flush_rows(uint8_t row_first, uint8_t row_last)
{
  static uint8_t row, col, cell;
  static uint16_t idx;

  for (row = row_first; row <= row_last && row < VID_ROWS; row++)
  {
    if (!dirty[row])
      continue;
    dirty[row] = 0;
    idx = (uint16_t)row * VID_COLS;
    for (col = 0; col < VID_COLS; col++, idx++)
    {
      cell = want[idx];
      if (row == bar_row && col == bar_x0)
        cell = '>';
      else if (row == cursor_y && col == cursor_x)
        cell = G_BLOCK;
      if (have[idx] != cell)
      {
        have[idx] = cell;
        VRAM[idx] = cell;
      }
    }
  }
}

void vid_flush(void)
{
  vid_flush_rows(0, VID_ROWS - 1);
}

#endif /* BUILD_TRS80M1 */
