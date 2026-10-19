/* st_info.c -- the adapter info screen: GET_ADAPTERCONFIG_EXTENDED, drawn
 * field by field straight out of the 240-byte reply in the window.
 */

#include "fujidisp.h"
#include "fujiin.h"
#include "sfx.h"
#include "state.h"

#define VAL_COL   (IN_L + 10)
#define VAL_W     (IN_R + 1 - VAL_COL)

static void draw_field(unsigned char row, const char *label,
                       unsigned int off, unsigned char max)
{
    disp_at(IN_L + 1, row, label);
    disp_text(VAL_COL, row, FN_REPLY + off, max);
}

void st_info(void)
{
    draw_frame("ADAPTER INFO");
    status_line("READING...");

    if (FUJICALL(FUJICMD_GET_ADAPTERCONFIG_EXTENDED)) {
        draw_field(2, "SSID", ACX_SSID, VAL_W);
        draw_field(3, "HOST", ACX_HOSTNAME, VAL_W);
        draw_field(5, "IP", ACX_SLOCALIP, 15);
        draw_field(6, "GATEWAY", ACX_SGATEWAY, 15);
        draw_field(7, "NETMASK", ACX_SNETMASK, 15);
        draw_field(8, "DNS", ACX_SDNSIP, 15);
        draw_field(10, "MAC", ACX_SMAC, 17);
        draw_field(11, "BSSID", ACX_SBSSID, 17);
        draw_field(13, "VERSION", ACX_VERSION, 14);
        status_line("");
        legend_line("#,*--BACK");
    } else {
        fail("INFO");
    }

    while (state == ST_INFO) {
        unsigned char ev = in_read();

        if (ev == IN_FIRE || ev == IN_BACK)
            state = ST_HOSTS;
    }
}
