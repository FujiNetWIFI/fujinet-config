/* config.c -- FujiNet CONFIG for the NES: entry point, the state dispatcher,
 * and the small drawing helpers every screen shares.
 *
 * Ported from the ColecoVision CONFIG (../coleco): one state byte, one screen
 * module per state, each running its own event loop until it hands the state
 * to someone else. Lists are drawn straight out of the cartridge's reply
 * window at $5000 and streamed straight back into the next transaction.
 *
 * It looks and sounds like the Famicom's Family BASIC: the GAME BASIC menu's
 * frame around every list, "A--OPEN" legends, "?xx ERROR" with a BEEP, a
 * click on every keypress.
 */

#include <string.h>

#include "fujidisp.h"
#include "fujiin.h"
#include "sfx.h"
#include "state.h"

unsigned char state;

unsigned char host;
unsigned char cur;
unsigned char nrows;
unsigned char at_end;
unsigned int  top;
unsigned char hosts_mask;

char path[PATH_MAX_LEN];
char src_spec[SPEC_MAX_LEN];

void status_line(const char *s)
{
    disp_row_clear(STATUS_ROW);
    disp_at(IN_L, STATUS_ROW, s);
}

void legend_line(const char *s)
{
    disp_row_clear(LEGEND_ROW);
    disp_at(IN_L, LEGEND_ROW, s);
}

/* "?MOUNT ERROR 8A", the way Family BASIC says "?SN ERROR", and its BEEP. */
void fail(const char *what)
{
    unsigned char col = (unsigned char)(IN_L + 1 + strlen(what));

    disp_row_clear(STATUS_ROW);
    disp_at(IN_L, STATUS_ROW, "?");
    disp_at(IN_L + 1, STATUS_ROW, what);
    disp_at(col, STATUS_ROW, " ERROR ");
    disp_at_hex8((unsigned char)(col + 7), STATUS_ROW, FN_ERRCODE);
    sfx_beep();
}

void draw_frame(const char *title)
{
    disp_frame(title);
}

void wait_frames(unsigned char n)
{
    unsigned char start = in_frames();

    while ((unsigned char)(in_frames() - start) < n)
        ;
}

/* The selection bar for the lists that fit on one screen (hosts, networks).
 * The browser has its own version with page-crossing. */
void bar_move(signed char d)
{
    if (d < 0) {
        if (cur == 0)
            return;
        disp_row_invert((unsigned char)(LIST_TOP + cur), false);
        cur--;
    } else {
        if ((unsigned char)(cur + 1) >= nrows)
            return;
        disp_row_invert((unsigned char)(LIST_TOP + cur), false);
        cur++;
    }
    disp_row_invert((unsigned char)(LIST_TOP + cur), true);
}

/* The power-on screen, after Family BASIC's: no frame, the name typing
 * itself out a character at a time, then OK and the blinking cursor. Any
 * key cuts it short. */
static void splash(void)
{
    static const char name[] = "FUJINET CONFIG";
    unsigned char i;

    for (i = 0; name[i]; i++) {
        disp_char((unsigned char)(IN_L + i), 3, name[i]);
        sfx_blip();
        wait_frames(1);
    }
    disp_at(IN_L, 4, "FOR THE NES");
    disp_at(IN_L, 5, "(C) FUJINET PROJECT");
    disp_at(IN_L, 6, "OK");
    disp_cursor_at(IN_L, 7);
    for (i = 0; i < 30; i++) {
        if (in_read() != IN_NONE)
            break;
        disp_cursor_tick();
    }
    disp_cursor_off();
}

void main(void)
{
    sfx_init();
    in_init();
    disp_init();

    if (!fuji_nes_present()) {
        disp_at(IN_L, 3, "?NO FUJINET CART");
        sfx_beep();
        for (;;)
            ;
    }

    splash();

    state = ST_CHECK_WIFI;
    for (;;) {
        switch (state) {
        case ST_CHECK_WIFI:
            st_check_wifi();
            break;
        case ST_SET_WIFI:
            st_set_wifi();
            break;
        case ST_CONNECT_WIFI:
            st_connect_wifi();
            break;
        case ST_FILES:
            st_files();
            break;
        case ST_INFO:
            st_info();
            break;
        default:
            st_hosts();
            break;
        }
    }
}
