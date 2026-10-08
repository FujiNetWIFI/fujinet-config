/* config.c -- FujiNet CONFIG for the Sega Master System: entry point, the
 * state dispatcher, and the small drawing helpers every screen shares.
 *
 * Ported from ../coleco: one state byte, one screen module per state, each
 * running its own event loop until it hands the state to someone else, with
 * lists drawn straight out of the cartridge's reply window.
 */

#include <string.h>

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
    msg_type(MSG_LINE1, s);
}

void status_now(const char *s)
{
    msg_put(MSG_LINE1, s);
    snd_settle();
    cur_steady();
}

void legend_line(const char *s)
{
    msg_put(MSG_LINE2, s);
}

void fail_code(const char *what, unsigned char code)
{
    snd_play(SND_ERROR);
    msg_type(MSG_LINE1, what);
    disp_at_hex8(28, MSG_LINE1, code);
}

void fail(const char *what)
{
    fail_code(what, FN_ERRCODE);
}

void draw_frame(const char *title)
{
    layout_show();
    win_clear();
    win_title(title ? title : "");
    disp_row_blank(MSG_LINE1);
    disp_row_blank(MSG_LINE2);
}

void wait_frames(unsigned char n)
{
    in_wait(n);
}

/* The selected row: its text carries the invisible sprite-palette bit (for
 * the harness) and the left border beside it becomes the blinking pointer. */
void list_select(unsigned char i, bool on)
{
    unsigned char row = LIST_Y(i);

    if (on) {
        disp_row_attr(row, A_SEL, 0);
        cur_show(0, row, T_CURSOR, T_VEDGE);
    } else {
        cur_hide();
        disp_row_attr(row, 0, A_SEL);
    }
}

/* The cursor for the lists that fit on one screen (hosts). The browser and
 * the network list have their own versions with page-crossing. */
void bar_move(signed char d)
{
    if (nrows < 2)
        return;
    list_select(cur, false);
    if (d < 0)
        cur = (unsigned char)(cur ? cur - 1 : nrows - 1);
    else
        cur = (unsigned char)(cur + 1 < nrows ? cur + 1 : 0);
    list_select(cur, true);
    snd_play(SND_MOVE);
}

/* A COMMAND window at the top right of the main window: items on every
 * other row, the pointer in its left edge, up and down wrapping. The list's
 * own pointer stays lit underneath until it closes. */
unsigned char menu_pick(const char *const *names, const unsigned char *events,
                        unsigned char n)
{
    unsigned char i = 0, w = 0, k, x0, ev;

    for (k = 0; k < n; k++) {
        unsigned char len = (unsigned char)strlen(names[k]);

        if (len > w)
            w = len;
    }
    x0 = (unsigned char)(TEXT_R - w - 2);

    snd_play(SND_OK);
    ovl_open(x0, 1, TEXT_R - 1, (unsigned char)(1 + 2 * n));
    for (k = 0; k < n; k++)
        disp_at((unsigned char)(x0 + 1), (unsigned char)(2 + 2 * k), names[k]);
    in_repeat(false);
    cur_show(x0, 2, T_CURSOR, T_VEDGE);
    for (;;) {
        ev = in_read();
        if (ev == IN_UP || ev == IN_DOWN) {
            if (ev == IN_UP)
                i = (unsigned char)(i ? i - 1 : n - 1);
            else
                i = (unsigned char)(i + 1 < n ? i + 1 : 0);
            cur_hide();
            cur_show(x0, (unsigned char)(2 + 2 * i), T_CURSOR, T_VEDGE);
            snd_play(SND_MOVE);
        } else if (ev == IN_FIRE || ev == IN_KEYSTAR || ev == IN_MENU) {
            break;
        }
    }
    in_repeat(true);
    snd_play(ev == IN_FIRE ? SND_OK : SND_BACK);
    ovl_close();
    if (nrows)
        list_select(cur, true);
    return ev == IN_FIRE ? events[i] : IN_NONE;
}

void main(void)
{
    snd_init();
    in_init();
    splash_show();
    in_init();                  /* drop presses made during the fade */
    disp_init();

    if (!fuji_sms_present()) {
        draw_frame("FUJINET");
        snd_play(SND_ERROR);
        status_line("NO FUJINET CART");
        for (;;)
            in_frames();        /* keeps the sound ticking to its end */
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
