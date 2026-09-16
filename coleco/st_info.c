/* st_info.c -- the adapter info screen: GET_ADAPTERCONFIG_EXTENDED, drawn
 * field by field straight out of the 240-byte reply in the window (offsets in
 * constants.h). Completely drawn before anything else runs -- any transaction
 * would repaint the struct this is being read out of.
 */

#include "fujidisp.h"
#include "fujiin.h"
#include "state.h"

static void draw_field(unsigned char row, const char *label,
                       unsigned int off, unsigned char max)
{
    volatile unsigned char *r = FN_REPLY + off;
    unsigned char i;

    disp_at(1, row, label);
    for (i = 0; i < max; i++) {
        char c = (char)r[i];

        if (c == 0)
            break;
        disp_char((unsigned char)(10 + i), row, c);
    }
}

void st_info(void)
{
    draw_frame("ADAPTER INFO");
    status_line("READING...");

    if (FUJICALL(FUJICMD_GET_ADAPTERCONFIG_EXTENDED)) {
        draw_field(5, "SSID", ACX_SSID, 22);
        draw_field(6, "HOST", ACX_HOSTNAME, 22);
        draw_field(8, "IP", ACX_SLOCALIP, 15);
        draw_field(9, "GATEWAY", ACX_SGATEWAY, 15);
        draw_field(10, "NETMASK", ACX_SNETMASK, 15);
        draw_field(11, "DNS", ACX_SDNSIP, 15);
        draw_field(13, "MAC", ACX_SMAC, 17);
        draw_field(14, "BSSID", ACX_SBSSID, 17);
        draw_field(16, "VERSION", ACX_VERSION, 14);
        status_line("FIRE OR * GOES BACK");
    } else {
        fail("EINFO");
    }

    while (state == ST_INFO) {
        unsigned char ev = in_read();

        if (ev == IN_FIRE || ev == IN_KEYSTAR)
            state = ST_HOSTS;
    }
}
