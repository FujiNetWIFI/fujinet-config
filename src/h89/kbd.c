#ifdef BUILD_H89

/**
 * @file kbd.c
 * @brief H19 keyboard - Heathkit H89.
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 *
 * The H19 sends its keypad and function keys as escape sequences: with the
 * keypad shifted (h19_init sends ESC t) the arrows are ESC A-D, f1-f5 are
 * ESC S-W and BLUE/RED/GRAY ESC P-R. Those arrive back to back at 9600 baud,
 * so an ESC with nothing behind it within a few ticks is the ESC key itself.
 */

#include <stdbool.h>
#include "../input.h"
#include "con.h"
#include "h19.h"
#include "kbd.h"

#define ESC_WAIT_TICKS 10 /* 20 ms */

/* A key read after an ESC that wasn't part of a sequence */
static int held = -1;

static int next_byte(void)
{
  static int c;

  if (held >= 0)
  {
    c = held;
    held = -1;
    return c;
  }
  return con_in();
}

/** The byte after an ESC, or -1 if none comes in time */
static int esc_follower(void)
{
  static uint16_t t;
  static int c;

  t = TICKS;
  do
  {
    c = con_in();
    if (c >= 0)
      return c;
  } while ((uint16_t)(TICKS - t) < ESC_WAIT_TICKS);

  return -1;
}

static uint8_t decode_esc(void)
{
  static int n;

  n = esc_follower();
  switch (n)
  {
  case 'A': return KEY_UP_ARROW;
  case 'B': return KEY_DOWN_ARROW;
  case 'C': return KEY_RIGHT_ARROW;
  case 'D': return KEY_LEFT_ARROW;
  case 'H': return KEY_HOME;
  case '@':                       /* IC enters insert mode... */
  case 'O': return KEY_HOME;      /* ...and leaves it again */
  case 'L': return KEY_END;       /* IL */
  case 'N': return KEY_PAGE_UP;   /* DC */
  case 'M': return KEY_PAGE_DOWN; /* DL */
  case 'J': return KEY_ERASE;
  case 'E': return KEY_CLEAR;
  case 'S': return KEY_F1;
  case 'T': return KEY_F2;
  case 'U': return KEY_F3;
  case 'V': return KEY_F4;
  case 'W': return KEY_F5;
  case 'P': return KEY_BLUE;
  case 'Q': return KEY_RED;
  case 'R': return KEY_GRAY;
  default:
    /* A lone ESC, or ESC and then an ordinary key: keep that for next time */
    if (n >= 0)
      held = n;
    return KEY_ESCAPE;
  }
}

uint8_t kbd_get(void)
{
  static int c;

  c = next_byte();
  if (c < 0)
    return 0;
#ifdef H89_RIG_KEYS
  /* The headless rig can only type printable keys; ` stands in for ESC */
  if (c == '`')
    return KEY_ESCAPE;
#endif

  c &= 0x7F;
  if (c == KEY_ESCAPE)
    return decode_esc();
  if (c == 0)
    return 0;
  return (uint8_t)c;
}

uint8_t kbd_wait(void)
{
  static uint8_t k;

  h19_flush();
  while (!(k = kbd_get()))
    ;
  return k;
}

#endif /* BUILD_H89 */
