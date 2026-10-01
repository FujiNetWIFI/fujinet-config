/* config.c -- FujiNet CONFIG for the NES: entry point, the state dispatcher,
 * and the small drawing helpers every screen shares.
 *
 * Ported from the ColecoVision CONFIG (../coleco): one state byte, one screen
 * module per state, each running its own event loop until it hands the state
 * to someone else. Lists are drawn straight out of the cartridge's reply
 * window at $5000 and streamed straight back into the next transaction.
 */

#include "fujidisp.h"
#include "fujiin.h"
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
    disp_at(1, 1, "FUJINET   C O N F I G");
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

void main(void)
{
    in_init();
    disp_init();

    if (!fuji_nes_present()) {
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
        default:
            st_hosts();
            break;
        }
    }
}
