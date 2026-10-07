/* sfx.c -- see sfx.h. Plain stores to POKEY channel 1. */

#include "maria.h"
#include "sfx.h"

#define POKEY(r)   (*(volatile unsigned char *)(0x0450 + (r)))
#define AUDF1      POKEY(0x0)
#define AUDC1      POKEY(0x1)
#define AUDCTL     POKEY(0x8)
#define SKCTL      POKEY(0xF)

#define PURE       0xA0         /* AUDC: pure tone; low nibble the volume */

/* AUDF for the 64 kHz clock: f = 63921 / (2 * (AUDF + 1)). */
#define F_CLICK    26           /* ~1180 Hz */
#define F_A5       35

/* The arpeggio: C5 E5 G5 C6. */
static const unsigned char accept_notes[4] = { 60, 47, 40, 30 };

static void frames(unsigned char n)
{
    while (n--)
        mt_sync();
}

void sfx_init(void)
{
    SKCTL = 0;
    SKCTL = 3;                  /* out of initialisation: the counters run */
    AUDCTL = 0;
    AUDC1 = 0;
}

static void tone(unsigned char f, unsigned char vol)
{
    AUDF1 = f;
    AUDC1 = (unsigned char)(PURE | vol);
}

void sfx_click(void)
{
    unsigned char i;

    tone(F_CLICK, 8);
    for (i = 0; i < 100; i++)   /* about 0.7 ms */
        ;
    AUDC1 = 0;
}

void sfx_beep(void)
{
    tone(F_CLICK, 8);
    frames(10);
    AUDC1 = 0;
}

void sfx_accept(void)
{
    unsigned char i;

    for (i = 0; i < sizeof accept_notes; i++) {
        tone(accept_notes[i], 8);
        frames(2);
    }
    frames(2);
    AUDC1 = 0;
}

void sfx_blip(void)
{
    tone(F_A5, 6);
    frames(2);
    AUDC1 = 0;
}
