#ifdef BUILD_MODEL2

/**
 * @file probe.c
 * @brief Display and keyboard test - TRS-80 Model II. Replaced by CONFIG's
 * own screens once those are in.
 */

#include "con.h"
#include "vid.h"

#define KEYS_Y   13
#define TIMING_Y 18

static const char hex[] = "0123456789ABCDEF";

static void put_hex(uint8_t n)
{
  static char s[3];

  s[0] = hex[n >> 4];
  s[1] = hex[n & 15];
  s[2] = 0;
  vid_text(s);
}

static void edge(uint8_t y, uint8_t left, uint8_t fill, uint8_t right)
{
  vid_goto(0, y);
  vid_glyph(left);
  vid_glyphs(fill, VID_COLS - 2);
  vid_glyph(right);
}

static void divider(uint8_t y, const char *title)
{
  edge(y, G_TEE_RIGHT_THIN, G_HLINE_THIN, G_TEE_LEFT_THIN);
  vid_goto(2, y);
  vid_text(" ");
  vid_text(title);
  vid_text(" ");
}

static void line(uint8_t y)
{
  vid_goto(0, y);
  vid_glyph(G_VLINE);
  vid_goto(VID_COLS - 1, y);
  vid_glyph(G_VLINE);
}

static void draw(void)
{
  uint8_t y, i;

  vid_init();
  edge(0, G_CORNER_UL, G_HLINE, G_CORNER_UR);
  vid_goto(2, 0);
  vid_text(" FujiNet CONFIG ");
  vid_goto(56, 0);
  vid_text(" Model II display test ");

  for (y = 1; y < 21; y++)
    line(y);

  divider(1, "GRAPHICS");
  for (i = 0; i < 32; i++)
  {
    vid_goto(3 + (i & 7) * 9, 3 + (i >> 3));
    put_hex(i);
    vid_text(" ");
    vid_glyph(i);
    vid_text(" ");
    vid_reverse(true);
    vid_glyph(i);
    vid_reverse(false);
  }

  vid_goto(3, 8);
  vid_text("Normal text  ");
  vid_reverse(true);
  vid_text(" Reverse text ");
  vid_reverse(false);
  vid_text("  lower case abc xyz  Signal ");
  vid_glyph(G_BAR_1);
  vid_glyph(G_BAR_2);
  vid_glyph(G_BAR_3);
  vid_glyph(G_BAR_4);
  vid_text("  More ");
  vid_glyph(G_ARROW_UP);

  vid_goto(3, 10);
  vid_text("Joins:");
  vid_goto(12, 9);
  vid_glyph(G_CORNER_UL);
  vid_glyphs(G_HLINE, 3);
  vid_glyph(G_TEE_DOWN_THIN);
  vid_glyphs(G_HLINE, 3);
  vid_glyph(G_CORNER_UR);
  vid_goto(12, 10);
  vid_glyph(G_TEE_RIGHT_THIN);
  vid_glyphs(G_HLINE_THIN, 3);
  vid_glyph(G_CROSS_THIN);
  vid_glyphs(G_HLINE_THIN, 3);
  vid_glyph(G_TEE_LEFT_THIN);
  vid_goto(12, 11);
  vid_glyph(G_CORNER_LL);
  vid_glyphs(G_HLINE, 3);
  vid_glyph(G_TEE_UP_THIN);
  vid_glyphs(G_HLINE, 3);
  vid_glyph(G_CORNER_LR);

  vid_goto(24, 9);
  vid_glyph(G_CORNER_UL);
  vid_glyphs(G_HLINE, 3);
  vid_glyph(G_TEE_DOWN);
  vid_glyphs(G_HLINE, 3);
  vid_glyph(G_CORNER_UR);
  vid_goto(24, 10);
  vid_glyph(G_TEE_RIGHT);
  vid_glyphs(G_HLINE, 3);
  vid_glyph(G_CROSS);
  vid_glyphs(G_HLINE, 3);
  vid_glyph(G_TEE_LEFT);
  vid_goto(24, 11);
  vid_glyph(G_CORNER_LL);
  vid_glyphs(G_HLINE, 3);
  vid_glyph(G_TEE_UP);
  vid_glyphs(G_HLINE, 3);
  vid_glyph(G_CORNER_LR);

  divider(12, "KEYS");
  vid_goto(3, KEYS_Y);
  vid_text("Last key:");

  divider(17, "TIMING");
  divider(19, "");
  vid_goto(3, 20);
  vid_text("Press any key to see its code.");
  edge(21, G_CORNER_LL, G_HLINE, G_CORNER_LR);

  vid_goto(1, 22);
  vid_reverse(true);
  vid_text(" R ");
  vid_reverse(false);
  vid_text(" Redraw   ");
  vid_reverse(true);
  vid_text(" T ");
  vid_reverse(false);
  vid_text(" Time 5 redraws   ");
  vid_reverse(true);
  vid_text(" BREAK ");
  vid_reverse(false);
  vid_text(" Quit");
}

static const char *key_name(uint8_t k)
{
  switch (k)
  {
  case 0x01: return "F1";
  case 0x02: return "F2";
  case 0x03: return "BREAK";
  case 0x08: return "BACKSPACE";
  case 0x09: return "TAB";
  case 0x0D: return "ENTER";
  case 0x13: return "HOLD";
  case 0x1B: return "ESC";
  case 0x1C: return "LEFT ARROW";
  case 0x1D: return "RIGHT ARROW";
  case 0x1E: return "UP ARROW";
  case 0x1F: return "DOWN ARROW";
  case 0x20: return "SPACE";
  }
  return "";
}

static uint8_t history[16];

static void show_key(uint8_t k)
{
  static char s[2];
  uint8_t i;

  for (i = 0; i < sizeof(history) - 1; i++)
    history[i] = history[i + 1];
  history[i] = k;

  vid_goto(13, KEYS_Y);
  put_hex(k);
  vid_text("  ");
  if (k > 0x20 && k < 0x7F)
  {
    s[0] = k;
    s[1] = 0;
    vid_text("'");
    vid_text(s);
    vid_text("'");
  }
  else
    vid_text(key_name(k));
  vid_text("            ");

  vid_goto(3, KEYS_Y + 2);
  vid_text("Last 16:");
  for (i = 0; i < sizeof(history); i++)
  {
    vid_text(" ");
    put_hex(history[i]);
  }
}

static void put_dec(uint16_t n)
{
  static char s[6];
  uint8_t i = sizeof(s) - 1;

  s[i] = 0;
  do
  {
    s[--i] = '0' + n % 10;
    n /= 10;
  } while (n);
  vid_text(&s[i]);
}

/* Hundredths as seconds, two places */
static void put_secs(uint16_t t)
{
  put_dec(t / 100);
  vid_text(t % 100 < 10 ? ".0" : ".");
  put_dec(t % 100);
  vid_text(" s");
}

static void timing(void)
{
  uint16_t t;
  uint8_t i;

  t = (uint16_t)con_ticks();
  for (i = 0; i < 5; i++)
    draw();
  t = (uint16_t)con_ticks() - t;
  draw();
  vid_goto(3, TIMING_Y);
  vid_text("5 full redraws: ");
  put_secs(t);
  vid_text(", ");
  put_secs(t / 5);
  vid_text(" each");
}

int main(void)
{
  int k;

  draw();
  while (1)
  {
    k = con_in();
    if (k < 0)
      continue;
    if (k == 0x03)
      break;
    if (k == 'r' || k == 'R')
      draw();
    else if (k == 't' || k == 'T')
      timing();
    else
      show_key(k);
  }
  vid_exit();
  return 0;
}

#endif /* BUILD_MODEL2 */
