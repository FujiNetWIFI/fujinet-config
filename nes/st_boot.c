/* st_boot.c -- mount into device slot 0 and hand the console to the loader.
 *
 * Not a dispatcher state: booting is called from the browser (and, for the
 * Game Lobby, from the host slots) and only ever RETURNS on failure --
 * success ends in fuji_nes_boot(), which jumps into the cartridge's loader
 * ROM. The loader copies the image into the SRAMs and
 * cold-starts it; getting back to CONFIG afterwards is a power cycle, which
 * the cartridge's M2 watchdog turns into a reload of CONFIG.
 *
 * The whole time is spent on a LOADING screen of its own: the file's name,
 * the host it comes from, a bar the cartridge's own progress fills, and the
 * bytes received so far against the image's size.
 */

#include <stdlib.h>
#include <string.h>

#include <fujinet-bus-nes.h>

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
#define BYTES_ROW  16

/* The FujiNet answers MOUNT_IMAGE only once the whole image is on the cart,
 * and the cart gives it 60 seconds; wait a little longer than that so its
 * own timeout is the one that surfaces. */
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

void boot_mount_swap(void)
{
    unsigned char pct = 0, want, st;
    bool acked = false;
    unsigned int frames = 0;

    /* Depending on the FujiNet, the image is pushed to the cartridge before
     * MOUNT_IMAGE is answered (so progress is only visible while that reply
     * is still outstanding) or after it. Either way: start the mount, then
     * watch the cart's boot registers -- and the reply -- until the image
     * is READY. The cart clears those registers when the mount starts. */
    status_line("LOADING...");
    want = fnraw_mount_start(DEVICE_SLOT, MODE_READ);
    for (;;) {
        unsigned char now;

        st = fuji_nes_boot_state();
        if (st == FUJI_NES_BOOT_FAILED) {
            load_error(fuji_nes_boot_error());
            return;
        }
        if (!acked && FN_ACKSEQ == want) {
            acked = true;
            if (!fnraw_reply_ok()) {
                fail("MOUNT");
                return;
            }
        }
        if (acked && st == FUJI_NES_BOOT_READY)
            break;
        if (++frames > MOUNT_FRAMES) {
            load_error(st);
            return;
        }
        now = fuji_nes_boot_percent();
        if (now > 100)
            now = 100;
        if (now != pct) {
            pct = now;
            boot_pct(pct);
        }
        boot_bytes(fuji_nes_boot_got(), fuji_nes_boot_total());
        wait_frames(1);
    }

    boot_pct(100);
    boot_bytes(fuji_nes_boot_total(), fuji_nes_boot_total());
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

/* Boot the Game Lobby off `host`, which the caller has already pointed at
 * LOBBY_HOST and mounted. The whole path is the RAM prefix; no name follows. */
void boot_lobby(void)
{
    boot_screen((volatile unsigned char *)"LOBBY.NES");
    status_line("SETTING PATH...");
    if (!fnraw_set_device_path_from_reply(DEVICE_SLOT, host, MODE_READ,
                                          LOBBY_PATH,
                                          (volatile unsigned char *)"")) {
        fail("PATH");
        return;
    }
    boot_mount_swap();
}
