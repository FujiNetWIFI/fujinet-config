#include <string.h>

#include "fujidisp.h"

/* MARIA colours: hue << 4 | luminance. A PAL console's hues sit one
 * higher than an NTSC one's for the same colour. */
#define C_BLACK  0x00
#define C_WHITE  0x0F
#define C_FRAME  0x9A           /* light blue */
#define C_BAR    0x84           /* the selection bar's blue */
#define PAL_HUE  0x10

static unsigned char cur_col, cur_row, cur_under;
static bool cur_on;             /* the cursor is placed */
static bool cur_lit;            /* ...and showing its block right now */
static unsigned char blink;     /* frames into the 24-frame blink */

static unsigned char bar_last[DISP_COLS];

static char clean(char c)
{
    return (c < 32 || c > 126) ? ' ' : c;
}

void disp_init(void)
{
    unsigned char shift;

    mt_init(' ');
    shift = mt_pal ? PAL_HUE : 0;
    mt_background(C_BLACK);
    mt_palette(0, C_WHITE, C_BLACK, (unsigned char)(C_FRAME + shift));
    mt_palette(1, C_WHITE, (unsigned char)(C_BAR + shift),
               (unsigned char)(C_FRAME + shift));
}

void disp_cls(void)
{
    cur_on = false;
    mt_clear(' ');
}

void disp_row_clear(unsigned char row)
{
    if (row >= DISP_ROWS)
        return;
    mt_at(IN_L, row);
    mt_fill(' ', IN_W);
    mt_at(IN_L, row);
    mt_setpal(0, IN_W);
}

void disp_at_hi(unsigned char col, unsigned char row, const char *s, bool hi)
{
    unsigned char n;

    if (row >= DISP_ROWS || col > IN_R)
        return;
    mt_at(col, row);
    n = mt_text(s, (unsigned char)(IN_R + 1 - col));
    mt_at(col, row);
    mt_setpal(hi, n);
}

void disp_text(unsigned char col, unsigned char row,
               volatile unsigned char *s, unsigned char max)
{
    unsigned char n;

    if (row >= DISP_ROWS || col > IN_R)
        return;
    if (max > IN_R + 1 - col)
        max = (unsigned char)(IN_R + 1 - col);
    mt_at(col, row);
    n = mt_text((const char *)s, max);
    mt_at(col, row);
    mt_setpal(0, n);
}

void disp_at(unsigned char col, unsigned char row, const char *s)
{
    disp_at_hi(col, row, s, false);
}

void disp_char_hi(unsigned char col, unsigned char row, char c, bool hi)
{
    if (row >= DISP_ROWS || col > IN_R)
        return;
    mt_at(col, row);
    mt_put(clean(c));
    mt_at(col, row);
    mt_setpal(hi, 1);
}

void disp_char(unsigned char col, unsigned char row, char c)
{
    disp_char_hi(col, row, c, false);
}

void disp_tile(unsigned char col, unsigned char row, unsigned char t)
{
    if (row >= DISP_ROWS || col >= DISP_COLS)
        return;
    mt_at(col, row);
    mt_put(t);
    mt_at(col, row);
    mt_setpal(0, 1);
}

void disp_box(unsigned char l, unsigned char t, unsigned char r,
              unsigned char b, const char *title)
{
    unsigned char i;

    mt_at(l, t);
    mt_put(T_TL);
    mt_fill(T_HLINE, (unsigned char)(r - l - 1));
    mt_put(T_TR);
    mt_at(l, b);
    mt_put(T_BL);
    mt_fill(T_HLINE, (unsigned char)(r - l - 1));
    mt_put(T_BR);
    for (i = (unsigned char)(t + 1); i < b; i++) {
        mt_at(l, i);
        mt_put(T_VLINE);
        mt_at(r, i);
        mt_put(T_VLINE);
    }
    if (title) {
        unsigned char len = (unsigned char)strlen(title);
        unsigned char span = (unsigned char)(r - l - 1);
        unsigned char at;

        if (len + 2 > span)
            len = (unsigned char)(span - 2);
        at = (unsigned char)(l + 1 + (span - len - 2) / 2);
        mt_at(at, t);
        mt_put(' ');
        for (i = 0; i < len; i++)
            mt_put(title[i]);
        mt_put(' ');
    }
}

void disp_frame(const char *title)
{
    disp_cls();
    disp_box(FRAME_L, FRAME_T, FRAME_R, FRAME_B, title);
}

void disp_row_invert(unsigned char row, bool on)
{
    if (row >= DISP_ROWS)
        return;
    mt_at(IN_L, row);
    mt_setpal(on, IN_W);
}

void disp_bar_reset(void)
{
    memset(bar_last, 0xFF, sizeof bar_last);
}

void disp_bar(unsigned char col, unsigned char row, unsigned char cells,
              unsigned char num, unsigned char den)
{
    unsigned int px;
    unsigned char i, t;

    if (den == 0)
        den = 1;
    if (num > den)
        num = den;
    px = (unsigned int)num * (unsigned int)(cells * 8u) / den;
    for (i = 0; i < cells && i < DISP_COLS; i++) {
        if (px >= 8) {
            t = T_BAR0 + 8;
            px -= 8;
        } else {
            t = (unsigned char)(T_BAR0 + px);
            px = 0;
        }
        if (bar_last[i] != t) {
            bar_last[i] = t;
            disp_tile((unsigned char)(col + i), row, t);
        }
    }
}

/* What the block covers is read back as it lights, and put back only if
 * the block is still there: a cell redrawn underneath it wins. */
static void cursor_paint(bool lit)
{
    unsigned char here = mt_get(cur_col, cur_row);

    mt_at(cur_col, cur_row);
    if (lit) {
        cur_under = here == T_CURSOR ? ' ' : here;
        mt_put(T_CURSOR);
    } else if (here == T_CURSOR) {
        mt_put(cur_under);
    }
    cur_lit = lit;
}

void disp_cursor_at(unsigned char col, unsigned char row)
{
    if (cur_on)
        cursor_paint(false);
    cur_col = col;
    cur_row = row;
    cur_on = true;
    blink = 0;
    cursor_paint(true);
}

void disp_cursor_tick(void)
{
    bool lit;

    if (!cur_on)
        return;
    /* 12 on, 12 off, counted in calls: the caller ticks once a frame */
    if (++blink >= 24)
        blink = 0;
    lit = (bool)(blink < 12);
    if (lit != cur_lit)
        cursor_paint(lit);
}

void disp_cursor_off(void)
{
    if (cur_on)
        cursor_paint(false);
    cur_on = false;
}

void disp_at_u16(unsigned char col, unsigned char row, unsigned int v)
{
    char buf[6];
    unsigned char i = 5;

    buf[5] = '\0';
    do {
        buf[--i] = (char)('0' + (v % 10u));
        v /= 10u;
    } while (v != 0 && i != 0);
    disp_at(col, row, buf + i);
}

void disp_at_hex8(unsigned char col, unsigned char row, unsigned char v)
{
    static const char hex[] = "0123456789ABCDEF";
    char buf[3];

    buf[0] = hex[(v >> 4) & 0x0F];
    buf[1] = hex[v & 0x0F];
    buf[2] = '\0';
    disp_at(col, row, buf);
}
