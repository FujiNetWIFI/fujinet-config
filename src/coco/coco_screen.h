#ifndef COCO_SCREEN_H
#define COCO_SCREEN_H

#include <cmoc.h>
#include <coco.h>
#include <hirestxt.h>
#include "../screen.h"

#define SCREEN_COLS   42
#define SCREEN_ROWS   24
#define SCREEN_BUFFER ((byte *)0x0E00)

#define LIST_X        1
#define LIST_W        40

#define HOSTS_BOX_Y   2
#define HOSTS_Y       3
#define DEVICES_BOX_Y 13
#define DEVICES_Y     14
#define INFO_BOX_Y    1
#define FILES_BOX_Y   6
#define FILES_Y       7
#define FILE_X        1
#define FILE_W        40
#define INFO_X        2
#define INFO_W        38
#define FILTER_X      (INFO_X + 6)
#define FILTER_Y      (INFO_BOX_Y + 2)
#define SLOT_BOX_Y    2
#define SLOT_Y        3
#define DETAIL_BOX_Y  9
#define NETWORKS_BOX_Y 5
#define NETWORKS_Y    6
#define PROMPT_BOX_Y  7
#define PROMPT_FIELD_Y 11

#define ASK_Y         19
#define MENU_BOX_Y    20
#define MENU_X        2
#define MENU_Y1       21
#define MENU_Y2       22
#define STATUS_Y      22

#define BOX_TL 0xA0
#define BOX_TR 0xA1
#define BOX_BL 0xA2
#define BOX_BR 0xA3
#define BOX_RT 0xA4
#define BOX_LT 0xA5
#define BOX_V  0xA8
#define BOX_H  0xA9

extern char text_empty[];
extern unsigned screen_generation;
extern byte colorset;
extern byte redraw_row;
#define COLOR_CSS() (colorset >> 1)
#define COLOR_INVERTED() (!(colorset & 1))
void screen_leave_hires(void);
void color_load(void);
void color_step(int dir);

void screen_handoff(void);
void screen_loading(const char *what);
void screen_title(const char *title);
void screen_frame(const char *title);
void screen_box(byte x, byte y, byte w, byte h, const char *title);
void screen_menu_clear(void);
void screen_prompt(const char *s);
void screen_message_target(byte y, bool center, byte hold);
char *screen_upper(char *s);

void bar_unlit(void);
bool bar_cell_lit(byte x, byte y);

#endif
