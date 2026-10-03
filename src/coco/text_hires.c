#if defined(_CMOC_VERSION_) && !defined(COCO3)

/**
 * CoCo 1/2 text backend: hirestxt 42x24 on PMODE 4.
 */

#include "coco_screen.h"
#include <hirestxt.h>

#define SCREEN_BUFFER ((byte *)0x0E00)

static bool hooked;

void txt_open(void)
{
  struct HiResTextScreenInit init =
    {
      SCREEN_COLS,
      writeCharAt_42cols,
      SCREEN_BUFFER,
      TRUE,
      (word *)0x112,
      0,
      NULL,
      NULL,
    };

  width(32);
  pmode(4, SCREEN_BUFFER);
  pcls(255);
  screen(1, colorset >> 1);
  initHiResTextScreen(&init);
  hooked = true;
  setScreenInverted(!(colorset & 1));
  clear();
}

void txt_release(void)
{
  if (hooked)
  {
    closeHiResTextScreen();
    hooked = false;
  }
}

void txt_close(void)
{
  txt_release();
  width(32);
  pmode(0, 0);
  screen(0, 0);
}

void txt_clear(void)
{
  clear();
}

void txt_put(byte x, byte y, byte c, byte role)
{
  bool bold = role == ROLE_TITLE || role == ROLE_WRITE;
  bool inverse = role == ROLE_KEY;

  if (bold)
    setBoldMode(TRUE);
  if (inverse)
    setInverseVideoMode(TRUE);
  writeCharAt_42cols(x, y, c);
  if (bold)
    setBoldMode(FALSE);
  if (inverse)
    setInverseVideoMode(FALSE);
}

void txt_invert(byte x, byte y)
{
  writeCharAt_42cols(x, y, 0);
}

bool txt_colors(void)
{
  screen(1, colorset >> 1);
  setScreenInverted(!(colorset & 1));
  return true;
}

#endif
