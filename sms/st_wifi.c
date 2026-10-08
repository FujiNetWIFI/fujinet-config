/* st_wifi.c -- the three WiFi states, mirroring src/check_wifi.c,
 * src/connect_wifi.c and src/set_wifi.c.
 *
 * The scan list is drawn straight out of the reply window, one
 * GET_SCAN_RESULT per row. Picking a network re-fetches its result and holds
 * the SSID in src_spec across the password edit -- the editor owns fn_entry,
 * and using src_spec (never busy during WiFi setup; see state.h) makes the
 * scan-pick and type-your-own paths identical from SET_SSID's point of view.
 */

#include <string.h>

#include "fujidisp.h"
#include "fujisnd.h"
#include "fujiin.h"
#include "fujiedit.h"
#include "fujiraw.h"
#include "state.h"

static unsigned char nnets;     /* networks the scan found (<= WIFI_MAX) */
static unsigned char wtop;      /* first entry on screen: 0 or LIST_ROWS */

void st_check_wifi(void)
{
    volatile unsigned char *r = FN_REPLY;
    unsigned char s = 0;

    /* Wifi can be disabled outright in the adapter's config; don't drag the
     * user through the connect screens if it is (src/check_wifi.c). */
    if (!fuji_get_wifi_enabled()) {
        state = ST_HOSTS;
        return;
    }
    if (!fuji_get_wifi_status(&s)) {
        state = ST_HOSTS;       /* can't ask: let the browser try its luck */
        return;
    }
    if (s == 3) {
        state = ST_HOSTS;
        return;
    }
    if (FUJICALL(FUJICMD_GET_SSID) && r[0] != 0)
        state = ST_CONNECT_WIFI;
    else
        state = ST_SET_WIFI;
}

void st_connect_wifi(void)
{
    volatile unsigned char *r = FN_REPLY;
    unsigned char tries, i, s;

    draw_frame("JOINING");
    legend_line("2 GIVES UP");
    if (FUJICALL(FUJICMD_GET_SSID))
        for (i = 0; i < SSID_LEN - 1 && r[i] != 0; i++)
            disp_char((unsigned char)(1 + i), LIST_TOP, (char)r[i]);
    status_line("PLEASE WAIT...");

    for (tries = 0; tries < 20; tries++) {
        unsigned char start = in_frames();

        /* ~2 seconds between polls, abortable the whole time. */
        while ((unsigned char)(in_frames() - start) < 120) {
            unsigned char ev = in_read();

            if (ev == IN_KEYSTAR || ev == IN_FIRE) {
                snd_play(SND_BACK);
                state = ST_SET_WIFI;
                return;
            }
        }

        s = 0;
        if (!fuji_get_wifi_status(&s))
            continue;
        switch (s) {
        case 3:
            snd_play(SND_CONNECT);
            status_line("CONNECTED!");
            wait_frames(60);
            state = ST_HOSTS;
            return;
        case 1:
            snd_play(SND_ERROR);
            status_line("NO SSID AVAILABLE.");
            wait_frames(120);
            state = ST_SET_WIFI;
            return;
        case 4:
            snd_play(SND_ERROR);
            status_line("CONNECT FAILED.");
            wait_frames(120);
            state = ST_SET_WIFI;
            return;
        case 5:
            snd_play(SND_ERROR);
            status_line("CONNECTION LOST.");
            wait_frames(120);
            state = ST_SET_WIFI;
            return;
        }
    }
    snd_play(SND_ERROR);
    status_line("UNABLE TO CONNECT.");
    wait_frames(120);
    state = ST_SET_WIFI;
}

/* RSSI arrives as a uint8_t but means dBm, so it has to be read back as
 * signed or every network looks like a very strong +200. */
static void draw_rssi(unsigned char row, unsigned char raw)
{
    signed char dbm = (signed char)raw;
    unsigned int mag = (unsigned int)(dbm < 0 ? -dbm : dbm);

    disp_at(26, row, dbm < 0 ? "-" : " ");
    disp_at_u16(27, row, mag);
}

/* One page of the list: up to 8 entries from wtop, each network re-fetched
 * from the scan (GET_SCAN_RESULT is random access), and OTHER after the
 * last network. */
static void wifi_page(void)
{
    volatile unsigned char *r = FN_REPLY;
    unsigned char i;

    win_clear();
    nrows = 0;
    for (i = 0; i < LIST_ROWS; i++) {
        unsigned char idx = (unsigned char)(wtop + i);
        unsigned char row = LIST_Y(i);
        unsigned char col;

        if (idx > nnets)
            break;
        if (idx == nnets) {
            disp_at(1, row, "OTHER (TYPE AN SSID)");
            nrows = (unsigned char)(i + 1);
            break;
        }
        if (!FUJICALL_A1(FUJICMD_GET_SCAN_RESULT, idx)) {
            nnets = idx;        /* the rest are gone: OTHER moves up */
            disp_at(1, row, "OTHER (TYPE AN SSID)");
            nrows = (unsigned char)(i + 1);
            break;
        }
        for (col = 0; col < 24 && r[col] != 0; col++)
            disp_char((unsigned char)(1 + col), row, (char)r[col]);
        draw_rssi(row, r[SSID_LEN]);
        nrows = (unsigned char)(i + 1);
    }
    if (cur >= nrows)
        cur = (unsigned char)(nrows - 1);
    list_select(cur, true);
}

static void wifi_draw(void)
{
    volatile unsigned char *r = FN_REPLY;

    draw_frame("NETWORKS");
    status_now("SCANNING...");

    nnets = 0;
    if (FUJICALL(FUJICMD_SCAN_NETWORKS)) {
        nnets = r[0];
        if (nnets > WIFI_MAX)
            nnets = WIFI_MAX;
    }
    wtop = 0;
    if (cur >= LIST_ROWS)
        cur = 0;
    wifi_page();
    legend_line("1 PICK  2 HOSTS  PAUSE RESCAN");
    status_line("SELECT A NETWORK.");
}

/* Back from the editor: the scan is still good, so just put the page back. */
static void wifi_redraw(void)
{
    draw_frame("NETWORKS");
    wifi_page();
    legend_line("1 PICK  2 HOSTS  PAUSE RESCAN");
    status_line("SELECT A NETWORK.");
}

/* The cursor, crossing between the two pages. */
static void wmove(signed char d)
{
    if (d < 0) {
        if (cur > 0) {
            list_select(cur, false);
            cur--;
            list_select(cur, true);
        } else if (wtop) {
            wtop = 0;
            cur = LIST_ROWS - 1;
            wifi_page();
        } else {
            return;
        }
    } else {
        if ((unsigned char)(cur + 1) < nrows) {
            list_select(cur, false);
            cur++;
            list_select(cur, true);
        } else if (!wtop && nnets + 1 > LIST_ROWS) {
            wtop = LIST_ROWS;
            cur = 0;
            wifi_page();
        } else {
            return;
        }
    }
    snd_play(SND_MOVE);
}

static void wifi_pick(void)
{
    volatile unsigned char *r = FN_REPLY;
    unsigned char idx = (unsigned char)(wtop + cur);
    unsigned char i;

    snd_play(SND_OK);
    if (idx < nnets) {
        /* Re-fetch so the SSID is fresh in the window, then hold it in
         * src_spec across the password edit. */
        if (!FUJICALL_A1(FUJICMD_GET_SCAN_RESULT, idx)) {
            fail("ESCAN");
            return;
        }
        for (i = 0; i < SSID_LEN - 1 && r[i] != 0; i++)
            src_spec[i] = (char)r[i];
        src_spec[i] = 0;
    } else {
        fn_entry[0] = 0;
        if (!fn_edit("NETWORK", "INPUT THE NETWORK NAME.", SSID_LEN - 1) ||
            fn_entry[0] == 0) {
            wifi_redraw();
            return;
        }
        strcpy(src_spec, fn_entry);
    }

    fn_entry[0] = 0;
    if (!fn_edit("PASSWORD", "INPUT THE PASSWORD.", PASS_MAX)) {
        wifi_redraw();
        return;
    }

    status_now("SETTING SSID...");
    if (!fnraw_set_ssid(src_spec, fn_entry)) {
        fail("ESSID");
        wait_frames(120);
        wifi_redraw();
        return;
    }
    state = ST_CONNECT_WIFI;
}

void st_set_wifi(void)
{
    cur = 0;
    wifi_draw();
    while (state == ST_SET_WIFI) {
        unsigned char ev = in_read();

        if (ev == IN_MENU)
            ev = IN_KEYHASH;            /* the one action: rescan */
        switch (ev) {
        case IN_UP:
            wmove(-1);
            break;
        case IN_DOWN:
            wmove(1);
            break;
        case IN_LEFT:
            if (wtop) {
                wtop = 0;
                cur = 0;
                wifi_page();
                snd_play(SND_MOVE);
            }
            break;
        case IN_RIGHT:
            if (!wtop && nnets + 1 > LIST_ROWS) {
                wtop = LIST_ROWS;
                cur = 0;
                wifi_page();
                snd_play(SND_MOVE);
            }
            break;
        case IN_FIRE:
            wifi_pick();
            break;
        case IN_KEYHASH:
            snd_play(SND_OK);
            cur = 0;
            wifi_draw();
            break;
        case IN_KEYSTAR:
            snd_play(SND_BACK);
            state = ST_HOSTS;
            break;
        }
    }
}
