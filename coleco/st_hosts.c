/* st_hosts.c -- the host slots screen, and (in copy livery) the destination
 * picker that replaces src/destination_host_slot.c, the way intv/st_hosts.bas
 * does it.
 *
 * Slot names are drawn straight out of the reply window; nothing is cached
 * but a one-byte emptiness mask, gathered while drawing, that keeps fire from
 * mounting an empty slot. Renaming re-reads the slots and streams all eight
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

    draw_frame(copy_mode ? "COPY TO WHICH HOST?" : "SELECT A HOST");
    if (copy_mode) {
        status_line("FIRE PICK DESTINATION");
        legend_line("* CANCEL COPY  1-8 JUMP");
    } else {
        status_line("FIRE OPEN  9 RENAME  0 LOBBY");
        legend_line("# INFO  * WIFI  1-8 JUMP");
    }

    if (!FUJICALL(FUJICMD_READ_HOST_SLOTS)) {
        fail("EHOSTS");
        nrows = 0;
        return;
    }

    hosts_mask = 0;
    for (i = 0; i < HOST_SLOTS; i++) {
        volatile unsigned char *name = FN_REPLY + (unsigned int)i * HOST_STRIDE;
        unsigned char row = (unsigned char)(LIST_TOP + i);
        unsigned char col;

        disp_row_clear(row);
        disp_at_u16(1, row, (unsigned int)(i + 1));
        if (*name == 0) {
            disp_at(3, row, "(empty)");
            continue;
        }
        /* Straight out of the reply window, a character at a time -- there is
         * nowhere in RAM to put it and no reason to. */
        for (col = 0; col < HOST_STRIDE && name[col] != 0; col++)
            disp_char((unsigned char)(3 + col), row, (char)name[col]);
        hosts_mask |= (unsigned char)(1 << i);
    }
    /* Empty slots stay navigable: 9 renames one into existence. */
    nrows = HOST_SLOTS;
    at_end = 1;
    if (cur >= nrows)
        cur = 0;
    disp_row_invert((unsigned char)(LIST_TOP + cur), true);
}

static void enter_host(void)
{
    if (!(hosts_mask & (unsigned char)(1 << cur))) {
        status_line(copy_mode ? "EMPTY SLOT" : "EMPTY SLOT: 9 SETS A NAME");
        return;
    }

    status_line("MOUNTING HOST...");
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

    if (fn_edit("EDIT HOST NAME", HOST_STRIDE - 1)) {
        if (!fnraw_write_host_slot(cur, fn_entry)) {
            fail("EWRITE");
            wait_frames(90);
        }
    }
    hosts_draw();               /* the editor owned the screen either way */
}

void st_hosts(void)
{
    hosts_draw();
    while (state == ST_HOSTS) {
        unsigned char ev = in_read();

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
            /* Keypad 1-8 jumps straight to a slot and opens it. */
            if (ev >= IN_KEY0 + 1 && ev <= IN_KEY0 + HOST_SLOTS) {
                disp_row_invert((unsigned char)(LIST_TOP + cur), false);
                cur = (unsigned char)(ev - IN_KEY0 - 1);
                disp_row_invert((unsigned char)(LIST_TOP + cur), true);
                snd_click();
                enter_host();
            }
            break;
        }
    }
}
