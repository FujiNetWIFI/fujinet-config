/* fujisplash.c -- the boot splash. */

#include <arch/sms.h>

#include "fujidisp.h"
#include "fujisplash.h"

/* Twelve glyph groups of eight, top of the artwork to the bottom: white
 * through light blue to the BIOS's SEGA blue (SMS colour bytes, --BBGGRR). */
static const unsigned char logo_colors[12] = {
    0x3F, 0x3F, 0x3E, 0x3D, 0x3C, 0x38, 0x39, 0x34, 0x34, 0x30, 0x30, 0x20,
};

static void centre(unsigned char row, const char *s, unsigned char color)
{
    unsigned char n = 0;

    while (s[n] != '\0')
        n++;
    disp_at_color((unsigned char)(n >= DISP_COLS ? 0 : (DISP_COLS - n) / 2), row, s, color);
}

void splash_show(void)
{
    unsigned int g;
    unsigned char r, i;

    disp_init();
    disp_vdp_reg(1, 0x80);
    for (i = 0; i < 12; i++)
        disp_cram((unsigned char)(4 + i), logo_colors[i]);

    /* Glyph g in colour 4 + g/8: its one plane spread over that index. */
    disp_vram_addr(SPLASH_PATTERN * 32);
    for (g = 0; g < 96 * 8; g++) {
        unsigned char ci = (unsigned char)(4 + g / 64);
        unsigned char row = splash_patterns[g];

        IO_VDP_DATA = (ci & 1) ? row : 0;
        IO_VDP_DATA = (ci & 2) ? row : 0;
        IO_VDP_DATA = (ci & 4) ? row : 0;
        IO_VDP_DATA = (ci & 8) ? row : 0;
    }

    for (r = 0; r < 10; r++) {
        disp_vram_addr(0x3800 + (unsigned int)(SPLASH_ROW + r) * DISP_COLS * 2);
        for (i = 0; i < DISP_COLS; i++) {
            unsigned char t = splash_nametable[(unsigned int)r * DISP_COLS + i];

            IO_VDP_DATA = t ? t : ' ';
            IO_VDP_DATA = 0;
        }
    }

    centre(18, "FOR THE SEGA MASTER SYSTEM", DISP_WHITE);
    centre(21, "PRESS BUTTON 1", DISP_MAGENTA);
    disp_vdp_reg(1, 0xC0);
}
