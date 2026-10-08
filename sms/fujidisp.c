/* fujidisp.c -- Phantasy Star-style windows on the SMS VDP in Mode 4. */

#include <string.h>

#include <arch/sms.h>

#include "fujidisp.h"
#include "fujiin.h"

#define NAME_TABLE 0x3800
#define SAT        0x3F00

/* build/font_data.c, from font.txt: 1bpp rows and a colour per tile. */
extern const unsigned char font_rows[128 * 8];
extern const unsigned char font_colour[128];

/* What each cell shows: tile, and the name-table high byte. */
static unsigned char scr_ch[DISP_ROWS][DISP_COLS];
static unsigned char scr_at[DISP_ROWS][DISP_COLS];

/* Spaces VRAM writes past the VDP's 29 T-state minimum in active display. */
static volatile unsigned char gap;

static bool ovl;                        /* a pop-up is open: VRAM only */
static unsigned char ovl_x0, ovl_y0, ovl_x1, ovl_y1;
static bool layout_up;

static bool cb_active;                  /* the blinking cursor */
static unsigned char cb_col, cb_row, cb_on, cb_off, cb_phase;

static unsigned char pal[32];
static unsigned char level = 9;

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

static void cell_addr(unsigned char col, unsigned char row)
{
    disp_vram_addr(NAME_TABLE + ((unsigned int)row * DISP_COLS + col) * 2);
}

static void put(unsigned char t, unsigned char a)
{
    IO_VDP_DATA = t;
    gap++;
    IO_VDP_DATA = a;
    gap++;
}

/* One cell at the current VRAM address, which the caller has set to it. */
static void cell(unsigned char col, unsigned char row, unsigned char t, unsigned char a)
{
    if (!ovl) {
        scr_ch[row][col] = t;
        scr_at[row][col] = a;
    }
    put(t, a);
}

static unsigned char text_tile(char c)
{
    unsigned char t = (unsigned char)c;

    return (t < 32 || t > 127) ? '?' : t;
}

void disp_at(unsigned char col, unsigned char row, const char *s)
{
    if (row >= DISP_ROWS || col > TEXT_R)
        return;
    cell_addr(col, row);
    while (*s && col <= TEXT_R) {
        cell(col, row, text_tile(*s++), 0);
        col++;
    }
}

void disp_char(unsigned char col, unsigned char row, char c)
{
    if (row >= DISP_ROWS || col > TEXT_R)
        return;
    cell_addr(col, row);
    cell(col, row, text_tile(c), 0);
}

void disp_tile(unsigned char col, unsigned char row, unsigned char t, unsigned char a)
{
    if (row >= DISP_ROWS || col >= DISP_COLS)
        return;
    cell_addr(col, row);
    cell(col, row, t, a);
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

void disp_row_blank(unsigned char row)
{
    unsigned char col;

    cell_addr(TEXT_L, row);
    for (col = TEXT_L; col <= TEXT_R; col++)
        cell(col, row, ' ', 0);
}

void disp_row_attr(unsigned char row, unsigned char set, unsigned char clr)
{
    unsigned char col;

    cell_addr(TEXT_L, row);
    for (col = TEXT_L; col <= TEXT_R; col++)
        cell(col, row, scr_ch[row][col],
             (unsigned char)((scr_at[row][col] & ~clr) | set));
}

void disp_row_vram(unsigned char row, const volatile unsigned char *s)
{
    unsigned char col;

    cell_addr(TEXT_L, row);
    for (col = TEXT_L; col <= TEXT_R; col++)
        put(text_tile((char)*s++), scr_at[row][col]);
}

/* ---- windows ------------------------------------------------------------ */

static void box_row(unsigned char x0, unsigned char x1, unsigned char y,
                    unsigned char y0, unsigned char y1)
{
    unsigned char lt, mt, rt, la, ma, ra, x;

    if (y == y0) {
        lt = T_CORNER; mt = T_HEDGE; rt = T_CORNER;
        la = 0;        ma = 0;       ra = A_HFLIP;
    } else if (y == y1) {
        lt = T_CORNER; mt = T_HEDGE; rt = T_CORNER;
        la = A_VFLIP;  ma = A_VFLIP; ra = A_HFLIP | A_VFLIP;
    } else {
        lt = T_VEDGE;  mt = ' ';     rt = T_VEDGE;
        la = 0;        ma = 0;       ra = A_HFLIP;
    }
    cell_addr(x0, y);
    cell(x0, y, lt, la);
    for (x = (unsigned char)(x0 + 1); x < x1; x++)
        cell(x, y, mt, ma);
    cell(x1, y, rt, ra);
}

void win_box(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, bool wipe)
{
    unsigned char y;

    for (y = y0; y <= y1; y++) {
        box_row(x0, x1, y, y0, y1);
        if (wipe)
            in_wait(1);
    }
}

void layout_show(void)
{
    if (layout_up)
        return;
    win_box(0, MAIN_TOP, DISP_COLS - 1, MAIN_BOT, true);
    win_box(0, MSG_TOP, DISP_COLS - 1, MSG_BOT, true);
    layout_up = true;
}

void layout_drop(void)
{
    layout_up = false;
}

/* The title sits in the top border with a blank either side, so the line
 * breaks around it. 26 characters at most: columns 3-28. */
void win_title(const char *s)
{
    unsigned char n = (unsigned char)strlen(s);
    unsigned char i;

    box_row(0, DISP_COLS - 1, MAIN_TOP, MAIN_TOP, MAIN_BOT);
    if (n == 0)
        return;
    if (n > 26)
        n = 26;
    cell_addr(2, MAIN_TOP);
    cell(2, MAIN_TOP, ' ', 0);
    for (i = 0; i < n; i++)
        cell((unsigned char)(3 + i), MAIN_TOP, text_tile(s[i]), 0);
    cell((unsigned char)(3 + n), MAIN_TOP, ' ', 0);
}

void win_clear(void)
{
    unsigned char row;

    cur_hide();
    for (row = MAIN_TOP + 1; row < MAIN_BOT; row++)
        disp_row_blank(row);
}

void msg_put(unsigned char row, const char *s)
{
    disp_row_blank(row);
    disp_at(TEXT_L, row, s);
}

void msg_type(unsigned char row, const char *s)
{
    unsigned char col = TEXT_L;

    disp_row_blank(row);
    while (*s && col <= TEXT_R) {
        disp_char(col++, row, *s++);
        in_wait(1);
    }
}

/* ---- pop-ups ------------------------------------------------------------ */

void ovl_open(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1)
{
    ovl = true;
    ovl_x0 = x0;
    ovl_y0 = y0;
    ovl_x1 = x1;
    ovl_y1 = y1;
    win_box(x0, y0, x1, y1, true);
}

void ovl_close(void)
{
    unsigned char y, x;

    if (cb_active && cb_col >= ovl_x0 && cb_col <= ovl_x1 &&
        cb_row >= ovl_y0 && cb_row <= ovl_y1)
        cb_active = false;      /* its cell is about to be restored */
    for (y = ovl_y0; y <= ovl_y1; y++) {
        cell_addr(ovl_x0, y);
        for (x = ovl_x0; x <= ovl_x1; x++)
            put(scr_ch[y][x], scr_at[y][x]);
        in_wait(1);
    }
    ovl = false;
}

/* ---- the blinking cursor ------------------------------------------------ */

static void cb_put(unsigned char t)
{
    cell_addr(cb_col, cb_row);
    put(t, 0);
}

void cur_show(unsigned char col, unsigned char row, unsigned char on_t, unsigned char off_t)
{
    if (cb_active && (cb_col != col || cb_row != row))
        cb_put(cb_on);
    cb_col = col;
    cb_row = row;
    cb_on = on_t;
    cb_off = off_t;
    cb_phase = 0;
    cb_active = true;
    cb_put(on_t);
}

void cur_hide(void)
{
    if (!cb_active)
        return;
    cb_put(cb_off);
    cb_active = false;
}

void cur_steady(void)
{
    if (!cb_active)
        return;
    cb_phase = 0;
    cb_put(cb_on);
}

void cur_tick(void)
{
    if (!cb_active)
        return;
    cb_phase++;
    if ((cb_phase & 7) == 0)
        cb_put((cb_phase & 8) ? cb_off : cb_on);
}

/* ---- sprites ------------------------------------------------------------ */

/* A sprite shows from the line after its Y, so 8 x row + 7 puts it on the
 * first line under the glyph. */
void spr_cell(unsigned char n, unsigned char col, unsigned char row, unsigned char t)
{
    disp_vram_addr(SAT + n);
    IO_VDP_DATA = (unsigned char)(row * 8 + 7);
    disp_vram_addr(SAT + 0x80 + n * 2);
    IO_VDP_DATA = (unsigned char)(col * 8);
    gap++;
    IO_VDP_DATA = t;
}

void spr_end(unsigned char n)
{
    disp_vram_addr(SAT + n);
    IO_VDP_DATA = 0xD0;
}

/* ---- palette and fades -------------------------------------------------- */

/* Colour bytes are --BBGGRR. At level l red is capped at l-6, green at l-3
 * and blue at l, so stepping down takes red away first, then green, then
 * blue -- nine steps from full colour to black. */
static unsigned char faded(unsigned char v, unsigned char l)
{
    unsigned char r = (unsigned char)(v & 3);
    unsigned char g = (unsigned char)((v >> 2) & 3);
    unsigned char b = (unsigned char)((v >> 4) & 3);
    unsigned char m;

    m = (unsigned char)(l > 6 ? l - 6 : 0);
    if (r > m)
        r = m;
    m = (unsigned char)(l > 3 ? l - 3 : 0);
    if (g > m)
        g = m;
    if (b > l)
        b = l;
    return (unsigned char)(r | (g << 2) | (b << 4));
}

void pal_set(unsigned char index, unsigned char value)
{
    pal[index] = value;
    IO_VDP_CONTROL = index;
    IO_VDP_CONTROL = 0xC0;
    IO_VDP_DATA = faded(value, level);
}

void pal_level(unsigned char l)
{
    unsigned char i;

    level = l;
    IO_VDP_CONTROL = 0;
    IO_VDP_CONTROL = 0xC0;
    for (i = 0; i < 32; i++)
        IO_VDP_DATA = faded(pal[i], l);
}

void fade_out(void)
{
    while (level) {
        in_wait(4);
        pal_level((unsigned char)(level - 1));
    }
}

void fade_in(void)
{
    while (level < 9) {
        in_wait(4);
        pal_level((unsigned char)(level + 1));
    }
}

/* ---- setup -------------------------------------------------------------- */

/* Every tile is one colour: each 1bpp row goes into the planes its colour
 * index selects. Display off, so no write spacing. */
static void load_cfg_tiles(void)
{
    const unsigned char *src = font_rows;
    unsigned char t, r, ci, b;

    disp_vram_addr(0);
    for (t = 0; t < 128; t++) {
        ci = font_colour[t];
        for (r = 0; r < 8; r++) {
            b = *src++;
            IO_VDP_DATA = (ci & 1) ? b : 0;
            IO_VDP_DATA = (ci & 2) ? b : 0;
            IO_VDP_DATA = (ci & 4) ? b : 0;
            IO_VDP_DATA = (ci & 8) ? b : 0;
        }
    }
}

void disp_init(void)
{
    unsigned char row, col;

    disp_vdp_reg(1, 0x80);              /* display off while VRAM fills */
    disp_vdp_reg(7, 0x00);              /* border: sprite colour 0 */
    load_cfg_tiles();
    disp_vram_addr(SAT);
    IO_VDP_DATA = 0xD0;                 /* no sprites */

    ovl = false;
    cb_active = false;
    layout_up = false;
    for (row = 0; row < DISP_ROWS; row++) {
        cell_addr(0, row);
        for (col = 0; col < DISP_COLS; col++)
            cell(col, row, ' ', 0);
    }

    pal[0] = 0x00;                      /* black */
    pal[1] = 0x3F;                      /* white */
    pal[2] = 0x0F;                      /* yellow */
    pal[16] = 0x00;                     /* the sprite palette mirrors it, */
    pal[17] = 0x3F;                     /* so A_SEL cells look the same */
    pal[18] = 0x0F;
    pal[19] = 0x3C;                     /* cyan: the keyboard's cursor */
    pal_level(9);
    disp_vdp_reg(1, 0xC0);              /* display on, frame interrupts off */
}
