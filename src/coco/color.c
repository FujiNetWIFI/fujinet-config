#ifdef _CMOC_VERSION_

/**
 * Color set toggle (space bar in menus), saved in an app key. Callers redraw.
 * 0 green on black, 1 black on green, 2 white on black, 3 black on white.
 */

#include "coco_screen.h"
#include <fujinet-fuji.h>

#define AK_CREATOR_ID 0x0001
#define AK_APP_ID     0x02
#define AK_KEY_COLOR  1

byte colorset;

void color_load(void)
{
  uint8_t buf[64];
  uint16_t count = 0;

  fuji_set_appkey_details(AK_CREATOR_ID, AK_APP_ID, DEFAULT);
  if (fuji_read_appkey(AK_KEY_COLOR, &count, buf) && count >= 1 && buf[0] < 4)
    colorset = buf[0];
}

void color_toggle(void)
{
  if (++colorset >= 4)
    colorset = 0;

  screen(1, COLOR_CSS());
  setScreenInverted(COLOR_INVERTED());

  fuji_set_appkey_details(AK_CREATOR_ID, AK_APP_ID, DEFAULT);
  fuji_write_appkey(AK_KEY_COLOR, 1, &colorset);
}

#endif
