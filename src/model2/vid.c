#ifdef BUILD_MODEL2

/**
 * @file vid.c
 * @brief The Model II's 80x24 screen through the P&T video driver.
 *
 * P&T's control codes: ESC Y row+32 col+32 places the cursor, ^N/^O turn
 * reverse on and off, ^Q/^T graphics. In graphics mode, graphics character
 * n is sent as 40h+n. The modes are tracked to send each switch once.
 */

#include "con.h"
#include "vid.h"

#define CTL_CLEAR_EOL 0x01
#define CTL_CLEAR     0x0C
#define CTL_REV_ON    0x0E
#define CTL_REV_OFF   0x0F
#define CTL_GFX_ON    0x11
#define CTL_GFX_OFF   0x14
#define CTL_CURS_OFF  0x19
#define CTL_CURS_ON   0x1A
#define CTL_ESC       0x1B

static bool gfx, rev;

static void graphics(bool on)
{
  if (on != gfx)
  {
    con_out(on ? CTL_GFX_ON : CTL_GFX_OFF);
    gfx = on;
  }
}

static void reset(void)
{
  con_out(CTL_GFX_OFF);
  con_out(CTL_REV_OFF);
  gfx = rev = false;
  con_out(CTL_CLEAR);
}

void vid_init(void)
{
  reset();
  con_out(CTL_CURS_OFF);
}

void vid_exit(void)
{
  reset();
  con_out(CTL_CURS_ON);
}

void vid_goto(uint8_t x, uint8_t y)
{
  con_out(CTL_ESC);
  con_out('Y');
  con_out(y + 32);
  con_out(x + 32);
}

void vid_reverse(bool on)
{
  if (on != rev)
  {
    con_out(on ? CTL_REV_ON : CTL_REV_OFF);
    rev = on;
  }
}

void vid_glyph(uint8_t g)
{
  graphics(true);
  con_out(0x40 + g);
}

void vid_glyphs(uint8_t g, uint8_t n)
{
  graphics(true);
  while (n--)
    con_out(0x40 + g);
}

void vid_text(const char *s)
{
  graphics(false);
  while (*s)
    con_out(*s++);
}

void vid_clear_eol(void)
{
  con_out(CTL_CLEAR_EOL);
}

#endif /* BUILD_MODEL2 */
