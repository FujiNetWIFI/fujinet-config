/* config.c -- FujiNet CONFIG for the Sega Master System: entry point, the
 * state dispatcher, and the small drawing helpers every screen shares.
 *
 * Ported from ../coleco: one state byte, one screen module per state, each
 * running its own event loop until it hands the state to someone else, with
 * lists drawn straight out of the cartridge's reply window.
 */

#include "fujidisp.h"
#include "fujisnd.h"
#include "fujiin.h"
#include "fujisplash.h"
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
    disp_at(1, STATUS_ROW, s);
}

void legend_line(const char *s)
{
    disp_row_clear(LEGEND_ROW);
    disp_at(1, LEGEND_ROW, s);
}

void fail(const char *what)
{
    disp_row_clear(STATUS_ROW);
    disp_at(1, STATUS_ROW, what);
    disp_at_hex8(28, STATUS_ROW, FN_ERRCODE);
}

void draw_frame(const char *subtitle)
{
    disp_cls();
    disp_at_color(1, 1, "FUJINET", DISP_BLUE);
    disp_at(10, 1, "C O N F I G");
    if (subtitle)
        disp_at(1, 3, subtitle);
}

void wait_frames(unsigned char n)
{
    unsigned char start = in_frames();

    while ((unsigned char)(in_frames() - start) < n)
        ;
}

/* The selection bar for the lists that fit on one screen (hosts, networks).
 * The browser has its own version with page-crossing. Every accepted move
 * clicks; a move that goes nowhere stays silent. */
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
    snd_click();
}

unsigned char menu_pick(const char *const *names, const unsigned char *events,
                        unsigned char n)
{
    unsigned char i = 0, ev;

    snd_click();
    for (;;) {
        disp_row_clear(STATUS_ROW);
        disp_at_color(1, STATUS_ROW, "<", DISP_MAGENTA);
        disp_at_hi(3, STATUS_ROW, names[i], true);
        disp_at_color(30, STATUS_ROW, ">", DISP_MAGENTA);
        legend_line("1 DO IT  2 OR PAUSE CANCEL");
        do
            ev = in_read();
        while (ev == IN_NONE || ev == IN_UP || ev == IN_DOWN);
        if (ev == IN_LEFT)
            i = (unsigned char)(i ? i - 1 : n - 1);
        else if (ev == IN_RIGHT)
            i = (unsigned char)(i + 1 < n ? i + 1 : 0);
        else
            break;
        snd_click();
    }
    disp_row_clear(STATUS_ROW);
    disp_row_clear(LEGEND_ROW);
    return ev == IN_FIRE ? events[i] : IN_NONE;
}

void main(void)
{
    snd_init();

    /* The logo's tiles overlap the magenta font; disp_init() puts it back. */
    splash_show();
    in_init();
    {
        unsigned char start = in_frames();

        while ((unsigned char)(in_frames() - start) < 120)
            if (in_read() == IN_FIRE)
                break;
    }

    disp_init();

    if (!fuji_sms_present()) {
        disp_at(4, 8, "NO FUJINET CART");
        for (;;)
            ;
    }

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
