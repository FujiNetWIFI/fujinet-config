#include <conio.h>
#include <string.h>

#include "fujidisp.h"

/* The palette, Family BASIC's: black ground, white text, light blue frame
 * (and a dark blue no screen uses yet). Every attribute is 0, so this one
 * background palette is the whole screen. */
static const unsigned char palette[4] = { 0x0F, 0x30, 0x21, 0x02 };

void __fastcall__ disp_pal_set(unsigned char idx, unsigned char val);

/* The shadow: what each cell shows, without the reverse bit. In WRAM, where
 * cc65 keeps BSS on this target; the console's own 2K holds the stacks and
 * the conio buffer. */
static char shadow[DISP_ROWS][DISP_COLS];

static unsigned char cur_col, cur_row;
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
    unsigned char i;

    for (i = 0; i < 4; i++)
        disp_pal_set(i, palette[i]);
    revers(0);
    disp_cls();
}

void disp_cls(void)
{
    cur_on = false;
    clrscr();
    memset(shadow, ' ', sizeof shadow);
}

void disp_row_clear(unsigned char row)
{
    if (row >= DISP_ROWS)
        return;
    revers(0);
    cclearxy(IN_L, row, IN_W);
    memset(&shadow[row][IN_L], ' ', IN_W);
}

void disp_at_hi(unsigned char col, unsigned char row, const char *s, bool hi)
{
    if (row >= DISP_ROWS || col > IN_R)
        return;
    revers(hi ? 1 : 0);
    gotoxy(col, row);
    while (*s && col <= IN_R) {
        char c = clean(*s++);

        cputc(c);
        shadow[row][col++] = c;
    }
    revers(0);
}

void disp_at(unsigned char col, unsigned char row, const char *s)
{
    disp_at_hi(col, row, s, false);
}

void disp_char_hi(unsigned char col, unsigned char row, char c, bool hi)
{
    if (row >= DISP_ROWS || col > IN_R)
        return;
    c = clean(c);
    revers(hi ? 1 : 0);
    cputcxy(col, row, c);
    revers(0);
    shadow[row][col] = c;
}

void disp_char(unsigned char col, unsigned char row, char c)
{
    disp_char_hi(col, row, c, false);
}

void disp_tile(unsigned char col, unsigned char row, unsigned char t)
{
    if (row >= DISP_ROWS || col >= DISP_COLS)
        return;
    revers(0);
    cputcxy(col, row, (char)t);
    shadow[row][col] = (char)t;
}

void disp_box(unsigned char l, unsigned char t, unsigned char r,
              unsigned char b, const char *title)
{
    unsigned char i;

    disp_tile(l, t, T_TL);
    disp_tile(r, t, T_TR);
    disp_tile(l, b, T_BL);
    disp_tile(r, b, T_BR);
    for (i = (unsigned char)(l + 1); i < r; i++) {
        disp_tile(i, t, T_HLINE);
        disp_tile(i, b, T_HLINE);
    }
    for (i = (unsigned char)(t + 1); i < b; i++) {
        disp_tile(l, i, T_VLINE);
        disp_tile(r, i, T_VLINE);
    }
    if (title) {
        unsigned char len = (unsigned char)strlen(title);
        unsigned char span = (unsigned char)(r - l - 1);
        unsigned char at;

        if (len + 2 > span)
            len = (unsigned char)(span - 2);
        at = (unsigned char)(l + 1 + (span - len - 2) / 2);
        disp_char(at, t, ' ');
        for (i = 0; i < len; i++)
            disp_char((unsigned char)(at + 1 + i), t, title[i]);
        disp_char((unsigned char)(at + 1 + len), t, ' ');
    }
}

void disp_frame(const char *title)
{
    disp_cls();
    disp_box(FRAME_L, FRAME_T, FRAME_R, FRAME_B, title);
}

void disp_row_invert(unsigned char row, bool on)
{
    unsigned char i;

    if (row >= DISP_ROWS)
        return;
    revers(on ? 1 : 0);
    gotoxy(IN_L, row);
    for (i = IN_L; i <= IN_R; i++)
        cputc(shadow[row][i]);
    revers(0);
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

static void cursor_paint(bool lit)
{
    revers(0);
    cputcxy(cur_col, cur_row, lit ? (char)T_CURSOR : shadow[cur_row][cur_col]);
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
