/* config.c -- FujiNet CONFIG for the Atari 7800: entry point, the state
 * dispatcher, and the small drawing helpers every screen shares.
 *
 * Ported from the NES CONFIG (../nes), with host-to-host copy from the
 * Master System CONFIG: one state byte, one screen module per state,
 * each running its own event loop until it hands the state to someone else.
 * Lists are drawn straight out of the cartridge's reply window at $0800 and
 * streamed straight back into the next transaction.
 */

#include <string.h>

#include "fujidisp.h"
#include "fujiin.h"
#include "sfx.h"
#include "state.h"

unsigned char state;

unsigned char host;
unsigned char src_host;
unsigned char copy_mode;

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

/* "?MOUNT ERROR 02", with a BEEP. */
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
    while (n--)
        mt_sync();
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

/* The power-on screen: the name typing itself out a character at a time,
 * then OK and the blinking cursor. Any key cuts it short. */
static void splash(void)
{
    static const char name[] = "FUJINET CONFIG";
    unsigned char i;

    for (i = 0; name[i]; i++) {
        disp_char((unsigned char)(IN_L + i), 2, name[i]);
        sfx_blip();
    }
    disp_at(IN_L, 3, "FOR THE ATARI 7800");
    disp_at(IN_L, 4, "(C) FUJINET PROJECT");
    disp_at(IN_L, 5, "OK");
    disp_cursor_at(IN_L, 6);
    for (i = 0; i < 30; i++) {
        if (in_read() != IN_NONE)
            break;
        disp_cursor_tick();
    }
    disp_cursor_off();
}

void main(void)
{
    disp_init();
    sfx_init();
    in_init();

    if (!fuji_a7800_present()) {
        disp_at(IN_L, 2, "?NO FUJINET CART");
        sfx_beep();
        for (;;)
            ;
    }
    fuji_a7800_set_tv(mt_pal);

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
        case ST_LOBBY:
            st_lobby();
            break;
        default:
            st_hosts();
            break;
        }
    }
}
