#include "antic.h"

#define REG(a)   (*(volatile unsigned char *)(a))
#define SDLSTL   REG(0x05)
#define SDLSTH   REG(0x06)
#define SDMCTL   REG(0x07)
#define COLOR1   REG(0x0D)      /* mode 2: the text's luminance */
#define COLOR2   REG(0x0E)      /* mode 2: the background */
#define COLOR4   REG(0x10)      /* the border */
#define CHACTL   REG(0xD401)
#define CHBASE   REG(0xD409)

/* The top 1K of RAM, which the Makefile reserves: 1K-aligned, so the
 * screen never crosses the 4K boundary ANTIC's address counter wraps at. */
#define SCREEN   ((unsigned char *)0x3C00)

extern const unsigned char mt_dlist[];  /* antic_dl.s */
extern const unsigned char mt_font[];

static unsigned char *wp;

/* ASCII -> ANTIC internal code. */
static unsigned char code(unsigned char t)
{
    t &= 0x7F;
    if (t < 0x20)
        return (unsigned char)(t + 0x40);
    if (t < 0x60)
        return (unsigned char)(t - 0x20);
    return t;
}

static unsigned char ascii(unsigned char c)
{
    c &= 0x7F;
    if (c < 0x40)
        return (unsigned char)(c + 0x20);
    if (c < 0x60)
        return (unsigned char)(c - 0x40);
    return c;
}

void mt_clear(unsigned char fill)
{
    unsigned char *p = SCREEN;
    unsigned int n = MT_COLS * MT_ROWS;
    unsigned char c = code(fill);

    while (n--)
        *p++ = c;
    wp = SCREEN;
}

void mt_colours(unsigned char background, unsigned char text)
{
    COLOR2 = background;
    COLOR4 = background;
    COLOR1 = text;
}

void mt_init(unsigned char fill)
{
    mt_clear(fill);
    CHBASE = (unsigned char)((unsigned int)mt_font >> 8);
    CHACTL = 2;                 /* code + $80 is reverse video */
    SDLSTL = (unsigned char)(unsigned int)mt_dlist;
    SDLSTH = (unsigned char)((unsigned int)mt_dlist >> 8);
    SDMCTL = 0x22;              /* normal playfield, display-list DMA */
}

void mt_at(unsigned char col, unsigned char row)
{
    wp = SCREEN + (unsigned int)row * MT_COLS + col;
}

void mt_put(unsigned char t)
{
    *wp++ = code(t);
}

void mt_fill(unsigned char t, unsigned char n)
{
    unsigned char c = code(t);

    while (n--)
        *wp++ = c;
}

unsigned char mt_text(const char *s, unsigned char max)
{
    unsigned char n = 0;

    while (n < max && s[n]) {
        unsigned char t = (unsigned char)s[n];

        *wp++ = code(t < 32 || t > 126 ? ' ' : t);
        n++;
    }
    return n;
}

unsigned char mt_get(unsigned char col, unsigned char row)
{
    return ascii(SCREEN[(unsigned int)row * MT_COLS + col]);
}

void mt_setpal(unsigned char pal, unsigned char n)
{
    unsigned char *p = wp;

    while (n--) {
        if (pal)
            *p++ |= 0x80;
        else
            *p++ &= 0x7F;
    }
}

void mt_sync(void)
{
    unsigned char f = mt_frames;

    while (mt_frames == f)
        ;
}
