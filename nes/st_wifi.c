/* st_wifi.c -- the three WiFi states. The scan list is drawn straight out of
 * the reply window, one GET_SCAN_RESULT per row. Picking a network re-fetches
 * its result and holds the SSID in src_spec across the password edit.
 */

#include <string.h>

#include "fujidisp.h"
#include "fujiin.h"
#include "fujiedit.h"
#include "fujiraw.h"
#include "sfx.h"
#include "state.h"

static unsigned char nnets;

void st_check_wifi(void)
{
    volatile unsigned char *r = FN_REPLY;
    unsigned char s = 0;

    if (!fuji_get_wifi_enabled()) {
        state = ST_HOSTS;
        return;
    }
    if (!fuji_get_wifi_status(&s)) {
        state = ST_HOSTS;
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

    draw_frame("JOINING NETWORK");
    status_line("PLEASE WAIT  B--CANCEL");
    if (FUJICALL(FUJICMD_GET_SSID))
        for (i = 0; i < SSID_LEN - 1 && r[i] != 0; i++)
            disp_char((unsigned char)(IN_L + 1 + i), 5, (char)r[i]);

    for (tries = 0; tries < 20; tries++) {
        unsigned char start = in_frames();

        while ((unsigned char)(in_frames() - start) < 120) {
            unsigned char ev = in_read();

            if (ev == IN_BACK || ev == IN_FIRE) {
                state = ST_SET_WIFI;
                return;
            }
        }

        s = 0;
        if (!fuji_get_wifi_status(&s))
            continue;
        switch (s) {
        case 3:
            status_line("CONNECTED");
            disp_at(IN_L, LEGEND_ROW, "OK");
            sfx_accept();
            wait_frames(50);
            state = ST_HOSTS;
            return;
        case 1:
            status_line("?NO SSID AVAILABLE");
            sfx_beep();
            wait_frames(120);
            state = ST_SET_WIFI;
            return;
        case 4:
            status_line("?CONNECT FAILED");
            sfx_beep();
            wait_frames(120);
            state = ST_SET_WIFI;
            return;
        case 5:
            status_line("?CONNECTION LOST");
            sfx_beep();
            wait_frames(120);
            state = ST_SET_WIFI;
            return;
        }
    }
    status_line("?UNABLE TO CONNECT");
    sfx_beep();
    wait_frames(120);
    state = ST_SET_WIFI;
}

static void draw_rssi(unsigned char row, unsigned char raw)
{
    signed char dbm = (signed char)raw;
    unsigned int mag = (unsigned int)(dbm < 0 ? -dbm : dbm);

    disp_at(25, row, dbm < 0 ? "-" : " ");
    disp_at_u16(26, row, mag);
}

static void wifi_draw(void)
{
    volatile unsigned char *r = FN_REPLY;
    unsigned char i;

    draw_frame("SELECT NETWORK");
    status_line("SCANNING...");
    disp_row_clear(LEGEND_ROW);

    nnets = 0;
    if (FUJICALL(FUJICMD_SCAN_NETWORKS)) {
        nnets = r[0];
        if (nnets > WIFI_MAX)
            nnets = WIFI_MAX;
    }
    for (i = 0; i < nnets; i++) {
        unsigned char row = (unsigned char)(LIST_TOP + i);
        unsigned char col;

        if (!FUJICALL_A1(FUJICMD_GET_SCAN_RESULT, i)) {
            nnets = i;
            break;
        }
        for (col = 0; col < 21 && r[col] != 0; col++)
            disp_char((unsigned char)(3 + col), row, (char)r[col]);
        draw_rssi(row, r[SSID_LEN]);
    }
    disp_at(3, (unsigned char)(LIST_TOP + nnets), "OTHER (TYPE AN SSID)");
    nrows = (unsigned char)(nnets + 1);

    status_line("A--SELECT  START--RESCAN");
    legend_line("B--HOSTS");
    if (cur >= nrows)
        cur = 0;
    disp_row_invert((unsigned char)(LIST_TOP + cur), true);
}

static void wifi_pick(void)
{
    volatile unsigned char *r = FN_REPLY;
    unsigned char i;

    if (cur < nnets) {
        if (!FUJICALL_A1(FUJICMD_GET_SCAN_RESULT, cur)) {
            fail("SCAN");
            return;
        }
        for (i = 0; i < SSID_LEN - 1 && r[i] != 0; i++)
            src_spec[i] = (char)r[i];
        src_spec[i] = 0;
    } else {
        fn_entry[0] = 0;
        if (!fn_edit("SSID", SSID_LEN - 1) || fn_entry[0] == 0) {
            wifi_draw();
            return;
        }
        strcpy(src_spec, fn_entry);
    }

    fn_entry[0] = 0;
    if (!fn_edit("PASSWORD", PASS_MAX)) {
        wifi_draw();
        return;
    }

    status_line("SETTING SSID...");
    if (!fnraw_set_ssid(src_spec, fn_entry)) {
        fail("SSID");
        wait_frames(120);
        wifi_draw();
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

        switch (ev) {
        case IN_UP:
            bar_move(-1);
            break;
        case IN_DOWN:
            bar_move(1);
            break;
        case IN_FIRE:
            wifi_pick();
            break;
        case IN_START:
            cur = 0;
            wifi_draw();
            break;
        case IN_BACK:
            state = ST_HOSTS;
            break;
        }
    }
}
