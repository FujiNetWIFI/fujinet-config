/* st_boot.c -- mount into device slot 0 and hand the console to the loader.
 *
 * Not a dispatcher state: booting is called from the browser and only ever
 * RETURNS on failure -- success ends in fuji_nes_boot(), which jumps into the
 * cartridge's loader ROM. The loader copies the image into the SRAMs and
 * cold-starts it; getting back to CONFIG afterwards is a power cycle, which
 * the cartridge's M2 watchdog turns into a reload of CONFIG.
 *
 * The whole time is spent on a LOADING screen of its own: the file's name,
 * the host it comes from, and a bar the cartridge's own progress fills.
 */

#include "fujidisp.h"
#include "fujiraw.h"
#include "sfx.h"
#include "state.h"

#define NAME_ROW   5
#define HOST_ROW   7
#define BAR_BOX_T  10
#define BAR_ROW    11
#define BAR_L      4            /* the box's left edge; the bar starts one in */
#define BAR_CELLS  22
#define PCT_ROW    14
#define PCT_COL    14

static void boot_pct(unsigned char pct)
{
    disp_bar((unsigned char)(BAR_L + 1), BAR_ROW, BAR_CELLS, pct, 100);
    disp_at(PCT_COL, PCT_ROW, "    ");
    disp_at_u16(PCT_COL, PCT_ROW, pct);
    disp_at((unsigned char)(PCT_COL + (pct >= 100 ? 3 : pct >= 10 ? 2 : 1)),
            PCT_ROW, "%");
}

/* The screen, with the name painted out of the reply window: drawing runs no
 * transaction, so the window still holds it for the SET_DEVICE_PATH after. */
static void boot_screen(volatile unsigned char *name)
{
    unsigned char i;

    draw_frame("LOADING");
    for (i = 0; i < IN_W && name[i] != 0; i++)
        disp_char((unsigned char)(IN_L + i), NAME_ROW, (char)name[i]);
    disp_at(IN_L, HOST_ROW, "FROM HOST");
    disp_at_u16(IN_L + 10, HOST_ROW, (unsigned int)(host + 1));
    disp_box(BAR_L, BAR_BOX_T, (unsigned char)(BAR_L + BAR_CELLS + 1),
             (unsigned char)(BAR_BOX_T + 2), NULL);
    disp_bar_reset();
    boot_pct(0);
}

void boot_mount_swap(void)
{
    unsigned char pct = 0;

    status_line("MOUNTING...");
    if (!fuji_mount_disk_image(DEVICE_SLOT, MODE_READ)) {
        fail("MOUNT");
        return;
    }

    /* The image arrives asynchronously, pushed to the cartridge while the
     * console keeps running; these three bytes are the cart's progress. */
    status_line("LOADING...");
    for (;;) {
        unsigned char st = fuji_nes_boot_state();
        unsigned char now;

        if (st == FUJI_NES_BOOT_READY)
            break;
        if (st == FUJI_NES_BOOT_FAILED) {
            disp_row_clear(STATUS_ROW);
            disp_at(IN_L, STATUS_ROW, "?LOAD ERROR");
            disp_at_hex8(IN_L + 12, STATUS_ROW, fuji_nes_boot_error());
            sfx_beep();
            return;
        }
        now = fuji_nes_boot_percent();
        if (now > 100)
            now = 100;
        if (now != pct) {
            pct = now;
            boot_pct(pct);
        }
    }

    boot_pct(100);
    status_line("BOOTING");
    disp_at(IN_L, LEGEND_ROW, "OK");
    sfx_accept();
    wait_frames(10);            /* let the screen catch up */
    fuji_nes_boot();            /* does not return */
}

/* Boot the entry whose FULL-width name is sitting in the reply window (the
 * caller just re-read it there): the path prefix comes from RAM, the filename
 * streams cartridge-to-cartridge without landing in console RAM. */
void boot_reply_entry(void)
{
    boot_screen(FN_REPLY);
    status_line("SETTING PATH...");
    if (!fnraw_set_device_path_from_reply(DEVICE_SLOT, host, MODE_READ,
                                          path, FN_REPLY)) {
        fail("PATH");
        return;
    }
    boot_mount_swap();
}
