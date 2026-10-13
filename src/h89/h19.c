#ifdef BUILD_H89

/**
 * @file h19.c
 * @brief H19 terminal screen - Heathkit H89.
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 *
 * Two copies of the screen: want, which the screens draw into, and have,
 * what the terminal shows. h19_flush() sends the difference. A whole screen
 * is about 1.5K - a second and a half at 9600 baud - but the frame is the
 * same on every screen and the drive list on three of them, so moving
 * between screens costs only what really changes. The selection bar is laid
 * over want as it is sent, so the screens never have to remember a line's
 * text to put it back.
 */

#include <string.h>
#include "con.h"
#include "h19.h"

#define ESC 0x1B
#define UNKNOWN 0xFF
#define STALE_FOR_ERASE 4

static uint8_t want[H19_COLS * H19_ROWS];
static uint8_t have[H19_COLS * H19_ROWS];
static uint8_t dirty[H19_ROWS];

static uint8_t bar_row = H19_NONE, bar_x0, bar_x1;
static uint8_t cursor_x = H19_NONE, cursor_y;

/* What the terminal is doing now. UNKNOWN makes the next cell set it. */
static uint8_t cur_x, cur_y, cur_rev, cur_gfx, cur_shown;
static bool active;

static void esc(uint8_t c)
{
  con_out(ESC);
  con_out(c);
}

static void forget(void)
{
  cur_x = cur_y = cur_rev = cur_gfx = UNKNOWN;
}

void h19_init(void)
{
  con_init();

  esc('G');      /* graphics mode off */
  esc('q');      /* reverse video off */
  esc('x');      /* cursor off */
  con_out('5');
  esc('w');      /* discard at the end of a line: never wrap or scroll */
  esc('t');      /* keypad shifted: the unshifted keypad sends ESC A-D */
  esc('x');      /* the 25th line on */
  con_out('1');
  esc('E');      /* clear lines 1-24 */
  esc('Y');      /* and the 25th */
  con_out(32 + H19_STATUS_ROW);
  con_out(32);
  esc('l');

  memset(want, ' ', sizeof(want));
  memset(have, ' ', sizeof(have));
  memset(dirty, 0, sizeof(dirty));
  forget();
  cur_rev = cur_gfx = cur_shown = 0;
  bar_row = cursor_x = H19_NONE;
  active = true;
}

void h19_exit(void)
{
  static uint16_t t;

  if (!active)
    return;
  active = false;

  /* Back to the power-up configuration the DIP switches set - modes,
     cursor, keypad, the 25th line off - and a clear screen. The terminal
     needs a moment for it. */
  esc('y');
  con_out('1');
  esc('z');
  t = TICKS;
  while ((uint16_t)(TICKS - t) < 100)
    ;
  con_exit();
}

void h19_begin(void)
{
  memset(want, ' ', H19_COLS * H19_STATUS_ROW);
  memset(dirty, 1, H19_STATUS_ROW);
  h19_bar(H19_NONE, 0, 0);
  cursor_x = H19_NONE;
}

void h19_put(uint8_t x, uint8_t y, uint8_t c)
{
  if (x >= H19_COLS || y >= H19_ROWS)
    return;
  want[(uint16_t)y * H19_COLS + x] = c;
  dirty[y] = 1;
}

void h19_fill(uint8_t x, uint8_t y, uint8_t w, uint8_t c)
{
  if (y >= H19_ROWS || x >= H19_COLS)
    return;
  if (w > H19_COLS - x)
    w = H19_COLS - x;
  memset(&want[(uint16_t)y * H19_COLS + x], c, w);
  dirty[y] = 1;
}

uint8_t h19_text(uint8_t x, uint8_t y, const char *s, uint8_t w, uint8_t attr, bool pad)
{
  static uint8_t *p;
  static uint8_t c;

  if (y >= H19_ROWS || x >= H19_COLS)
    return x;
  if (w > H19_COLS - x)
    w = H19_COLS - x;

  p = &want[(uint16_t)y * H19_COLS + x];
  dirty[y] = 1;

  while (w && *s)
  {
    c = (uint8_t)*s++;
    /* Only the printable ASCII the H19 has; anything else (UTF-8 in a
       file name, say) shows as a question mark */
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

void h19_bar(uint8_t y, uint8_t x0, uint8_t x1)
{
  if (bar_row < H19_ROWS)
    dirty[bar_row] = 1;
  bar_row = y;
  bar_x0 = x0;
  bar_x1 = x1;
  if (bar_row < H19_ROWS)
    dirty[bar_row] = 1;
}

void h19_cursor(uint8_t x, uint8_t y)
{
  cursor_x = x;
  cursor_y = y;
}

void h19_bell(void)
{
  con_out(0x07);
}

/** The cell as it should look, with the bar laid over it */
static uint8_t cell(uint8_t x, uint8_t y, const uint8_t *row)
{
  if (y == bar_row && x >= bar_x0 && x <= bar_x1)
    return (x == bar_x0 ? G_ARROW_R : (row[x] & 0x7F)) | REV;
  return row[x];
}

/** 1 when the cell needs graphics mode, 0 when it needs it off, 2 when it
    looks the same either way */
static uint8_t gfx_of(uint8_t code)
{
  code &= 0x7F;
  if (code < 0x20 || code == 0x7F)
    return 1;
  if (code >= 0x5E)
    return 0;
  return 2;
}

/** Send one cell where the cursor is */
static void send(uint8_t code)
{
  static uint8_t g, m;

  if ((code & REV) != cur_rev)
  {
    cur_rev = code & REV;
    esc(cur_rev ? 'p' : 'q');
  }

  m = gfx_of(code);
  if (m != 2 && m != cur_gfx)
  {
    cur_gfx = m;
    esc(m ? 'F' : 'G');
  }

  g = code & 0x7F;
  if (g == 0x7F)
    g = '^';
  else if (g == 0x1F)
    g = '_';
  else if (g < 0x20)
    g += 0x60;
  con_out(g);

  /* With discard at the end of a line the cursor stays on the last column */
  cur_x = (cur_x == H19_COLS - 1) ? UNKNOWN : cur_x + 1;
}

/** Move the cursor to x on line y. A short hop along the line resends the
    cells in between when that is no dearer than ESC Y and keeps the modes. */
static void go(uint8_t x, uint8_t y, const uint8_t *row)
{
  static uint8_t i, c, m;

  if (cur_y == y && cur_x != UNKNOWN && cur_x < x && x - cur_x <= 3)
  {
    for (i = cur_x; i < x; i++)
    {
      c = cell(i, y, row);
      m = gfx_of(c);
      if ((c & REV) != cur_rev || (m != 2 && m != cur_gfx))
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
    esc('Y');
    con_out(32 + y);
    con_out(32 + x);
    cur_x = x;
    cur_y = y;
  }
}

static void flush_row(uint8_t y)
{
  static uint8_t *row, *hrow;
  static uint8_t x, e, last, stale, i;

  row = &want[(uint16_t)y * H19_COLS];
  hrow = &have[(uint16_t)y * H19_COLS];

  /* The last column before the right edge that is not blank: past it, a
     line with enough old text left on it is cleared with one ESC K */
  last = UNKNOWN;
  for (x = 0; x < H19_COLS - 1; x++)
    if (cell(x, y, row) != ' ')
      last = x;

  for (x = 0; x < H19_COLS; x++)
  {
    e = cell(x, y, row);
    if (hrow[x] == e)
      continue;

    if (e == ' ' && (last == UNKNOWN || x > last) && x < H19_COLS - 1)
    {
      stale = 0;
      for (i = x; i < H19_COLS - 1; i++)
        if (hrow[i] != ' ')
          stale++;
      if (stale >= STALE_FOR_ERASE)
      {
        go(x, y, row);
        if (cur_rev)
        {
          cur_rev = 0;
          esc('q');
        }
        esc('K');
        memset(&hrow[x], ' ', H19_COLS - x);
        continue;
      }
    }

    go(x, y, row);
    send(e);
    hrow[x] = e;
  }

  dirty[y] = 0;
}

void h19_flush_rows(uint8_t y0, uint8_t y1)
{
  static uint8_t y;

  if (!active)
    return;

  for (y = y0; y <= y1 && y < H19_ROWS; y++)
    if (dirty[y])
      flush_row(y);

  if (cursor_x != H19_NONE)
  {
    if (cur_x != cursor_x || cur_y != cursor_y)
    {
      esc('Y');
      con_out(32 + cursor_y);
      con_out(32 + cursor_x);
      cur_x = cursor_x;
      cur_y = cursor_y;
    }
    if (!cur_shown)
    {
      esc('y');
      con_out('5');
      cur_shown = 1;
    }
  }
  else if (cur_shown)
  {
    esc('x');
    con_out('5');
    cur_shown = 0;
  }
}

void h19_flush(void)
{
  h19_flush_rows(0, H19_ROWS - 1);
}

#endif /* BUILD_H89 */
