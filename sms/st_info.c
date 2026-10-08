/* st_info.c -- the adapter info screen: GET_ADAPTERCONFIG_EXTENDED, drawn
 * field by field straight out of the 240-byte reply in the window (offsets in
 * constants.h). Completely drawn before anything else runs -- any transaction
 * would repaint the struct this is being read out of.
 */

#include "fujidisp.h"
#include "fujisnd.h"
#include "fujiin.h"
#include "state.h"

static void draw_value(unsigned char col, unsigned char row,
                       unsigned int off, unsigned char max)
{
    volatile unsigned char *r = FN_REPLY + off;
    unsigned char i;

    for (i = 0; i < max; i++) {
        char c = (char)r[i];

        if (c == 0)
            break;
        disp_char((unsigned char)(col + i), row, c);
    }
}

static void draw_field(unsigned char row, const char *label,
                       unsigned int off, unsigned char max)
{
    disp_at(1, row, label);
    draw_value(10, row, off, max);
}

void st_info(void)
{
    draw_frame("ADAPTER INFO");
    status_now("READING...");

    if (FUJICALL(FUJICMD_GET_ADAPTERCONFIG_EXTENDED)) {
        draw_field(LIST_Y(0), "SSID", ACX_SSID, 21);
        draw_field(LIST_Y(1), "HOST", ACX_HOSTNAME, 21);
        draw_field(LIST_Y(2), "IP", ACX_SLOCALIP, 15);
        draw_field(LIST_Y(3), "GATEWAY", ACX_SGATEWAY, 15);
        draw_field(LIST_Y(4), "NETMASK", ACX_SNETMASK, 15);
        draw_field(LIST_Y(5), "DNS", ACX_SDNSIP, 15);
        draw_field(LIST_Y(6), "MAC", ACX_SMAC, 17);
        draw_field(LIST_Y(7), "BSSID", ACX_SBSSID, 17);
        msg_put(MSG_LINE1, "VERSION");
        draw_value(10, MSG_LINE1, ACX_VERSION, 14);
        legend_line("1 OR 2 GOES BACK");
    } else {
        fail("EINFO");
    }

    while (state == ST_INFO) {
        unsigned char ev = in_read();

        if (ev == IN_FIRE || ev == IN_KEYSTAR) {
            snd_play(SND_BACK);
            state = ST_HOSTS;
        }
    }
}
