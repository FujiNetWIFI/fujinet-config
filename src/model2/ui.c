#ifdef BUILD_MODEL2

/**
 * @file ui.c
 * @brief The frame, its WiFi status, hints and info line - TRS-80 Model II.
 */

#include <string.h>
#include "../globals.h"
#include "vid.h"
#include "layout.h"
#include "ui.h"

/* The WiFi status, shown at the right of the top border */
static char status[50];

/** A line across y, with the corners or tees at each end */
static void rule(uint8_t y, uint8_t left, uint8_t fill, uint8_t right)
{
  vid_put(0, y, left);
  vid_fill(1, y, VID_COLS - 2, fill);
  vid_put(VID_COLS - 1, y, right);
}

static void top_border(void)
{
  rule(FRAME_TOP_Y, G_CORNER_UL, G_HLINE, G_CORNER_UR);
  vid_text(2, FRAME_TOP_Y, " FujiNet CONFIG ", 16, 0, false);
  ui_title_right(FRAME_TOP_Y, status);
}

void ui_frame(void)
{
  static uint8_t y;

  vid_begin();
  top_border();

  for (y = FRAME_TOP_Y + 1; y < FRAME_BOTTOM_Y; y++)
  {
    vid_put(0, y, G_VLINE);
    vid_put(VID_COLS - 1, y, G_VLINE);
  }

  ui_divider(FOOT_Y, NULL, NULL);
  rule(FRAME_BOTTOM_Y, G_CORNER_LL, G_HLINE, G_CORNER_LR);
}

void ui_title_right(uint8_t y, const char *s)
{
  static uint8_t n, x;

  if (!s || !*s)
    return;
  n = (uint8_t)strlen(s);
  if (n > 40)
    n = 40;
  x = VID_COLS - 4 - n;
  vid_put(x, y, ' ');
  vid_text(x + 1, y, s, n, 0, false);
  vid_put(x + 1 + n, y, ' ');
}

void ui_divider(uint8_t y, const char *left, const char *right)
{
  static uint8_t x;

  rule(y, G_TEE_RIGHT_THIN, G_HLINE_THIN, G_TEE_LEFT_THIN);

  if (left && *left)
  {
    x = vid_text(2, y, " ", 1, 0, false);
    x = vid_text(x, y, left, 60, 0, false);
    vid_put(x, y, ' ');
  }

  ui_title_right(y, right);
}

void ui_pane(uint8_t y, const char *title, bool active)
{
  static uint8_t x, attr;

  attr = active ? REV : 0;
  rule(y, G_TEE_RIGHT_THIN, G_HLINE_THIN, G_TEE_LEFT_THIN);
  vid_put(2, y, ' ' | attr);
  x = vid_text(3, y, title, 60, attr, false);
  vid_put(x, y, ' ' | attr);
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
  if (n > VID_COLS - 2)
    n = VID_COLS - 2;
  ui_line(TEXT_X, y, NULL);
  vid_text((VID_COLS - n) / 2, y, s, n, 0, false);
}

static void hint_line(uint8_t y, const char *s)
{
  static uint8_t x, attr, c;

  vid_fill(1, y, VID_COLS - 2, ' ');
  if (!s)
    return;

  x = TEXT_X - 1;
  attr = 0;
  while ((c = (uint8_t)*s++) && x < VID_COLS - 2)
  {
    if (c == '[')
    {
      attr = REV;
      c = ' ';
    }
    else if (c == ']')
    {
      vid_put(x++, y, ' ' | REV);
      attr = 0;
      continue;
    }
    vid_put(x++, y, c | attr);
  }
}

void ui_hints(const char *line1, const char *line2)
{
  hint_line(HINT1_Y, line1);
  hint_line(HINT2_Y, line2);
}

void ui_line(uint8_t x, uint8_t y, const char *s)
{
  vid_fill(1, y, VID_COLS - 2, ' ');
  if (s)
    vid_text(x, y, s, VID_COLS - 2 - x, 0, false);
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
  vid_flush();
}

void ui_labelled(uint8_t y, const char *label, const char *value)
{
  static uint8_t x;

  ui_line(TEXT_X, y, NULL);
  x = vid_text(14 - (uint8_t)strlen(label), y, label, 12, 0, false);
  x = vid_text(x, y, ": ", 2, 0, false);
  vid_text(x, y, value, VID_COLS - 2 - x, 0, false);
}

void ui_status(bool refresh)
{
  if (refresh)
    fuji_get_adapter_config_extended(&adapterConfigExt);

  if (wifiEnabled && adapterConfigExt.ssid[0])
  {
    strncpy(status, adapterConfigExt.ssid, 32);
    status[32] = 0;
    strcat(status, " ");
    strncat(status, adapterConfigExt.sLocalIP, 15);
  }
  else
    strcpy(status, "WiFi not connected");
  top_border();
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

#endif /* BUILD_MODEL2 */
