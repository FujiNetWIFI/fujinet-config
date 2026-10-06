/* fujisnd.c -- the SN76489 on port $7F. */

#include <arch/sms.h>

#include "fujisnd.h"

static unsigned char left;

void snd_init(void)
{
    IO_PSG = 0x9F;
    IO_PSG = 0xBF;
    IO_PSG = 0xDF;
    IO_PSG = 0xFF;
    left = 0;
}

void snd_click(void)
{
    IO_PSG = 0x8E;                      /* tone 0, period $0E0: ~1 kHz */
    IO_PSG = 0x0E;
    IO_PSG = 0x92;                      /* loud */
    left = 3;
}

void snd_tick(void)
{
    if (left && --left == 0)
        IO_PSG = 0x9F;
}
