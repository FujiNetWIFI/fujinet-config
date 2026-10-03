#ifdef _CMOC_VERSION_

/**
 * Color set change (SHIFT-LEFT/RIGHT in menus) and, on CoCo 3, RGB/composite (SHIFT-CLEAR), saved in app keys.
 */

#include "coco_screen.h"
#include <fujinet-fuji.h>

#define AK_CREATOR_ID 0x0001
#define AK_APP_ID     0x02
#define AK_KEY_COLOR  1
#define AK_KEY_MONITOR 2

byte colorset;
#ifdef COCO3
bool monitor_unset;
#endif

void color_load(void)
{
  uint8_t buf[MAX_APPKEY_LEN + 2];
  uint16_t count = 0;

  fuji_set_appkey_details(AK_CREATOR_ID, AK_APP_ID, DEFAULT);
  if (fuji_read_appkey(AK_KEY_COLOR, &count, buf) && count >= 1 && buf[0] < NUM_COLORSETS)
    colorset = buf[0];
#ifdef COCO3
  /* 1 = RGB, 2 = composite; a never-written key reads back as zeros. */
  if (fuji_read_appkey(AK_KEY_MONITOR, &count, buf) && (buf[0] == 1 || buf[0] == 2))
    composite = buf[0] - 1;
  else
    monitor_unset = true;
#endif
}

/* The CoCo library always sends MAX_APPKEY_LEN bytes from the buffer. */
static void save_key(byte key, byte value)
{
  byte buf[MAX_APPKEY_LEN];

  memset(buf, 0, sizeof(buf));
  buf[0] = value;
  fuji_set_appkey_details(AK_CREATOR_ID, AK_APP_ID, DEFAULT);
  fuji_write_appkey(key, 1, buf);
}

bool color_step(int dir)
{
  bool redraw;

  colorset = (byte)((colorset + (dir > 0 ? 1 : NUM_COLORSETS - 1)) % NUM_COLORSETS);
  redraw = txt_colors();
  save_key(AK_KEY_COLOR, colorset);
  return redraw;
}

#ifdef COCO3
void monitor_set(byte cmp)
{
  composite = cmp;
  monitor_unset = false;
  txt_colors();
  save_key(AK_KEY_MONITOR, composite + 1);
}

/* First run, before any screen content: RGB is assumed until the user answers. */
void monitor_ask(void)
{
  byte k;

  screen_frame("FujiNet Config");
  screen_puts(MENU_X, MENU_Y1, "Display? R = RGB, C = Composite");
  screen_puts(MENU_X, MENU_Y2, "SHIFT-CLEAR changes it later.");
  do
    k = inkey();
  while (k != 'R' && k != 'r' && k != 'C' && k != 'c' && k != 0x0D);
  monitor_set(k == 'C' || k == 'c');
  screen_clear();
}
#endif

#endif
