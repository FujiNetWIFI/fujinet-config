/* st_boot.c -- mount into device slot 0 and take the ROM swap.
 *
 * Not a dispatcher state: booting is called from wherever the choice was made
 * (the browser, the lobby) and only ever RETURNS on failure -- success ends
 * in fuji_sms_boot(), which hands the console to the cart's loader page;
 * the loader copies the image into the SRAM and starts it. A power cycle
 * brings CONFIG back.
 */

#include "fujidisp.h"
#include "fujiraw.h"
#include "state.h"

void boot_mount_swap(void)
{
    unsigned char pct = 0xFF;

    status_line("MOUNTING...");
    if (!fuji_mount_disk_image(DEVICE_SLOT, MODE_READ)) {
        fail("EMOUNT");
        return;
    }

    /* The image arrives asynchronously, pushed to the cartridge while the
     * console keeps running; these three bytes are the cart's progress. */
    status_line("LOADING");
    for (;;) {
        unsigned char st = fuji_sms_boot_state();

        if (st == FUJI_SMS_BOOT_READY)
            break;
        if (st == FUJI_SMS_BOOT_FAILED) {
            disp_row_clear(STATUS_ROW);
            disp_at(1, STATUS_ROW, "ELOAD");
            disp_at_hex8(28, STATUS_ROW, fuji_sms_boot_error());
            return;
        }
        if (fuji_sms_boot_percent() != pct) {
            pct = fuji_sms_boot_percent();
            disp_at_u16(24, STATUS_ROW, pct);
        }
    }

    status_line("BOOTING");
    fuji_sms_boot();            /* does not return */
}

/* Boot the entry whose FULL-width name is sitting in the reply window (the
 * caller just re-read it there): the path prefix comes from RAM, the filename
 * streams cartridge-to-cartridge without ever landing in console RAM. */
void boot_reply_entry(void)
{
    status_line("SET PATH...");
    if (!fnraw_set_device_path_from_reply(DEVICE_SLOT, host, MODE_READ,
                                          path, FN_REPLY)) {
        fail("EPATH");
        return;
    }
    boot_mount_swap();
}
