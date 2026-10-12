#ifdef BUILD_S100
/**
 * Console HAL and card info for the S-100 card.
 */

#include <string.h>
#ifndef S100_ROM
#include <cpm.h>
#endif
#include "s100.h"

#define PIPE_GETC    0
#define PIPE_STATUS  1
#define PIPE_CONTROL 3 /* write; a read is the 1 kHz TICK */
#define ST_AVAIL     0x80
#define CTL_FLUSH    0x08
#define CTL_LOAD     0x20

/* The card rewrites the five bytes after "FNS1" while it streams this image.
   No other copy of that magic may appear in the binary. */
const uint8_t s100_info[9] = {'F', 'N', 'S', '1', 0xA0, 0, CON_2SIO, 0, 0};

S100Info s100;

#ifdef S100_ROM
/* status, data, RX ready mask and xor, TX ready mask and xor */
static const uint8_t con_ports[3][6] = {
    {0x10, 0x11, 0x01, 0x00, 0x02, 0x00}, /* 6850: RDRF, TDRE */
    {0x00, 0x01, 0x01, 0x01, 0x80, 0x80}, /* 88-SIO: both active low */
    {0x03, 0x02, 0x02, 0x00, 0x01, 0x00}, /* 8251: RxRDY, TxRDY */
};
/* 8251 back to command state, internal reset, 8N1 x16, RTS DTR RxE TxEN */
static const uint8_t init_8251[] = {0x00, 0x00, 0x00, 0x40, 0x4E, 0x37};
static uint8_t cp[6];
#endif /* S100_ROM */

static uint8_t tick(void)
{
    return s100_in(s100.base + PIPE_CONTROL);
}

static bool is_card(uint8_t b)
{
    return (s100_in(b + PIPE_STATUS) & 0x7E) == 0x2C;
}

/* The DIP default first, then every base but the FDC and a 2SIO console */
static uint8_t find_base(void)
{
    uint8_t b = 0xA0;

    if (is_card(b))
        return b;
    b = 0;
    do
    {
        if (b != 0x08 && b != 0x10 && is_card(b))
            return b;
        b += 4;
    } while (b);
    return 0xA0;
}

/* A LOAD header carries the card info: ask for image 7, keep the header, flush the rest */
static void card_query(void)
{
    uint8_t hdr[10], i = 0, last;

    s100_out(s100.base + PIPE_CONTROL, CTL_LOAD | 7);
    last = tick();
    while (i < sizeof(hdr) && (uint8_t)(tick() - last) < 200)
        if (s100_in(s100.base + PIPE_STATUS) & ST_AVAIL)
            hdr[i++] = s100_in(s100.base + PIPE_GETC);
    s100_out(s100.base + PIPE_CONTROL, CTL_FLUSH);
    if (i == sizeof(hdr))
        memcpy(&s100, &hdr[6], 4);
}

void con_init(void)
{
#ifdef S100_ROM
    uint8_t i;
#endif

    memcpy(&s100, &s100_info[4], sizeof(s100));
    if (!s100.page)
    {
        s100.base = find_base();
        card_query();
    }

#ifdef S100_ROM
    i = s100.console > CON_8251 ? CON_2SIO : s100.console;
    memcpy(cp, con_ports[i], sizeof(cp));
    if (i == CON_2SIO)
    {
        s100_out(0x10, 0x03); /* master reset, then 8N1 /16 */
        s100_out(0x10, 0x15);
    }
    else if (i == CON_8251)
        for (i = 0; i < sizeof(init_8251); i++)
            s100_out(0x03, init_8251[i]);
#endif /* S100_ROM */
}

void con_putc(char c)
{
#ifdef S100_ROM
    while (!((s100_in(cp[0]) ^ cp[5]) & cp[4]))
        ;
    s100_out(cp[1], c);
#else
    bdos(CPM_DCIO, (uint8_t)c);
#endif
}

void con_print(const char *s)
{
    while (*s)
        con_putc(*s++);
}

/* 0 when no key is waiting */
uint8_t con_getc(void)
{
#ifdef S100_ROM
    if (!((s100_in(cp[0]) ^ cp[3]) & cp[2]))
        return 0;
    return s100_in(cp[1]) & 0x7F;
#else
    return bdos(CPM_DCIO, 0xFF) & 0x7F;
#endif
}

uint8_t con_getc_wait(uint8_t ms)
{
    uint8_t c, last = tick();

    while (!(c = con_getc()) && (uint8_t)(tick() - last) < ms)
        ;
    return c;
}

void con_wait_ms(uint16_t ms)
{
    uint8_t now, d, last = tick();

    while (ms)
    {
        now = tick();
        d = now - last;
        last = now;
        if (d >= ms)
            break;
        ms -= d;
    }
}

#endif /* BUILD_S100 */
