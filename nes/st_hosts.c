/* st_hosts.c -- the host slots screen.
 *
 * Slot names are drawn straight out of the reply window; nothing is cached
 * but a one-byte emptiness mask, gathered while drawing, that keeps A from
 * mounting an empty slot. Renaming re-reads the slots and streams all eight
 * back with the edited one substituted -- see fnraw_write_host_slot.
 */

#include "fujidisp.h"
#include "fujiin.h"
#include "fujiedit.h"
#include "fujiraw.h"
#include "sfx.h"
#include "state.h"

static void hosts_draw(void)
{
    unsigned char i;

    draw_frame("HOST SLOTS");
    status_line("A--OPEN  SEL--RENAME");
    legend_line("B--WIFI  START--INFO");

    if (!FUJICALL(FUJICMD_READ_HOST_SLOTS)) {
        fail("HOSTS");
        nrows = 0;
        return;
    }

    hosts_mask = 0;
    for (i = 0; i < HOST_SLOTS; i++) {
        volatile unsigned char *name = FN_REPLY + (unsigned int)i * HOST_STRIDE;
        unsigned char row = (unsigned char)(LIST_TOP + i);
        unsigned char col;

        disp_row_clear(row);
        disp_at_u16(3, row, (unsigned int)(i + 1));
        if (*name == 0) {
            disp_at(5, row, "(EMPTY)");
            continue;
        }
        for (col = 0; col <= IN_R - 5 && name[col] != 0; col++)
            disp_char((unsigned char)(5 + col), row, (char)name[col]);
        hosts_mask |= (unsigned char)(1 << i);
    }
    nrows = HOST_SLOTS;
    at_end = 1;
    if (cur >= nrows)
        cur = 0;
    disp_row_invert((unsigned char)(LIST_TOP + cur), true);
}

static void enter_host(void)
{
    if (!(hosts_mask & (unsigned char)(1 << cur))) {
        status_line("?EMPTY SLOT  SEL--RENAME");
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
            enter_host();
            break;
        case IN_BACK:
            state = ST_SET_WIFI;
            break;
        case IN_START:
            state = ST_INFO;
            break;
        case IN_SELECT:
            rename_host();
            break;
        }
    }
}
