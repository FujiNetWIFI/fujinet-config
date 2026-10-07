/* st_info.c -- the adapter info screen: GET_ADAPTERCONFIG_EXTENDED, drawn
 * field by field straight out of the 240-byte reply in the window, then the
 * cartridge's own state from its status page: the TV standard, whether
 * INPTCTRL is still unlocked (so games can go through the console's BIOS),
 * and the High Score Cart, which SELECT turns on and off.
 */

#include "fujidisp.h"
#include "fujiin.h"
#include "sfx.h"
#include "state.h"

#define VAL_COL   (IN_L + 10)
#define TV_ROW    15
#define INPT_ROW  16
#define HSC_ROW   17

static void draw_field(unsigned char row, const char *label,
                       unsigned int off, unsigned char max)
{
    disp_at(IN_L + 1, row, label);
    disp_text(VAL_COL, row, FN_REPLY + off, max);
}

static const char *tv_name(unsigned char pal)
{
    return pal ? "PAL" : "NTSC";
}

static void draw_cart(void)
{
    unsigned char hsc = fuji_a7800_hsc();

    disp_row_clear(TV_ROW);
    disp_at(IN_L + 1, TV_ROW, "TV");
    disp_at(VAL_COL, TV_ROW, tv_name(mt_pal));
    if (fuji_a7800_tv() != mt_pal) {
        disp_at(VAL_COL + 5, TV_ROW, "CART");
        disp_at(VAL_COL + 10, TV_ROW, tv_name(fuji_a7800_tv()));
    }

    disp_row_clear(INPT_ROW);
    disp_at(IN_L + 1, INPT_ROW, "INPTCTRL");
    disp_at(VAL_COL, INPT_ROW, "$");
    disp_at_hex8(VAL_COL + 1, INPT_ROW, fuji_a7800_inptctrl());
    disp_at(VAL_COL + 4, INPT_ROW,
            fuji_a7800_inpt_locked() ? "LOCKED" : "UNLOCKED");

    disp_row_clear(HSC_ROW);
    disp_at(IN_L + 1, HSC_ROW, "HSC");
    if (!(hsc & FUJI_A7800_HSC_ROM)) {
        disp_at(VAL_COL, HSC_ROW, "NOT INSTALLED");
        return;
    }
    disp_at(VAL_COL, HSC_ROW, hsc & FUJI_A7800_HSC_ON ? "ON" : "OFF");
    if (hsc & FUJI_A7800_HSC_DIRTY)
        disp_at(VAL_COL + 4, HSC_ROW, "SAVING");
    else if (hsc & FUJI_A7800_HSC_SD)
        disp_at(VAL_COL + 4, HSC_ROW, "SAVED");
}

void st_info(void)
{
    unsigned char shown;

    draw_frame("ADAPTER INFO");
    status_line("READING...");

    if (FUJICALL(FUJICMD_GET_ADAPTERCONFIG_EXTENDED)) {
        draw_field(2, "SSID", ACX_SSID, 18);
        draw_field(3, "HOST", ACX_HOSTNAME, 18);
        draw_field(5, "IP", ACX_SLOCALIP, 15);
        draw_field(6, "GATEWAY", ACX_SGATEWAY, 15);
        draw_field(7, "NETMASK", ACX_SNETMASK, 15);
        draw_field(8, "DNS", ACX_SDNSIP, 15);
        draw_field(10, "MAC", ACX_SMAC, 17);
        draw_field(11, "BSSID", ACX_SBSSID, 17);
        draw_field(13, "VERSION", ACX_VERSION, 14);
        status_line("SEL--HIGH SCORE CART");
        legend_line("1,2--BACK");
    } else {
        fail("INFO");
    }
    draw_cart();
    shown = fuji_a7800_hsc();

    while (state == ST_INFO) {
        unsigned char ev = in_read();
        unsigned char hsc = fuji_a7800_hsc();

        if (ev == IN_FIRE || ev == IN_BACK)
            state = ST_HOSTS;
        else if (ev == IN_SELECT) {
            if (!(hsc & FUJI_A7800_HSC_ROM)) {
                sfx_beep();
                continue;
            }
            fuji_a7800_hsc_op(hsc & FUJI_A7800_HSC_ON ? FUJI_A7800_HSCOP_OFF
                                                      : FUJI_A7800_HSCOP_ON);
        }
        if (hsc != shown) {
            shown = hsc;
            draw_cart();
        }
    }
}
