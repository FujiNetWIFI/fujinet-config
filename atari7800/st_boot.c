/* st_boot.c -- mount into device slot 0 and hand the console to the loader.
 *
 * Not a dispatcher state: booting is called from the browser (and, for the
 * Game Lobby, from the host slots) and only ever RETURNS on failure, or once
 * the High Score Cart ROM has been installed instead of booted -- success
 * ends in fuji_a7800_boot(), which jumps into the cartridge's loader at
 * $0600. The loader copies the image into the cart's SRAM and starts it,
 * through the console's own BIOS when it would accept it.
 *
 * The whole time is spent on a LOADING screen of its own: the file's name,
 * the host it comes from, a bar the cartridge's own progress fills, and the
 * bytes received so far against the image's size.
 */

#include <stdlib.h>
#include <string.h>

#include "fujidisp.h"
#include "fujiin.h"
#include "fujiraw.h"
#include "sfx.h"
#include "state.h"

#define NAME_ROW   3
#define HOST_ROW   5
#define BAR_BOX_T  8
#define BAR_ROW    9
#define BAR_L      4            /* the box's left edge; the bar starts one in */
#define BAR_CELLS  22
#define PCT_ROW    12
#define PCT_COL    14
#define BYTES_ROW  14

/* The cart gives MOUNT_IMAGE 60 seconds; wait a little longer so its own
 * timeout is the one that surfaces. */
#define MOUNT_FRAMES 3900u

/* "got/total BYTES": the total and the layout are fixed once the image
 * stream opens, so each update rewrites only the right-aligned count. */
static char tot_txt[9];
static unsigned char tot_len, got_col;
static unsigned long shown_tot, shown_got;

static void boot_bytes(unsigned long got, unsigned long tot)
{
    char txt[9];
    unsigned char n;

    if (tot != shown_tot) {
        shown_tot = tot;
        shown_got = ~0ul;
        disp_row_clear(BYTES_ROW);
        if (tot == 0)
            return;
        ultoa(tot, tot_txt, 10);
        tot_len = (unsigned char)strlen(tot_txt);
        got_col = (unsigned char)((DISP_COLS - (tot_len * 2 + 7)) / 2);
        disp_at((unsigned char)(got_col + tot_len), BYTES_ROW, "/");
        disp_at((unsigned char)(got_col + tot_len + 1), BYTES_ROW, tot_txt);
        disp_at((unsigned char)(got_col + tot_len * 2 + 1), BYTES_ROW,
                " BYTES");
    }
    if (tot == 0 || got == shown_got)
        return;
    shown_got = got;
    ultoa(got, txt, 10);
    n = (unsigned char)strlen(txt);
    if (n > tot_len)
        n = tot_len;
    disp_at((unsigned char)(got_col + tot_len - n), BYTES_ROW, txt);
    while (n < tot_len)
        disp_char((unsigned char)(got_col + tot_len - 1 - n++), BYTES_ROW,
                  ' ');
}

static void boot_pct(unsigned char pct)
{
    disp_bar((unsigned char)(BAR_L + 1), BAR_ROW, BAR_CELLS, pct, 100);
    disp_at(PCT_COL, PCT_ROW, "    ");
    disp_at_u16(PCT_COL, PCT_ROW, pct);
    disp_at((unsigned char)(PCT_COL + (pct >= 100 ? 3 : pct >= 10 ? 2 : 1)),
            PCT_ROW, "%");
}

/* The screen, with the name painted out of the reply window: drawing runs no
 * transaction, so the window still holds it for the SET_DEVICE_FULLPATH
 * after. */
void boot_screen(volatile unsigned char *name)
{
    draw_frame("LOADING");
    disp_text(IN_L, NAME_ROW, name, IN_W);
    disp_at(IN_L, HOST_ROW, "FROM HOST");
    disp_at_u16(IN_L + 10, HOST_ROW, (unsigned int)(host + 1));
    disp_box(BAR_L, BAR_BOX_T, (unsigned char)(BAR_L + BAR_CELLS + 1),
             (unsigned char)(BAR_BOX_T + 2), NULL);
    disp_bar_reset();
    boot_pct(0);
    shown_tot = 0;
    disp_row_clear(BYTES_ROW);
}

static void load_error(unsigned char code)
{
    disp_row_clear(STATUS_ROW);
    disp_at(IN_L, STATUS_ROW, "?LOAD ERROR");
    disp_at_hex8(IN_L + 12, STATUS_ROW, code);
    sfx_beep();
}

/* The pushed file is the High Score Cart ROM, which is no game to boot:
 * offer to install it, after which every game gets the HSC. */
static void hsc_offer(void)
{
    unsigned char i;

    status_line("HIGH SCORE CART ROM 2--BACK");
    legend_line("1--USE AS HIGH SCORE CART");
    for (;;) {
        unsigned char ev = in_read();

        if (ev == IN_BACK)
            return;
        if (ev == IN_FIRE)
            break;
    }
    fuji_a7800_hsc_op(FUJI_A7800_HSCOP_INSTALL);
    for (i = 0; i < 60; i++) {
        wait_frames(1);
        if (fuji_a7800_hsc() & FUJI_A7800_HSC_ROM) {
            status_line("HIGH SCORE CART INSTALLED");
            legend_line("EVERY GAME GETS IT NOW");
            sfx_accept();
            return;
        }
    }
    load_error(fuji_a7800_boot_error());
}

void boot_mount_swap(void)
{
    unsigned char pct = 0, want, st;
    bool acked = false;
    unsigned int frames = 0;

    /* Depending on the FujiNet, the image is pushed to the cartridge before
     * MOUNT_IMAGE is answered (so progress is only visible while that reply
     * is still outstanding) or after it. Either way: start the mount, then
     * watch the cart's boot registers -- and the reply -- until the image
     * is READY. The cart clears those registers when it starts the mount,
     * which may be after our first look, so FAILED counts only once the
     * reply is in. */
    status_line("LOADING...");
    want = fnraw_mount_start(DEVICE_SLOT, MODE_READ);
    for (;;) {
        unsigned char now;
        bool replied = !acked && FN_ACKSEQ == want;

        st = fuji_a7800_boot_state();
        if (replied)
            acked = true;
        if (acked && st == FUJI_A7800_BOOT_FAILED) {
            load_error(fuji_a7800_boot_error());
            return;
        }
        if (replied && !fnraw_reply_ok()) {
            fail("MOUNT");
            return;
        }
        if (acked && st == FUJI_A7800_BOOT_READY)
            break;
        if (++frames > MOUNT_FRAMES) {
            load_error(st);
            return;
        }
        now = fuji_a7800_boot_percent();
        if (now > 100)
            now = 100;
        if (now != pct) {
            pct = now;
            boot_pct(pct);
        }
        boot_bytes(fuji_a7800_boot_got(), fuji_a7800_boot_total());
        wait_frames(1);
    }

    boot_pct(100);
    boot_bytes(fuji_a7800_boot_total(), fuji_a7800_boot_total());
    if (fuji_a7800_staged() & FUJI_A7800_STAGED_HSCROM) {
        hsc_offer();
        return;
    }
    status_line("BOOTING");
    disp_at(IN_L, LEGEND_ROW, "OK");
    sfx_accept();
    wait_frames(10);            /* let the screen catch up */
    fuji_a7800_boot();          /* does not return */
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
