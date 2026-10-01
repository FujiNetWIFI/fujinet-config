#ifdef _CMOC_VERSION_

/**
 * FujiNet Configuration Program: CoCo screen core (hirestxt 42x24).
 * Per-screen code is in screen_hosts.c, screen_slots.c, screen_files.c and screen_wifi.c.
 */

#include "coco_screen.h"
#include "../globals.h"
#include "../pause.h"

char text_empty[] = "<Empty>";
unsigned screen_generation;
static byte orig_casflag;
static byte msg_y = STATUS_Y;
static bool msg_center;
static byte msg_hold;
static char uppercase_tmp[32];

char *screen_upper(char *s)
{
  memset(uppercase_tmp, 0, sizeof(uppercase_tmp));
  strncpy(uppercase_tmp, s, sizeof(uppercase_tmp) - 1);
  return strupr(uppercase_tmp);
}

void screen_init(void)
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

  asm {
    lda $011A
    sta orig_casflag
    clr $011A
  }

  color_load();
  width(32);
  pmode(4, SCREEN_BUFFER);
  pcls(255);
  screen(1, COLOR_CSS());
  initHiResTextScreen(&init);
  setScreenInverted(COLOR_INVERTED());
  clear();
}

void screen_handoff(void)
{
  asm {
    lda orig_casflag
    sta $011A
  }
}

void screen_loading(const char *what)
{
  char s[SCREEN_COLS];
  byte x;
  const char *p = s;

  setBoldMode(FALSE);
  setInverseVideoMode(FALSE);
  for (x = 0; x < SCREEN_COLS; x++)
    writeCharAt_42cols(x, 0, ' ');

  strcpy(s, "Loading ");
  x = (byte)strlen(s);
  while (*what && x < SCREEN_COLS - 4)
    s[x++] = *what++;
  strcpy(s + x, "...");

  x = (SCREEN_COLS - (byte)strlen(s)) / 2;
  while (*p)
    writeCharAt_42cols(x++, 0, (byte)*p++);
}

void screen_end(void)
{
  screen_handoff();
  closeHiResTextScreen();
  width(32);
  pmode(0, 0);
  screen(0, 0);
  cls(255);
}

static void put_cell(byte x, byte y, byte c)
{
  writeCharAt_42cols(x, y, c);
  if (bar_cell_lit(x, y))
    writeCharAt_42cols(x, y, 0);
}

void screen_clear(void)
{
  screen_generation++;
  clear();
  bar_unlit();
}

void screen_clear_line(unsigned char y)
{
  byte x;

  for (x = 1; x < SCREEN_COLS - 1; x++)
    put_cell(x, y, ' ');
}

void screen_put(int x, int y, uint8_t c)
{
  put_cell((byte)x, (byte)y, c);
}

void screen_puts(unsigned char x, unsigned char y, const char *s)
{
  while (*s && x < SCREEN_COLS - 1)
    put_cell(x++, y, (byte)*s++);
}

void screen_menu_clear(void)
{
  screen_clear_line(MENU_Y1);
  screen_clear_line(MENU_Y2);
}

void screen_prompt(const char *s)
{
  screen_menu_clear();
  screen_puts(MENU_X, MENU_Y1, s);
}

/* Where screen_error() draws; hold pauses (ticks) so a message about to be left behind can be read. */
void screen_message_target(byte y, bool center, byte hold)
{
  msg_y = y;
  msg_center = center;
  msg_hold = hold;
}

void screen_error(const char *msg)
{
  screen_clear_line(msg_y);
  screen_puts(msg_center ? (SCREEN_COLS - (byte)strlen(msg)) / 2 : MENU_X, msg_y, msg);
  if (msg_hold)
    pause(msg_hold);
}

static void hline(byte x, byte y, byte n)
{
  while (n--)
    writeCharAt_42cols(x++, y, BOX_H);
}

void screen_box(byte x, byte y, byte w, byte h, const char *title)
{
  byte i;
  byte n = title ? (byte)strlen(title) : 0;
  byte tx = x + (w - n) / 2;

  hline(x + 1, y, w - 2);
  hline(x + 1, y + h - 1, w - 2);
  for (i = 1; i < h - 1; i++)
  {
    writeCharAt_42cols(x, y + i, BOX_V);
    writeCharAt_42cols(x + w - 1, y + i, BOX_V);
  }
  writeCharAt_42cols(x, y, BOX_TL);
  writeCharAt_42cols(x + w - 1, y, BOX_TR);
  writeCharAt_42cols(x, y + h - 1, BOX_BL);
  writeCharAt_42cols(x + w - 1, y + h - 1, BOX_BR);
  if (n)
  {
    writeCharAt_42cols(tx - 1, y, ' ');
    screen_puts(tx, y, title);
    writeCharAt_42cols(tx + n, y, ' ');
  }
}

void screen_title(const char *title)
{
  setBoldMode(TRUE);
  moveCursor((SCREEN_COLS - (byte)strlen(title)) / 2, 0);
  writeString(title);
  setBoldMode(FALSE);
}

void screen_frame(const char *title)
{
  screen_clear();
  screen_message_target(STATUS_Y, false, 0);
  screen_title(title);
  screen_box(0, MENU_BOX_Y, SCREEN_COLS, SCREEN_ROWS - MENU_BOX_Y, NULL);
}

void screen_print_inverse(const char *s)
{
  setInverseVideoMode(TRUE);
  writeString(s);
  setInverseVideoMode(FALSE);
}

/* Key cap reversed, then what it does. */
void screen_print_menu(const char *si, const char *sc)
{
  screen_print_inverse(si);
  writeString(sc);
}

#endif
