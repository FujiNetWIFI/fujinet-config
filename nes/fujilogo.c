/* fujilogo.c -- the FujiNet logo: beside its wordmark on HOST SLOTS, and
 * faded behind the file browser's list.
 *
 * The symbol is sprites (logo.txt, which mkfont.py cuts into 8x16 cells in
 * the CHR-ROM's otherwise empty $1000 table), so it costs none of the
 * background tiles the font and frame need. In the browser it sits *behind*
 * the background: a sprite with the priority bit set shows only where the
 * background is colour 0, so the names and the selection bar stay solid on
 * top and the logo shows between the letters. At 64 pixels wide it is
 * exactly the eight sprites a scanline allows.
 *
 * The wordmark beside it on HOST SLOTS is background tiles ($8F-$9C), so
 * disp_cls() clears it along with everything else.
 */

#include <string.h>

#include "fujidisp.h"

extern unsigned char oam[256];              /* oam.s */
extern const unsigned char logo_count;      /* mkfont.py --sprites */
extern const unsigned char logo_spr[];      /* dy, dx, tile per sprite */

#define ATTR_BEHIND 0x20        /* drawn under the background's colours 1-3 */

/* Where the logo's top-left pixel goes. conio row r is nametable row r+1,
 * and a sprite is drawn one line below its Y. */
#define HOSTS_X  105            /* between FUJI (cols 8-11) and NET (21-23) */
#define HOSTS_Y  120            /* conio row 14, under PLAY GAME LOBBY */
#define MARK_X   97             /* centred on IN_L..IN_R */
#define MARK_Y   80             /* centred on the list window, rows 5-20 */

/* FUJI and NET, level with the logo's middle row of circles. */
#define WORD_ROW 16

static unsigned char shown = 0xFF;  /* nothing known about OAM yet */

static void wordmark(void)
{
    static const unsigned char cols[7] = { 8, 9, 10, 11, 21, 22, 23 };
    unsigned char i, t = T_WORD0;

    for (i = 0; i < 7; i++, t += 2) {
        disp_tile(cols[i], WORD_ROW, t);
        disp_tile(cols[i], WORD_ROW + 1, (unsigned char)(t + 1));
    }
}

void disp_logo(unsigned char mode)
{
    const unsigned char *s = logo_spr;
    unsigned char i, n, x, y, attr;

    if (mode == shown)
        return;
    memset(oam, 0xFF, sizeof oam);  /* Y $FF: below the screen */
    if (mode != LOGO_OFF) {
        if (mode == LOGO_HOSTS) {
            x = HOSTS_X;
            y = HOSTS_Y - 1;
            attr = 1;               /* palette 1, in front */
            wordmark();
        } else {
            x = MARK_X;
            y = MARK_Y - 1;
            attr = ATTR_BEHIND;     /* palette 0 */
        }
        for (i = 0, n = 0; i < logo_count; i++, s += 3, n += 4) {
            oam[n] = (unsigned char)(y + s[0]);
            oam[n + 1] = s[2];
            oam[n + 2] = attr;
            oam[n + 3] = (unsigned char)(x + s[1]);
        }
    }
    shown = mode;
    oam_sync();
}
