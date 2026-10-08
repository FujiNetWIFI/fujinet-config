/* st_boot.c -- mount into device slot 0 and take the ROM swap.
 *
 * Not a dispatcher state: booting is called from wherever the choice was made
 * (the browser, the lobby) and only ever RETURNS on failure -- success ends
 * in fuji_sms_boot(), which hands the console to the cart's loader page;
 * the loader copies the image into the SRAM and starts it. A power cycle
 * brings CONFIG back.
 */

#include "fujidisp.h"
#include "fujisnd.h"
#include "fujiin.h"
#include "fujiraw.h"
#include "state.h"

void boot_mount_swap(void)
{
    unsigned char pct = 0xFF;

    status_now("MOUNTING...");
    if (!fuji_mount_disk_image(DEVICE_SLOT, MODE_READ)) {
        fail("EMOUNT");
        return;
    }

    /* The image arrives asynchronously, pushed to the cartridge while the
     * console keeps running; these three bytes are the cart's progress. */
    status_now("LOADING");
    for (;;) {
        unsigned char st = fuji_sms_boot_state();

        in_frames();            /* keeps the cursor and sound ticking */
        if (st == FUJI_SMS_BOOT_READY)
            break;
        if (st == FUJI_SMS_BOOT_FAILED) {
            fail_code("ELOAD", fuji_sms_boot_error());
            return;
        }
        if (fuji_sms_boot_percent() != pct) {
            pct = fuji_sms_boot_percent();
            disp_at(24, MSG_LINE1, "   %");
            disp_at_u16(pct < 10 ? 26 : pct < 100 ? 25 : 24, MSG_LINE1, pct);
        }
    }

    /* The chime, then the scene fades out the way Phantasy Star leaves one,
     * and the PSG goes quiet so nothing carries into the game. */
    msg_put(MSG_LINE1, "BOOTING");
    snd_play(SND_READY);
    while (snd_busy())
        in_frames();
    fade_out();
    snd_init();
    fuji_sms_boot();            /* does not return */
}

/* Boot the entry whose FULL-width name is sitting in the reply window (the
 * caller just re-read it there): the path prefix comes from RAM, the filename
 * streams cartridge-to-cartridge without ever landing in console RAM. */
void boot_reply_entry(void)
{
    status_now("SET PATH...");
    if (!fnraw_set_device_path_from_reply(DEVICE_SLOT, host, MODE_READ,
                                          path, FN_REPLY)) {
        fail("EPATH");
        return;
    }
    boot_mount_swap();
}
