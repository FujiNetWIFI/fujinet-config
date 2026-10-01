#include <conio.h>
#include <string.h>

#include "fujidisp.h"

/* The shadow: what each cell shows, without the reverse bit. In WRAM, where
 * cc65 keeps BSS on this target; the console's own 2K holds the stacks and
 * the conio buffer. */
static char shadow[DISP_ROWS][DISP_COLS];

static char clean(char c)
{
    return (c < 32 || c > 126) ? ' ' : c;
}

void disp_init(void)
{
    revers(0);
    disp_cls();
}

void disp_cls(void)
{
    clrscr();
    memset(shadow, ' ', sizeof shadow);
}

void disp_row_clear(unsigned char row)
{
    if (row >= DISP_ROWS)
        return;
    revers(0);
    cclearxy(0, row, DISP_COLS);
    memset(shadow[row], ' ', DISP_COLS);
}

void disp_at_hi(unsigned char col, unsigned char row, const char *s, bool hi)
{
    if (row >= DISP_ROWS || col >= DISP_COLS)
        return;
    revers(hi ? 1 : 0);
    gotoxy(col, row);
    while (*s && col < DISP_COLS) {
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
    if (row >= DISP_ROWS || col >= DISP_COLS)
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

void disp_row_invert(unsigned char row, bool on)
{
    unsigned char i;

    if (row >= DISP_ROWS)
        return;
    revers(on ? 1 : 0);
    gotoxy(0, row);
    for (i = 0; i < DISP_COLS; i++)
        cputc(shadow[row][i]);
    revers(0);
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
