/* st_hosts.c -- the host slots screen, and (in copy livery) the destination
 * picker that replaces src/destination_host_slot.c, the way intv/st_hosts.bas
 * does it.
 *
 * Slot names are drawn straight out of the reply window; nothing is cached
 * but a one-byte emptiness mask, gathered while drawing, that keeps fire from
 * mounting an empty slot. Renaming (Pause menu) re-reads the slots and streams all eight
 * back with the edited one substituted -- see fnraw_write_host_slot.
 */

#include "fujidisp.h"
#include "fujisnd.h"
#include "fujiin.h"
#include "fujiedit.h"
#include "fujiraw.h"
#include "state.h"

static void hosts_draw(void)
{
    unsigned char i;

    draw_frame(copy_mode ? "COPY" : "HOSTS");
    legend_line(copy_mode ? "1 PICK  2 CANCEL COPY" : "1 OPEN  2 WIFI  PAUSE MENU");

    if (!FUJICALL(FUJICMD_READ_HOST_SLOTS)) {
        fail("EHOSTS");
        nrows = 0;
        return;
    }

    hosts_mask = 0;
    for (i = 0; i < HOST_SLOTS; i++) {
        volatile unsigned char *name = FN_REPLY + (unsigned int)i * HOST_STRIDE;
        unsigned char row = LIST_Y(i);
        unsigned char col;

        disp_at_u16(1, row, (unsigned int)(i + 1));
        if (*name == 0) {
            disp_at(3, row, "(EMPTY)");
            continue;
        }
        /* Straight out of the reply window, a character at a time -- there is
         * nowhere in RAM to put it and no reason to. */
        for (col = 0; col < NAMELEN - 2 && name[col] != 0; col++)
            disp_char((unsigned char)(3 + col), row, (char)name[col]);
        hosts_mask |= (unsigned char)(1 << i);
    }
    /* Empty slots stay navigable: RENAME SLOT names one into existence. */
    nrows = HOST_SLOTS;
    at_end = 1;
    if (cur >= nrows)
        cur = 0;
    list_select(cur, true);
    status_line(copy_mode ? "COPY TO WHICH HOST?" : "SELECT A HOST.");
}

static void enter_host(void)
{
    if (!(hosts_mask & (unsigned char)(1 << cur))) {
        snd_play(SND_ERROR);
        status_line(copy_mode ? "THAT SLOT IS EMPTY." : "EMPTY. PAUSE, RENAME SLOT.");
        return;
    }

    snd_play(SND_OK);
    status_now("MOUNTING HOST...");
    if (!fuji_mount_host_slot(cur)) {
        fail("EHOST");
        return;
    }

    host = cur;
    path[0] = '/';
    path[1] = 0;
    fn_entry[0] = 0;            /* the filter starts clear -- see constants.h */
    top = 0;
    if (!dir_open()) {
        fail("EOPEN");
        return;
    }
    cur = 0;
    state = ST_FILES;
}

static void rename_host(void)
{
    volatile unsigned char *name;
    unsigned char i;

    /* Fetch the current name into the edit buffer. The write below re-reads
     * the slots for itself, so nothing has to survive the editor. */
    if (!FUJICALL(FUJICMD_READ_HOST_SLOTS)) {
        fail("EHOSTS");
        return;
    }
    name = FN_REPLY + (unsigned int)cur * HOST_STRIDE;
    for (i = 0; i < HOST_STRIDE - 1 && name[i] != 0; i++)
        fn_entry[i] = (char)name[i];
    fn_entry[i] = 0;

    if (fn_edit("HOST NAME", "INPUT THE HOST NAME.", HOST_STRIDE - 1)) {
        status_now("SAVING...");
        if (!fnraw_write_host_slot(cur, fn_entry)) {
            fail("EWRITE");
            wait_frames(90);
        }
    }
    hosts_draw();               /* the editor owned the screen either way */
}

static const char *const menu_names[] = {
    "ADAPTER INFO", "GAME LOBBY", "RENAME SLOT", "WIFI SETUP"
};
static const unsigned char menu_events[] = {
    IN_KEYHASH, IN_KEY0, IN_KEY0 + 9, IN_KEYSTAR
};

void st_hosts(void)
{
    hosts_draw();
    while (state == ST_HOSTS) {
        unsigned char ev = in_read();

        if (ev == IN_MENU && !copy_mode)
            ev = menu_pick(menu_names, menu_events, 4);

        switch (ev) {
        case IN_UP:
            bar_move(-1);
            break;
        case IN_DOWN:
            bar_move(1);
            break;
        case IN_FIRE:
            enter_host();
            break;
        case IN_KEYSTAR:
            snd_play(SND_BACK);
            if (copy_mode) {
                copy_cancel();
                if (state == ST_HOSTS)
                    hosts_draw();   /* cancel failed to reopen the source */
            } else {
                state = ST_SET_WIFI;
            }
            break;
        case IN_KEYHASH:
            if (!copy_mode)
                state = ST_INFO;
            break;
        case IN_KEY0:
            if (!copy_mode)
                state = ST_LOBBY;
            break;
        case IN_KEY0 + 9:
            if (!copy_mode)
                rename_host();
            break;
        default:
            break;
        }
    }
}
