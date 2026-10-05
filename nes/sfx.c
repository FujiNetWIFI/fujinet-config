/* sfx.c -- see sfx.h. Plain stores to the APU's write-only registers. */

#include <nes.h>

#include "sfx.h"

#define APUREG(a)     (*(volatile unsigned char *)(a))
#define SQ1_VOL    APUREG(0x4000)
#define SQ1_SWEEP  APUREG(0x4001)
#define SQ1_LO     APUREG(0x4002)
#define SQ1_HI     APUREG(0x4003)
#define TRI_LINEAR APUREG(0x4008)
#define TRI_LO     APUREG(0x400A)
#define TRI_HI     APUREG(0x400B)
#define SND_CHN    APUREG(0x4015)
#define FRAME_CNT  APUREG(0x4017)

/* The arpeggio's triangle periods: C4 E4 G4 C5. */
static const unsigned char accept_notes[4] = { 0xD4, 0xA8, 0x8D, 0x69 };

static void frames(unsigned char n)
{
    while (n--)
        waitvsync();
}

void sfx_init(void)
{
    FRAME_CNT = 0x40;           /* 4-step, frame IRQ off */
    SND_CHN = 0x00;
}

/* Pulse 1 at 25% duty, constant volume 15, period $05F (about 1165 Hz),
 * length counter halted so it sounds until switched off. */
static void tone_on(void)
{
    SND_CHN = 0x01;
    FRAME_CNT = 0xC0;
    SQ1_VOL = 0xEF;
    SQ1_SWEEP = 0x00;
    SQ1_LO = 0x5F;
    SQ1_HI = 0x00;
}

void sfx_click(void)
{
    unsigned char i;

    tone_on();
    for (i = 0; i < 100; i++)   /* about 0.7 ms */
        ;
    SND_CHN = 0x00;
}

void sfx_beep(void)
{
    tone_on();
    frames(10);
    SND_CHN = 0x00;
}

void sfx_accept(void)
{
    unsigned char i;

    SND_CHN = 0x04;
    for (i = 0; i < sizeof accept_notes; i++) {
        TRI_LINEAR = 0x08;      /* the linear counter ends each note */
        TRI_LO = accept_notes[i];
        TRI_HI = 0x08;
        frames(2);
    }
    frames(2);
    SND_CHN = 0x00;
}

void sfx_blip(void)
{
    SND_CHN = 0x01;
    SQ1_VOL = 0x80;             /* 50% duty, fast decaying envelope */
    SQ1_SWEEP = 0x7F;
    SQ1_LO = 0x7E;              /* A5 */
    SQ1_HI = 0x08;
    frames(2);
    SQ1_VOL = 0x90;             /* constant volume 0 */
}
