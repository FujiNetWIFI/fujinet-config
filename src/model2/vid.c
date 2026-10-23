#ifdef BUILD_MODEL2

/**
 * @file vid.c
 * @brief The Model II's 80x24 screen, drawn by cells and sent by difference.
 *
 * Two copies of the screen: want, which the screens draw into, and have,
 * what the screen shows. vid_flush() sends the difference through P&T's
 * video driver, about half a second for a whole screen, so moving between
 * screens costs only what really changes. The selection bar is laid over
 * want as it is sent.
 *
 * P&T's control codes: ESC Y row+32 col+32 places the cursor, ^N/^O turn
 * reverse on and off, ^Q/^T graphics. In graphics mode every character is a
 * graphic, graphics character n sent as 40h+n.
 */

#include <string.h>
#include "con.h"
#include "vid.h"

#define CTL_BELL      0x07
#define CTL_CLEAR_EOL 0x01
#define CTL_CLEAR     0x0C
#define CTL_REV_ON    0x0E
#define CTL_REV_OFF   0x0F
#define CTL_GFX_ON    0x11
#define CTL_GFX_OFF   0x14
#define CTL_CURS_OFF  0x19
#define CTL_CURS_ON   0x1A
#define CTL_ESC       0x1B

#define UNKNOWN 0xFF
#define STALE_FOR_ERASE 4

static uint8_t want[VID_COLS * VID_ROWS];
static uint8_t have[VID_COLS * VID_ROWS];
static uint8_t dirty[VID_ROWS];

static uint8_t bar_row = VID_NONE, bar_x0, bar_x1;
static uint8_t cursor_x = VID_NONE, cursor_y;

/* What the driver is doing now. UNKNOWN makes the next cell set it. */
static uint8_t cur_x, cur_y, cur_rev, cur_gfx, cur_shown;
static bool active;

static void modes_off(void)
{
  con_out(CTL_GFX_OFF);
  con_out(CTL_REV_OFF);
}

void vid_init(void)
{
  modes_off();
  con_out(CTL_CURS_OFF);
  con_out(CTL_CLEAR);

  memset(want, ' ', sizeof(want));
  memset(have, ' ', sizeof(have));
  memset(dirty, 0, sizeof(dirty));
  cur_x = cur_y = UNKNOWN;
  cur_rev = cur_gfx = cur_shown = 0;
  bar_row = cursor_x = VID_NONE;
  active = true;
}

void vid_exit(void)
{
  if (!active)
    return;
  active = false;

  modes_off();
  con_out(CTL_CLEAR);
  con_out(CTL_CURS_ON);
}

void vid_begin(void)
{
  memset(want, ' ', sizeof(want));
  memset(dirty, 1, sizeof(dirty));
  vid_bar(VID_NONE, 0, 0);
  cursor_x = VID_NONE;
}

void vid_put(uint8_t x, uint8_t y, uint8_t c)
{
  if (x >= VID_COLS || y >= VID_ROWS)
    return;
  want[(uint16_t)y * VID_COLS + x] = c;
  dirty[y] = 1;
}

void vid_fill(uint8_t x, uint8_t y, uint8_t w, uint8_t c)
{
  if (y >= VID_ROWS || x >= VID_COLS)
    return;
  if (w > VID_COLS - x)
    w = VID_COLS - x;
  memset(&want[(uint16_t)y * VID_COLS + x], c, w);
  dirty[y] = 1;
}

uint8_t vid_text(uint8_t x, uint8_t y, const char *s, uint8_t w, uint8_t attr, bool pad)
{
  static uint8_t *p;
  static uint8_t c;

  if (y >= VID_ROWS || x >= VID_COLS)
    return x;
  if (w > VID_COLS - x)
    w = VID_COLS - x;

  p = &want[(uint16_t)y * VID_COLS + x];
  dirty[y] = 1;

  while (w && *s)
  {
    c = (uint8_t)*s++;
    /* Printable ASCII only; anything else (UTF-8 in a file name, say)
       shows as a question mark */
    if (c < 0x20 || c > 0x7E)
      c = '?';
    *p++ = c | attr;
    x++;
    w--;
  }

  if (pad)
    while (w--)
    {
      *p++ = ' ' | attr;
      x++;
    }

  return x;
}

void vid_bar(uint8_t y, uint8_t x0, uint8_t x1)
{
  if (bar_row < VID_ROWS)
    dirty[bar_row] = 1;
  bar_row = y;
  bar_x0 = x0;
  bar_x1 = x1;
  if (bar_row < VID_ROWS)
    dirty[bar_row] = 1;
}

void vid_cursor(uint8_t x, uint8_t y)
{
  cursor_x = x;
  cursor_y = y;
}

void vid_bell(void)
{
  con_out(CTL_BELL);
}

/** The cell as it should look, with the bar laid over it */
static uint8_t cell(uint8_t x, uint8_t y, const uint8_t *row)
{
  if (y == bar_row && x >= bar_x0 && x <= bar_x1)
    return (x == bar_x0 ? '>' : (row[x] & 0x7F)) | REV;
  return row[x];
}

static uint8_t is_gfx(uint8_t code)
{
  return (code & 0x7F) < 0x20;
}

/** Send one cell where the cursor is */
static void send(uint8_t code)
{
  static uint8_t m;

  if ((code & REV) != cur_rev)
  {
    cur_rev = code & REV;
    con_out(cur_rev ? CTL_REV_ON : CTL_REV_OFF);
  }

  m = is_gfx(code);
  if (m != cur_gfx)
  {
    cur_gfx = m;
    con_out(m ? CTL_GFX_ON : CTL_GFX_OFF);
  }

  code &= 0x7F;
  con_out(m ? 0x40 + code : code);

  /* Past the last column the driver wraps or stays put, as set up in P&T */
  cur_x = (cur_x == VID_COLS - 1) ? UNKNOWN : cur_x + 1;
}

/** Move the cursor to x on line y. A short hop along the line resends the
    cells in between when that is no longer than ESC Y and keeps the modes. */
static void go(uint8_t x, uint8_t y, const uint8_t *row)
{
  static uint8_t i, c;

  if (cur_y == y && cur_x != UNKNOWN && cur_x < x && x - cur_x <= 3)
  {
    for (i = cur_x; i < x; i++)
    {
      c = cell(i, y, row);
      if ((c & REV) != cur_rev || is_gfx(c) != cur_gfx)
        break;
    }
    if (i == x)
    {
      for (i = cur_x; i < x; i++)
        send(cell(i, y, row));
      return;
    }
  }

  if (cur_x != x || cur_y != y)
  {
    con_out(CTL_ESC);
    con_out('Y');
    con_out(32 + y);
    con_out(32 + x);
    cur_x = x;
    cur_y = y;
  }
}

static void flush_row(uint8_t y)
{
  static uint8_t *row, *hrow;
  static uint8_t x, e, last, stale, i, end;
  static uint16_t off;

  row = &want[(uint16_t)y * VID_COLS];
  hrow = &have[(uint16_t)y * VID_COLS];

  /* Printing the bottom right cell scrolls the screen in wrap mode, which
     only SETMISC knows about; it goes straight into video memory instead */
  end = (y == VID_ROWS - 1) ? VID_COLS - 1 : VID_COLS;
  if (end < VID_COLS)
  {
    e = cell(end, y, row);
    if (hrow[end] != e)
    {
      off = (uint16_t)y * VID_COLS + end;
      con_vram_put(off, e);
      hrow[end] = e;
    }
  }

  /* The last column before the right edge that is not blank: past it, a
     line with enough old text left on it is cleared with one ^A */
  last = UNKNOWN;
  for (x = 0; x < VID_COLS - 1; x++)
    if (cell(x, y, row) != ' ')
      last = x;

  for (x = 0; x < end; x++)
  {
    e = cell(x, y, row);
    if (hrow[x] == e)
      continue;

    if (e == ' ' && (last == UNKNOWN || x > last) && x < VID_COLS - 1)
    {
      stale = 0;
      for (i = x; i < VID_COLS - 1; i++)
        if (hrow[i] != ' ')
          stale++;
      if (stale >= STALE_FOR_ERASE)
      {
        go(x, y, row);
        if (cur_rev)
        {
          cur_rev = 0;
          con_out(CTL_REV_OFF);
        }
        con_out(CTL_CLEAR_EOL);
        memset(&hrow[x], ' ', VID_COLS - x);
        continue;
      }
    }

    go(x, y, row);
    send(e);
    hrow[x] = e;
  }

  dirty[y] = 0;
}

void vid_flush_rows(uint8_t y0, uint8_t y1)
{
  static uint8_t y;

  if (!active)
    return;

  for (y = y0; y <= y1 && y < VID_ROWS; y++)
    if (dirty[y])
      flush_row(y);

  if (cursor_x != VID_NONE)
  {
    if (cur_x != cursor_x || cur_y != cursor_y)
    {
      con_out(CTL_ESC);
      con_out('Y');
      con_out(32 + cursor_y);
      con_out(32 + cursor_x);
      cur_x = cursor_x;
      cur_y = cursor_y;
    }
    if (!cur_shown)
    {
      con_out(CTL_CURS_ON);
      cur_shown = 1;
    }
  }
  else if (cur_shown)
  {
    con_out(CTL_CURS_OFF);
    cur_shown = 0;
  }
}

void vid_flush(void)
{
  vid_flush_rows(0, VID_ROWS - 1);
}

#endif /* BUILD_MODEL2 */
