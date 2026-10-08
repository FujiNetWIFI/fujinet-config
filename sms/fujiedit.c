/* fujiedit.c -- the on-screen keyboard, laid out like Phantasy Star's
 * name-entry screen: characters on every other column of every other row, a
 * cyan underline under the one the cursor is on, a white one under the place
 * the next character goes. See fujiedit.h.
 */

#include <string.h>

#include "fujidisp.h"
#include "fujiedit.h"
#include "fujiin.h"
#include "fujisnd.h"

#define VALUE_ROW  2
#define VALUE_W    29           /* characters shown; the marker takes col 30 */
#define GRID_COL0  4
#define GRID_ROW0  6
#define GRID_COLS  13
#define GRID_ROWS  5
#define ACT_ROW    16
#define ACT_CELLS  5

/* Sprites: 0-3 the cursor's underline (CASE is four wide), 4 the edit
 * point, 5 the end of the list. */
#define SPR_MARK   4
#define SPR_COUNT  5

char fn_entry[FN_ENTRY_MAX];

/* All 63 printable characters from '!' to '_', letters first; a 0 is an
 * empty cell, which the cursor steps around. CASE folds $40-$5E up by $20,
 * which makes the letters lowercase and @[\]^ into `{|}~ -- so with SPC
 * every printable ASCII character can be typed. */
static const char kb_cells[GRID_ROWS * GRID_COLS + 1] =
    "ABCDEFGHIJKLM"
    "NOPQRSTUVWXYZ"
    "0123456789.-_"
    "!\"#$%&'()*+,/"
    ":;<=>?@[\\]^\0\0";

static const char *const act_text[ACT_CELLS] = { "CASE", "SPC", "RUB", "END", "ESC" };
static const unsigned char act_col[ACT_CELLS] = { 4, 10, 15, 20, 25 };

static unsigned char glen;      /* current length */
static unsigned char gmax;      /* caller's limit */
static unsigned char gcase;     /* 0 upper, 1 lower */
static unsigned char gx, gy;    /* cursor; gy == GRID_ROWS is the action row */

static char cell_char(unsigned char cx, unsigned char cy)
{
    char c = kb_cells[cy * GRID_COLS + cx];

    if (gcase && c >= 0x40 && c <= 0x5E)
        c = (char)(c + 0x20);
    return c;
}

static void draw_grid(void)
{
    unsigned char cx, cy;

    for (cy = 0; cy < GRID_ROWS; cy++)
        for (cx = 0; cx < GRID_COLS; cx++) {
            char c = cell_char(cx, cy);

            disp_char((unsigned char)(GRID_COL0 + 2 * cx),
                      (unsigned char)(GRID_ROW0 + 2 * cy), c ? c : ' ');
        }
}

static void draw_actions(void)
{
    unsigned char i;

    for (i = 0; i < ACT_CELLS; i++)
        disp_at(act_col[i], ACT_ROW, act_text[i]);
}

/* The cursor's underline: one sprite under a grid character, one per letter
 * under an action word. */
static void place_cursor(void)
{
    unsigned char n = 1, col, row, i;

    if (gy == GRID_ROWS) {
        col = act_col[gx];
        row = ACT_ROW;
        n = (unsigned char)strlen(act_text[gx]);
    } else {
        col = (unsigned char)(GRID_COL0 + 2 * gx);
        row = (unsigned char)(GRID_ROW0 + 2 * gy);
    }
    for (i = 0; i < 4; i++)
        if (i < n)
            spr_cell(i, (unsigned char)(col + i), row, T_ULINE);
        else
            spr_cell(i, 0, 24, T_ULINE);    /* parked below the screen */
}

/* The value line: the tail of the buffer, with the white marker under the
 * cell the next character lands in. Editing is append and rub only. */
static void draw_value(void)
{
    unsigned char start = 0;
    unsigned char i;

    if (glen > VALUE_W)
        start = (unsigned char)(glen - VALUE_W);

    disp_row_blank(VALUE_ROW);
    for (i = 0; start + i < glen; i++)
        disp_char((unsigned char)(TEXT_L + i), VALUE_ROW, fn_entry[start + i]);
    spr_cell(SPR_MARK, (unsigned char)(TEXT_L + glen - start), VALUE_ROW, T_ULINE2);
}

static void move_to(unsigned char nx, unsigned char ny)
{
    gx = nx;
    gy = ny;
    place_cursor();
    snd_play(SND_MOVE);
}

static void type_char(char c)
{
    if (glen >= gmax)
        return;                 /* full: silently ignore, as the family does */
    fn_entry[glen++] = c;
    fn_entry[glen] = '\0';
    draw_value();
    snd_play(SND_TYPE);
}

static void backspace(void)
{
    if (glen == 0)
        return;
    fn_entry[--glen] = '\0';
    draw_value();
    snd_play(SND_BACK);
}

/* Down from the grid's last row, or into an empty cell, is the action row,
 * at the action nearest the column. */
static void to_actions(void)
{
    unsigned char a = (unsigned char)(gx * ACT_CELLS / GRID_COLS);

    move_to(a, GRID_ROWS);
}

static bool finish(bool accept)
{
    spr_end(0);
    snd_play(accept ? SND_OK : SND_BACK);
    return accept;
}

bool fn_edit(const char *title, const char *prompt, unsigned char maxlen)
{
    if (maxlen > FN_ENTRY_MAX - 1)
        maxlen = FN_ENTRY_MAX - 1;
    gmax = maxlen;
    fn_entry[maxlen] = '\0';
    glen = (unsigned char)strlen(fn_entry);
    gcase = 0;
    gx = 0;
    gy = 0;

    layout_show();
    win_clear();
    win_title(title);
    disp_row_blank(MSG_LINE1);
    msg_put(MSG_LINE2, "1 TYPE  2 RUB  PAUSE END");
    draw_grid();
    draw_actions();
    spr_end(SPR_COUNT);
    draw_value();
    place_cursor();
    msg_type(MSG_LINE1, prompt);

    for (;;) {
        unsigned char ev = in_read();

        switch (ev) {
        case IN_UP:
            if (gy == GRID_ROWS) {
                /* Back up into the grid near where the word was. */
                unsigned char nx = (unsigned char)(gx * GRID_COLS / ACT_CELLS + 1);

                while (!kb_cells[(GRID_ROWS - 1) * GRID_COLS + nx])
                    nx--;
                move_to(nx, GRID_ROWS - 1);
            } else if (gy > 0) {
                move_to(gx, (unsigned char)(gy - 1));
            }
            break;

        case IN_DOWN:
            if (gy < GRID_ROWS - 1 && kb_cells[(gy + 1) * GRID_COLS + gx])
                move_to(gx, (unsigned char)(gy + 1));
            else if (gy < GRID_ROWS)
                to_actions();
            break;

        case IN_LEFT:
            if (gx > 0)
                move_to((unsigned char)(gx - 1), gy);
            break;

        case IN_RIGHT:
            if (gy == GRID_ROWS) {
                if (gx + 1 < ACT_CELLS)
                    move_to((unsigned char)(gx + 1), gy);
            } else if (gx + 1 < GRID_COLS && kb_cells[gy * GRID_COLS + gx + 1]) {
                move_to((unsigned char)(gx + 1), gy);
            }
            break;

        case IN_FIRE:
            if (gy < GRID_ROWS) {
                type_char(cell_char(gx, gy));
                break;
            }
            switch (gx) {
            case 0:             /* CASE */
                gcase = (unsigned char)(gcase ^ 1);
                draw_grid();
                snd_play(SND_OK);
                break;
            case 1:             /* SPC */
                type_char(' ');
                break;
            case 2:             /* RUB */
                backspace();
                break;
            case 3:             /* END */
                return finish(true);
            default:            /* ESC -- the only way to cancel */
                return finish(false);
            }
            break;

        case IN_KEYSTAR:
            backspace();
            break;

        case IN_MENU:
            return finish(true);

        default:
            break;
        }
    }
}
