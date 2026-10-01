#include "scroll.h"
#include <fujinet-fuji.h>
#include "../constants.h"
#include "../globals.h"
#include "../select_file.h"

word lastTimer;
word idleCounter = IDLE_TIMEOUT_COUNT;
int scrollOffset = 0;
int scrollDir = 1;
byte got_scrolltext;

static void scroll_draw_line_at_y(const char *line, byte y, int offset)
{
  char buf[SCREEN_WIDTH + 1];
  int len = strlen(line);

  memset(buf, ' ', SCREEN_WIDTH);
  buf[SCREEN_WIDTH] = 0;

  if (offset < len)
  {
    int copyLen = len - offset;
    if (copyLen > SCREEN_WIDTH)
      copyLen = SCREEN_WIDTH;
    memcpy(buf, line + offset, copyLen);
  }

  screen_puts(FILE_X, FILES_Y + y, buf);
}

void scroll_step(void)
{
  byte y = (byte)bar_get();
  int len;

  if (!got_scrolltext)
  {
    select_get_filename(255);
    got_scrolltext = true;
  }

  len = strlen(response);
  if (len <= SCREEN_WIDTH)
    return;

  scrollOffset += scrollDir;
  if (scrollOffset < 0)
  {
    scrollOffset = 0;
    scrollDir = 1;
  }
  else if (scrollOffset > len - SCREEN_WIDTH)
  {
    scrollOffset = len - SCREEN_WIDTH;
    scrollDir = -1;
  }

  scroll_draw_line_at_y(response, y, scrollOffset);
}

void scroll_reset(bool init)
{
  byte y = (byte)bar_get();

  scrollOffset = 0;
  scrollDir = 1;
  idleCounter = IDLE_TIMEOUT_COUNT;
  if (init)
  {
    lastTimer = 0;
  }
  else if (got_scrolltext && strlen(response) > SCREEN_WIDTH)
  {
    select_get_filename(DIR_MAX_LEN);
    screen_select_file_display_entry(y, response, 0);
  }
  got_scrolltext = false;
}
