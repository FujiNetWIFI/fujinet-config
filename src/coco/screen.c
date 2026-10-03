#ifdef _CMOC_VERSION_

/**
 * FujiNet Configuration Program: CoCo screen core, drawn through the text backend (coco_text.h).
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
static byte cur_x, cur_y;
static char uppercase_tmp[32];

char *screen_upper(char *s)
{
  memset(uppercase_tmp, 0, sizeof(uppercase_tmp));
  strncpy(uppercase_tmp, s, sizeof(uppercase_tmp) - 1);
  return strupr(uppercase_tmp);
}

void screen_init(void)
{
  asm {
    lda $011A
    sta orig_casflag
    clr $011A
  }

  color_load();
  txt_open();
#ifdef COCO3
  if (monitor_unset)
    monitor_ask();
#endif
}

/* Before running another program: nothing of ours may stay hooked into Basic. */
void screen_handoff(void)
{
  txt_release();
  asm {
    lda orig_casflag
    sta $011A
  }
}

void screen_leave_graphics(void)
{
  txt_close();
}

void screen_loading(const char *what)
{
  char s[SCREEN_COLS];
  byte x;
  const char *p = s;

  for (x = 0; x < SCREEN_COLS; x++)
    txt_put(x, 0, ' ', ROLE_TEXT);

  strcpy(s, "Loading ");
  x = (byte)strlen(s);
  while (*what && x < SCREEN_COLS - 4)
    s[x++] = *what++;
  strcpy(s + x, "...");

  x = (SCREEN_COLS - (byte)strlen(s)) / 2;
  while (*p)
    txt_put(x++, 0, (byte)*p++, ROLE_TEXT);
}

/* c == 0 flips the cell, as the line editor's cursor does. */
void screen_put_role(byte x, byte y, byte c, byte role)
{
  if (c)
    txt_put(x, y, c, role);
  else
    txt_invert(x, y);
  if (bar_cell_lit(x, y))
    txt_invert(x, y);
}

void screen_puts_role(byte x, byte y, const char *s, byte role)
{
  while (*s && x < SCREEN_COLS - 1)
    screen_put_role(x++, y, (byte)*s++, role);
}

void screen_clear(void)
{
  screen_generation++;
  txt_clear();
  bar_unlit();
}

void screen_clear_line(unsigned char y)
{
  byte x;

  for (x = 1; x < SCREEN_COLS - 1; x++)
    screen_put_role(x, y, ' ', ROLE_TEXT);
}

void screen_put(int x, int y, uint8_t c)
{
  screen_put_role((byte)x, (byte)y, c, ROLE_TEXT);
}

void screen_puts(unsigned char x, unsigned char y, const char *s)
{
  screen_puts_role(x, y, s, ROLE_TEXT);
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

#ifdef COCO3
/* Shared code reports progress and success through screen_error() too. */
static bool is_status(const char *msg)
{
  return strncmp(msg, "CONNECTION SUCCESS", 18) == 0 || strncmp(msg, "PLEASE WAIT", 11) == 0;
}
#else
#define is_status(msg) false
#endif

void screen_error(const char *msg)
{
  /* Shared code names ESC; the CoCo key is BREAK. */
  if (strncmp(msg, "PLEASE WAIT...(ESC", 18) == 0)
    msg = "PLEASE WAIT...(BREAK TO ABORT)";
  screen_clear_line(msg_y);
  screen_puts_role(msg_center ? (SCREEN_COLS - (byte)strlen(msg)) / 2 : MENU_X, msg_y, msg,
                   is_status(msg) ? ROLE_STATUS : ROLE_ERROR);
  if (msg_hold)
    pause(msg_hold);
}

static void hline(byte x, byte y, byte n)
{
  while (n--)
    txt_put(x++, y, BOX_H, ROLE_BOX);
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
    txt_put(x, y + i, BOX_V, ROLE_BOX);
    txt_put(x + w - 1, y + i, BOX_V, ROLE_BOX);
  }
  txt_put(x, y, BOX_TL, ROLE_BOX);
  txt_put(x + w - 1, y, BOX_TR, ROLE_BOX);
  txt_put(x, y + h - 1, BOX_BL, ROLE_BOX);
  txt_put(x + w - 1, y + h - 1, BOX_BR, ROLE_BOX);
  if (n)
  {
    txt_put(tx - 1, y, ' ', ROLE_TEXT);
    screen_puts(tx, y, title);
    txt_put(tx + n, y, ' ', ROLE_TEXT);
  }
}

void screen_title(const char *title)
{
  byte x = (SCREEN_COLS - (byte)strlen(title)) / 2;

  while (*title)
    txt_put(x++, 0, (byte)*title++, ROLE_TITLE);
}

void screen_frame(const char *title)
{
  screen_clear();
  screen_message_target(STATUS_Y, false, 0);
  screen_title(title);
  screen_box(0, MENU_BOX_Y, SCREEN_COLS, SCREEN_ROWS - MENU_BOX_Y, NULL);
}

/* Menu text is written left to right from here. */
void screen_move(byte x, byte y)
{
  cur_x = x;
  cur_y = y;
}

static void print_role(const char *s, byte role)
{
  while (*s)
    txt_put(cur_x++, cur_y, (byte)*s++, role);
}

void screen_print_inverse(const char *s)
{
  print_role(s, ROLE_KEY);
}

/* Key cap reversed, then what it does. */
void screen_print_menu(const char *si, const char *sc)
{
  print_role(si, ROLE_KEY);
  print_role(sc, ROLE_TEXT);
}

#endif
