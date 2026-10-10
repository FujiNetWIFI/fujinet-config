#ifndef COCO_SCREEN_H
#define COCO_SCREEN_H

#include <cmoc.h>
#include <coco.h>
#include "../screen.h"
#include "coco_text.h"

#define LIST_X        1
#define LIST_W        (SCREEN_COLS - 2)

#define HOSTS_BOX_Y   2
#define HOSTS_Y       3
#define DEVICES_BOX_Y 13
#define DEVICES_Y     14
#define INFO_BOX_Y    1
#define FILES_BOX_Y   6
#define FILES_Y       7
#define FILE_X        1
#define FILE_W        (SCREEN_COLS - 2)
#define INFO_X        2
#define INFO_W        (SCREEN_COLS - 4)
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
extern byte redraw_row;
void screen_leave_graphics(void);
void color_load(void);
bool color_step(int dir);
#ifdef COCO3
extern bool monitor_unset;
void monitor_set(byte cmp);
void monitor_ask(void);
#endif

void screen_handoff(void);
void screen_loading(const char *what);
void screen_title(const char *title);
void screen_frame(const char *title);
void screen_box(byte x, byte y, byte w, byte h, const char *title);
void screen_put_role(byte x, byte y, byte c, byte role);
void screen_puts_role(byte x, byte y, const char *s, byte role);
void screen_move(byte x, byte y);
void screen_menu_clear(void);
void screen_prompt(const char *s);
void screen_message_target(byte y, bool center, byte hold);
char *screen_upper(char *s);
bool screen_select_file_row_is_dir(byte y);

void bar_unlit(void);
bool bar_cell_lit(byte x, byte y);

#endif
