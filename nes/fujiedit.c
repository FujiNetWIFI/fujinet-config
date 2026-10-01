#include <string.h>

#include "fujidisp.h"
#include "fujiedit.h"
#include "fujiin.h"

#define TITLE_ROW  1
#define VALUE_ROW  4
#define VALUE_COL  1
#define VALUE_W    30
#define GRID_COL0  8
#define GRID_ROW0  8
#define GRID_PITCH 2
#define GRID_COLS  16
#define GRID_ROWS  4
#define ACT_ROW    17
#define ACT_CELLS  5
#define FOOT_ROW   23

char fn_entry[FN_ENTRY_MAX];

static unsigned char glen;
static unsigned char gmax;
static unsigned char gcase;
static unsigned char gx, gy;

static const char *act_text[ACT_CELLS] = { "CASE", "SPC", "DEL", "OK", "ESC" };
static const unsigned char act_col[ACT_CELLS] = { 1, 8, 13, 19, 24 };

static char cell_char(unsigned char cx, unsigned char cy)
{
    char c = (char)(0x20 + cy * GRID_COLS + cx);

    if (gcase && c >= 'A' && c <= 'Z')
        c = (char)(c + 0x20);
    return c;
}

static unsigned char grid_screen_row(unsigned char cy)
{
    return (unsigned char)(GRID_ROW0 + cy * GRID_PITCH);
}

static void draw_cell(unsigned char cx, unsigned char cy, bool sel)
{
    disp_char_hi((unsigned char)(GRID_COL0 + cx), grid_screen_row(cy),
                 cell_char(cx, cy), sel);
}

static void draw_grid(void)
{
    unsigned char cx, cy;

    for (cy = 0; cy < GRID_ROWS; cy++)
        for (cx = 0; cx < GRID_COLS; cx++)
            draw_cell(cx, cy, (bool)(gy == cy && gx == cx));
}

static void draw_actions(void)
{
    unsigned char i;

    for (i = 0; i < ACT_CELLS; i++)
        disp_at_hi(act_col[i], ACT_ROW, act_text[i],
                   (bool)(gy == GRID_ROWS && gx == i));
}

static void draw_value(void)
{
    unsigned char start = 0;
    unsigned char i;

    if (glen > VALUE_W - 1)
        start = (unsigned char)(glen - (VALUE_W - 1));

    disp_row_clear(VALUE_ROW);
    for (i = 0; i < VALUE_W; i++) {
        unsigned char idx = (unsigned char)(start + i);
        bool cursor = (bool)(idx == glen);

        disp_char_hi((unsigned char)(VALUE_COL + i), VALUE_ROW,
                     idx < glen ? fn_entry[idx] : ' ', cursor);
    }
}

static void move_to(unsigned char nx, unsigned char ny)
{
    if (gy == GRID_ROWS)
        disp_at_hi(act_col[gx], ACT_ROW, act_text[gx], false);
    else
        draw_cell(gx, gy, false);

    gx = nx;
    gy = ny;

    if (gy == GRID_ROWS)
        disp_at_hi(act_col[gx], ACT_ROW, act_text[gx], true);
    else
        draw_cell(gx, gy, true);
}

static void type_char(char c)
{
    if (glen >= gmax)
        return;
    fn_entry[glen++] = c;
    fn_entry[glen] = '\0';
    draw_value();
}

static void backspace(void)
{
    if (glen == 0)
        return;
    fn_entry[--glen] = '\0';
    draw_value();
}

bool fn_edit(const char *title, unsigned char maxlen)
{
    if (maxlen > FN_ENTRY_MAX - 1)
        maxlen = FN_ENTRY_MAX - 1;
    gmax = maxlen;
    fn_entry[maxlen] = '\0';
    glen = (unsigned char)strlen(fn_entry);
    gcase = 0;
    gx = 0;
    gy = 2;

    disp_cls();
    disp_at(1, TITLE_ROW, title);
    disp_at(1, FOOT_ROW, "A PICK  B DEL  SEL CASE  START OK");
    draw_value();
    draw_grid();
    draw_actions();

    for (;;) {
        unsigned char ev = in_read();

        switch (ev) {
        case IN_NONE:
            break;

        case IN_UP:
            if (gy == GRID_ROWS)
                move_to((unsigned char)(gx * 3 + 1), GRID_ROWS - 1);
            else if (gy > 0)
                move_to(gx, (unsigned char)(gy - 1));
            break;

        case IN_DOWN:
            if (gy < GRID_ROWS - 1)
                move_to(gx, (unsigned char)(gy + 1));
            else if (gy == GRID_ROWS - 1)
                move_to((unsigned char)(gx * ACT_CELLS / GRID_COLS), GRID_ROWS);
            break;

        case IN_LEFT:
            if (gx > 0)
                move_to((unsigned char)(gx - 1), gy);
            break;

        case IN_RIGHT:
            if (gy == GRID_ROWS) {
                if (gx + 1 < ACT_CELLS)
                    move_to((unsigned char)(gx + 1), gy);
            } else if (gx + 1 < GRID_COLS) {
                move_to((unsigned char)(gx + 1), gy);
            }
            break;

        case IN_FIRE:
            if (gy < GRID_ROWS) {
                type_char(cell_char(gx, gy));
                break;
            }
            switch (gx) {
            case 0:
                gcase = (unsigned char)(gcase ^ 1);
                draw_grid();
                break;
            case 1:
                type_char(' ');
                break;
            case 2:
                backspace();
                break;
            case 3:
                return true;
            default:            /* ESC -- the only way to cancel */
                return false;
            }
            break;

        case IN_BACK:
            backspace();
            break;

        case IN_SELECT:
            gcase = (unsigned char)(gcase ^ 1);
            draw_grid();
            break;

        case IN_START:
            return true;
        }
    }
}
