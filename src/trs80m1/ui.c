#ifdef BUILD_TRS80M1

/**
 * @file ui.c
 * @brief The title line, WiFi status, hints and info line - TRS-80 Model I.
 */

#include <string.h>
#include "../globals.h"
#include "vid.h"
#include "layout.h"
#include "ui.h"

#define LABEL_END 12 /* where ui_labelled's labels end */

/* The WiFi status, shown at the right of the title line */
static char status[50];

static void title_line(void)
{
  vid_fill(0, TITLE_Y, VID_COLS, ' ');
  vid_text(TEXT_X, TITLE_Y, "FUJINET CONFIG", 14, false);
  ui_title_right(TITLE_Y, status);
}

void ui_frame(void)
{
  vid_begin();
  title_line();
  ui_divider(FOOT_Y, NULL, NULL);
}

void ui_title_right(uint8_t row, const char *text)
{
  static uint8_t len;

  if (!text || !*text)
    return;
  len = (uint8_t)strlen(text);
  if (len > 46)
    len = 46;
  vid_text(VID_COLS - 1 - len, row, text, len, false);
}

void ui_divider(uint8_t row, const char *left, const char *right)
{
  static uint8_t col;

  vid_fill(0, row, VID_COLS, G_RULE);

  if (left && *left)
  {
    col = vid_text(TEXT_X, row, " ", 1, false);
    col = vid_text(col, row, left, 40, false);
    vid_put(col, row, ' ');
  }

  if (right && *right)
  {
    vid_put(VID_COLS - 2 - (uint8_t)strlen(right) - 1, row, ' ');
    vid_text(VID_COLS - 2 - (uint8_t)strlen(right), row, right, 40, false);
    vid_put(VID_COLS - 2, row, ' ');
  }
}

void ui_pane(uint8_t row, const char *title)
{
  static uint8_t col;

  vid_fill(0, row, VID_COLS, G_RULE);
  vid_put(TEXT_X, row, ' ');
  col = vid_text(TEXT_X + 1, row, title, VID_COLS - 6, false);
  vid_put(col, row, ' ');
}

uint8_t ui_field_x;

void ui_prompt(const char *label)
{
  ui_info(label);
  ui_field_x = TEXT_X + (uint8_t)strlen(label) + 1;
}

void ui_center(uint8_t row, const char *text)
{
  static uint8_t len;

  len = (uint8_t)strlen(text);
  if (len > VID_COLS)
    len = VID_COLS;
  ui_line(TEXT_X, row, NULL);
  vid_text((VID_COLS - len) / 2, row, text, len, false);
}

void ui_hints(const char *line1, const char *line2)
{
  ui_line(TEXT_X, HINT1_Y, line1);
  ui_line(TEXT_X, HINT2_Y, line2);
}

void ui_line(uint8_t col, uint8_t row, const char *text)
{
  vid_fill(0, row, VID_COLS, ' ');
  if (text)
    vid_text(col, row, text, VID_COLS - col, false);
}

void ui_info(const char *text)
{
  ui_line(TEXT_X, INFO_Y, text);
}

void ui_message(const char *text)
{
  if (text)
    while (*text == ' ')
      text++;
  ui_info(text);
  vid_flush();
}

void ui_labelled(uint8_t row, const char *label, const char *value)
{
  static uint8_t col;

  ui_line(TEXT_X, row, NULL);
  col = vid_text(LABEL_END - (uint8_t)strlen(label), row, label, LABEL_END, false);
  col = vid_text(col, row, ": ", 2, false);
  vid_text(col, row, value, VID_COLS - col, false);
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
  title_line();
}

char *ui_num(uint32_t len, char *buf)
{
  static char digits[11];
  static uint8_t idx;

  idx = sizeof(digits) - 1;
  digits[idx] = 0;
  do
  {
    digits[--idx] = '0' + (uint8_t)(len % 10);
    len /= 10;
  } while (len && idx);
  strcpy(buf, &digits[idx]);
  return buf;
}

#endif /* BUILD_TRS80M1 */
