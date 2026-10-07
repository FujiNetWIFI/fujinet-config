/* st_hosts.c -- the host slots screen, and (in copy livery) the destination
 * picker for a host-to-host copy.
 *
 * Slot names are drawn straight out of the reply window; nothing is cached
 * but a one-byte emptiness mask, gathered while drawing, that keeps button 1
 * from mounting an empty slot. Renaming re-reads the slots and streams all
 * eight back with the edited one substituted -- see fnraw_write_host_slot.
 * A ninth row under the slots boots the FujiNet Game Lobby (st_lobby.c).
 */

#include "fujidisp.h"
#include "fujiin.h"
#include "fujiedit.h"
#include "fujiraw.h"
#include "sfx.h"
#include "state.h"

#define LOBBY_ROW   HOST_SLOTS  /* the bar index of PLAY GAME LOBBY */

static void hosts_draw(void)
{
    unsigned char i;

    draw_frame(copy_mode ? "COPY TO WHICH HOST?" : "HOST SLOTS");
    if (copy_mode) {
        status_line("1--PICK DESTINATION");
        legend_line("2--CANCEL COPY");
    } else {
        status_line("1--OPEN  SEL--RENAME");
        legend_line("2--WIFI  PAUSE--INFO");
    }

    if (!FUJICALL(FUJICMD_READ_HOST_SLOTS)) {
        fail("HOSTS");
        nrows = 0;
        return;
    }

    hosts_mask = 0;
    for (i = 0; i < HOST_SLOTS; i++) {
        volatile unsigned char *name = FN_REPLY + (unsigned int)i * HOST_STRIDE;
        unsigned char row = (unsigned char)(LIST_TOP + i);

        disp_row_clear(row);
        disp_at_u16(3, row, (unsigned int)(i + 1));
        if (*name == 0) {
            disp_at(5, row, "(EMPTY)");
            continue;
        }
        disp_text(5, row, name, IN_R - 4);
        hosts_mask |= (unsigned char)(1 << i);
    }
    nrows = HOST_SLOTS;
    if (!copy_mode) {
        disp_row_clear(LIST_TOP + LOBBY_ROW);
        disp_at(5, LIST_TOP + LOBBY_ROW, "PLAY GAME LOBBY");
        nrows = HOST_SLOTS + 1;
    }
    at_end = 1;
    if (cur >= nrows)
        cur = 0;
    disp_row_invert((unsigned char)(LIST_TOP + cur), true);
}

static void enter_host(void)
{
    if (!(hosts_mask & (unsigned char)(1 << cur))) {
        status_line(copy_mode ? "?EMPTY SLOT" : "?EMPTY SLOT  SEL--RENAME");
        sfx_beep();
        return;
    }

    status_line("MOUNTING HOST...");
    if (!fuji_mount_host_slot(cur)) {
        fail("HOST");
        return;
    }

    host = cur;
    path[0] = '/';
    path[1] = 0;
    fn_entry[0] = 0;            /* the filter starts clear */
    top = 0;
    if (!dir_open()) {
        fail("OPEN");
        return;
    }
    cur = 0;
    sfx_accept();
    state = ST_FILES;
}

static void rename_host(void)
{
    volatile unsigned char *name;
    unsigned char i;

    if (cur == LOBBY_ROW) {
        sfx_beep();
        return;
    }
    if (!FUJICALL(FUJICMD_READ_HOST_SLOTS)) {
        fail("HOSTS");
        return;
    }
    name = FN_REPLY + (unsigned int)cur * HOST_STRIDE;
    for (i = 0; i < HOST_STRIDE - 1 && name[i] != 0; i++)
        fn_entry[i] = (char)name[i];
    fn_entry[i] = 0;

    if (fn_edit("HOST NAME", HOST_STRIDE - 1)) {
        if (!fnraw_write_host_slot(cur, fn_entry)) {
            fail("WRITE");
            wait_frames(90);
        }
    }
    hosts_draw();
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
            if (cur == LOBBY_ROW && !copy_mode)
                state = ST_LOBBY;
            else
                enter_host();
            break;
        case IN_BACK:
            if (copy_mode) {
                copy_cancel();
                if (state == ST_HOSTS)
                    hosts_draw();   /* the source could not be reopened */
            } else {
                state = ST_SET_WIFI;
            }
            break;
        case IN_START:
            if (!copy_mode)
                state = ST_INFO;
            break;
        case IN_SELECT:
            if (!copy_mode)
                rename_host();
            break;
        }
    }
}
