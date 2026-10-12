#ifdef BUILD_H89

/**
 * @file ui.c
 * @brief The frame, hints, info and status lines - Heathkit H89.
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 */

#include <string.h>
#include "../globals.h"
#include "h19.h"
#include "layout.h"
#include "ui.h"

#ifdef __HDOS__
#define OS_NAME " H89 HDOS "
#else
#define OS_NAME " H89 CP/M "
#endif

/** "x name x" across line y: the corners or tees at each end */
static void rule(uint8_t y, uint8_t left, uint8_t right)
{
  h19_put(0, y, left);
  h19_fill(1, y, H19_COLS - 2, G_HLINE);
  h19_put(H19_COLS - 1, y, right);
}

void ui_frame(void)
{
  static uint8_t y;

  h19_begin();

  rule(FRAME_TOP_Y, G_CORNER_UL, G_CORNER_UR);
  h19_text(2, FRAME_TOP_Y, " FujiNet CONFIG ", 16, 0, false);
  h19_text(H19_COLS - 2 - (uint8_t)strlen(OS_NAME), FRAME_TOP_Y, OS_NAME, H19_COLS, 0, false);

  for (y = FRAME_TOP_Y + 1; y < FRAME_BOTTOM_Y; y++)
  {
    h19_put(0, y, G_VLINE);
    h19_put(H19_COLS - 1, y, G_VLINE);
  }

  ui_divider(FOOT_Y, NULL, NULL);
  rule(FRAME_BOTTOM_Y, G_CORNER_LL, G_CORNER_LR);
}

void ui_title_right(uint8_t y, const char *s)
{
  static uint8_t n, x;

  if (!s || !*s)
    return;
  n = (uint8_t)strlen(s);
  if (n > 40)
    n = 40;
  x = H19_COLS - 4 - n;
  h19_put(x, y, ' ');
  h19_text(x + 1, y, s, n, 0, false);
  h19_put(x + 1 + n, y, ' ');
}

void ui_divider(uint8_t y, const char *left, const char *right)
{
  static uint8_t x;

  rule(y, G_TEE_RIGHT, G_TEE_LEFT);

  if (left && *left)
  {
    x = h19_text(2, y, " ", 1, 0, false);
    x = h19_text(x, y, left, 60, 0, false);
    h19_put(x, y, ' ');
  }

  ui_title_right(y, right);
}

void ui_pane(uint8_t y, const char *title, bool active)
{
  static uint8_t x, attr;

  attr = active ? REV : 0;
  rule(y, G_TEE_RIGHT, G_TEE_LEFT);
  h19_put(2, y, ' ' | attr);
  x = h19_text(3, y, title, 60, attr, false);
  h19_put(x, y, ' ' | attr);
}

uint8_t ui_field_x;

void ui_prompt(const char *label)
{
  ui_info(label);
  ui_field_x = TEXT_X + (uint8_t)strlen(label) + 1;
}

void ui_center(uint8_t y, const char *s)
{
  static uint8_t n;

  n = (uint8_t)strlen(s);
  if (n > H19_COLS - 2)
    n = H19_COLS - 2;
  ui_line(TEXT_X, y, NULL);
  h19_text((H19_COLS - n) / 2, y, s, n, 0, false);
}

static void hint_line(uint8_t y, const char *s)
{
  static uint8_t x, attr, c;

  h19_fill(0, y, H19_COLS, ' ');
  if (!s)
    return;

  /* Leave the last column alone: nothing is ever written at the bottom
     right corner */
  x = 1;
  attr = 0;
  while ((c = (uint8_t)*s++) && x < H19_COLS - 1)
  {
    if (c == '[')
    {
      attr = REV;
      c = ' ';
    }
    else if (c == ']')
    {
      h19_put(x++, y, ' ' | REV);
      attr = 0;
      continue;
    }
    h19_put(x++, y, c | attr);
  }
}

void ui_hints(const char *line1, const char *line2)
{
  hint_line(HINT1_Y, line1);
  hint_line(HINT2_Y, line2);
}

void ui_line(uint8_t x, uint8_t y, const char *s)
{
  h19_fill(1, y, H19_COLS - 2, ' ');
  if (s)
    h19_text(x, y, s, H19_COLS - 2 - x, 0, false);
}

void ui_info(const char *s)
{
  ui_line(TEXT_X, INFO_Y, s);
}

void ui_message(const char *s)
{
  if (s)
    while (*s == ' ')
      s++;
  ui_info(s);
  h19_flush();
}

void ui_labelled(uint8_t y, const char *label, const char *value)
{
  static uint8_t x;

  ui_line(TEXT_X, y, NULL);
  x = h19_text(14 - (uint8_t)strlen(label), y, label, 12, 0, false);
  x = h19_text(x, y, ": ", 2, 0, false);
  h19_text(x, y, value, H19_COLS - 2 - x, 0, false);
}

static uint8_t status_part(uint8_t x, const char *label, const char *value)
{
  x = h19_text(x, STATUS_Y, label, H19_COLS - 1 - x, 0, false);
  x = h19_text(x, STATUS_Y, value, H19_COLS - 1 - x, 0, false);
  x = h19_text(x, STATUS_Y, " ", 1, 0, false);
  h19_put(x, STATUS_Y, G_CHECKER);
  return x + 1;
}

void ui_status(bool refresh)
{
  static uint8_t x;

  if (refresh)
    fuji_get_adapter_config_extended(&adapterConfigExt);

  h19_fill(0, STATUS_Y, H19_COLS, ' ');
  h19_put(0, STATUS_Y, G_CHECKER);
  x = 1;
  if (wifiEnabled && adapterConfigExt.ssid[0])
  {
    x = status_part(x, " SSID: ", adapterConfigExt.ssid);
    x = status_part(x, " IP: ", adapterConfigExt.sLocalIP);
  }
  else
    x = status_part(x, " WiFi: ", "not connected");
  if (adapterConfigExt.fn_version[0])
    status_part(x, " FujiNet ", adapterConfigExt.fn_version);
}

char *ui_num(uint32_t n, char *buf)
{
  static char digits[11];
  static uint8_t i;

  i = sizeof(digits) - 1;
  digits[i] = 0;
  do
  {
    digits[--i] = '0' + (uint8_t)(n % 10);
    n /= 10;
  } while (n && i);
  strcpy(buf, &digits[i]);
  return buf;
}

#endif /* BUILD_H89 */
