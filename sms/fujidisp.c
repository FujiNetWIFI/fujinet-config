/* fujidisp.c -- 32x24 text on the SMS VDP in Mode 4. */

#include <arch/sms.h>

#include "fujidisp.h"

#define NAME_TABLE 0x3800

/* What each cell shows: character, and colour/highlight bits. */
static unsigned char scr_ch[DISP_ROWS][DISP_COLS];
static unsigned char scr_at[DISP_ROWS][DISP_COLS];

/* Spaces VRAM writes past the VDP's 29 T-state minimum in active display. */
static volatile unsigned char gap;

#define AT_HI   0x80                    /* shadow only: drawn highlighted */

void disp_vram_addr(unsigned int a)
{
    IO_VDP_CONTROL = (unsigned char)a;
    IO_VDP_CONTROL = (unsigned char)(a >> 8) | 0x40;
}

void disp_vdp_reg(unsigned char r, unsigned char v)
{
    IO_VDP_CONTROL = v;
    IO_VDP_CONTROL = (unsigned char)(0x80 | r);
}

void disp_cram(unsigned char index, unsigned char value)
{
    IO_VDP_CONTROL = index;
    IO_VDP_CONTROL = 0xC0;
    IO_VDP_DATA = value;
}

/* Name-table word for a cell: tile = char + 128 x colour, palette 1 if hi. */
static void put_cell(unsigned char c, unsigned char at)
{
    unsigned char color = (unsigned char)(at & 3);
    unsigned char lo = c, hi = 0;

    if (color == DISP_MAGENTA)
        lo = (unsigned char)(c + 128);
    else if (color == DISP_BLUE)
        hi = 0x01;
    if (at & AT_HI)
        hi |= 0x08;
    IO_VDP_DATA = lo;
    gap++;
    IO_VDP_DATA = hi;
    gap++;
}

static void cell(unsigned char col, unsigned char row, unsigned char c, unsigned char at)
{
    if (c < 32 || c > 127)
        c = '?';
    scr_ch[row][col] = c;
    scr_at[row][col] = at;
    put_cell(c, at);
}

static void at_n(unsigned char col, unsigned char row, const char *s, unsigned char at)
{
    if (row >= DISP_ROWS)
        return;
    disp_vram_addr(NAME_TABLE + ((unsigned int)row * DISP_COLS + col) * 2);
    while (*s && col < DISP_COLS)
        cell(col++, row, (unsigned char)*s++, at);
}

void disp_at(unsigned char col, unsigned char row, const char *s)
{
    at_n(col, row, s, DISP_WHITE);
}

void disp_at_color(unsigned char col, unsigned char row, const char *s, unsigned char color)
{
    at_n(col, row, s, color);
}

void disp_at_hi(unsigned char col, unsigned char row, const char *s, bool hi)
{
    at_n(col, row, s, hi ? AT_HI : 0);
}

void disp_char(unsigned char col, unsigned char row, char c)
{
    disp_char_hi(col, row, c, false);
}

void disp_char_hi(unsigned char col, unsigned char row, char c, bool hi)
{
    if (col >= DISP_COLS || row >= DISP_ROWS)
        return;
    disp_vram_addr(NAME_TABLE + ((unsigned int)row * DISP_COLS + col) * 2);
    cell(col, row, (unsigned char)c, hi ? AT_HI : 0);
}

void disp_row_clear(unsigned char row)
{
    unsigned char i;

    disp_vram_addr(NAME_TABLE + (unsigned int)row * DISP_COLS * 2);
    for (i = 0; i < DISP_COLS; i++)
        cell(i, row, ' ', 0);
}

void disp_row_invert(unsigned char row, bool on)
{
    unsigned char i;

    disp_vram_addr(NAME_TABLE + (unsigned int)row * DISP_COLS * 2);
    for (i = 0; i < DISP_COLS; i++) {
        unsigned char at = scr_at[row][i];

        at = on ? (unsigned char)(at | AT_HI) : (unsigned char)(at & ~AT_HI);
        cell(i, row, scr_ch[row][i], at);
    }
}

void disp_cls(void)
{
    unsigned char row;

    for (row = 0; row < DISP_ROWS; row++)
        disp_row_clear(row);
}

void disp_at_u16(unsigned char col, unsigned char row, unsigned int v)
{
    char buf[6];
    unsigned char i = 5;

    buf[5] = '\0';
    do {
        buf[--i] = (char)('0' + v % 10);
        v /= 10;
    } while (v && i);
    disp_at(col, row, buf + i);
}

void disp_at_hex8(unsigned char col, unsigned char row, unsigned char v)
{
    static const char hex[] = "0123456789ABCDEF";
    char buf[3];

    buf[0] = hex[v >> 4];
    buf[1] = hex[v & 15];
    buf[2] = '\0';
    disp_at(col, row, buf);
}

/* One copy of the font in colour index `ci`: 1bpp rows into the planes ci
 * selects. */
static void load_font(unsigned int first_tile, unsigned char ci)
{
    const unsigned char *src = standard_font + 32 * 8;
    unsigned int n;

    disp_vram_addr(first_tile * 32);
    for (n = 0; n < 96 * 8; n++) {
        unsigned char r = src[n];

        IO_VDP_DATA = (ci & 1) ? r : 0;
        IO_VDP_DATA = (ci & 2) ? r : 0;
        IO_VDP_DATA = (ci & 4) ? r : 0;
        IO_VDP_DATA = (ci & 8) ? r : 0;
    }
}

void disp_init(void)
{
    disp_vdp_reg(1, 0x80);              /* display off while VRAM fills */
    disp_vdp_reg(7, 0x00);              /* backdrop: colour 0 */
    load_font(32, 1);
    load_font(160, 2);
    load_font(288, 3);
    disp_cram(0, 0x00);                 /* black */
    disp_cram(1, 0x3F);                 /* white */
    disp_cram(2, 0x33);                 /* magenta, the BIOS's ENJOY!!! */
    disp_cram(3, 0x34);                 /* SEGA blue */
    disp_cram(16, 0x3F);                /* highlight: white field... */
    disp_cram(17, 0x00);                /* ...black text */
    disp_cram(18, 0x33);
    disp_cram(19, 0x34);
    disp_cls();
    disp_vdp_reg(1, 0xC0);              /* display on, frame interrupts off */
}
