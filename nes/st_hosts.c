/* st_hosts.c -- the host slots screen.
 *
 * Slot names are drawn straight out of the reply window; nothing is cached
 * but a one-byte emptiness mask, gathered while drawing, that keeps A from
 * mounting an empty slot. Renaming re-reads the slots and streams all eight
 * back with the edited one substituted -- see fnraw_write_host_slot.
 *
 * A ninth row under the slots boots the FujiNet Game Lobby. It needs a slot
 * holding LOBBY_HOST: the one that already does, else the first empty one,
 * else slot 8 -- picked while drawing, so it costs no extra transaction.
 */

#include "fujidisp.h"
#include "fujiin.h"
#include "fujiedit.h"
#include "fujiraw.h"
#include "sfx.h"
#include "state.h"

#define LOBBY_ROW   HOST_SLOTS  /* the bar index of PLAY GAME LOBBY */

static unsigned char lobby_slot;    /* where the lobby host is, or will go */
static bool lobby_there;            /* lobby_slot already holds LOBBY_HOST */

/* Case-insensitive: a hand-typed EC.TNFS.IO is the same host. */
static bool is_lobby_host(volatile unsigned char *name)
{
    static const char want[] = LOBBY_HOST;
    unsigned char i;

    for (i = 0; i < sizeof want; i++) {
        unsigned char c = name[i];

        if (c >= 'A' && c <= 'Z')
            c = (unsigned char)(c + ('a' - 'A'));
        if (c != (unsigned char)want[i])
            return false;
    }
    return true;
}

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
    lobby_slot = 0xFF;
    lobby_there = false;
    for (i = 0; i < HOST_SLOTS; i++) {
        volatile unsigned char *name = FN_REPLY + (unsigned int)i * HOST_STRIDE;
        unsigned char row = (unsigned char)(LIST_TOP + i);
        unsigned char col;

        disp_row_clear(row);
        disp_at_u16(3, row, (unsigned int)(i + 1));
        if (*name == 0) {
            disp_at(5, row, "(EMPTY)");
            if (lobby_slot == 0xFF)
                lobby_slot = i;
            continue;
        }
        if (!lobby_there && is_lobby_host(name)) {
            lobby_slot = i;
            lobby_there = true;
        }
        for (col = 0; col <= IN_R - 5 && name[col] != 0; col++)
            disp_char((unsigned char)(5 + col), row, (char)name[col]);
        hosts_mask |= (unsigned char)(1 << i);
    }
    if (lobby_slot == 0xFF)
        lobby_slot = HOST_SLOTS - 1;
    disp_row_clear(LIST_TOP + LOBBY_ROW);
    disp_at(5, LIST_TOP + LOBBY_ROW, "PLAY GAME LOBBY");
    nrows = HOST_SLOTS + 1;
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

static void enter_lobby(void)
{
    if (!lobby_there) {
        status_line("ADDING LOBBY HOST...");
        if (!fnraw_write_host_slot(lobby_slot, LOBBY_HOST)) {
            fail("WRITE");
            return;
        }
        lobby_there = true;
        hosts_mask |= (unsigned char)(1 << lobby_slot);
        disp_row_clear((unsigned char)(LIST_TOP + lobby_slot));
        disp_at_u16(3, (unsigned char)(LIST_TOP + lobby_slot),
                    (unsigned int)(lobby_slot + 1));
        disp_at(5, (unsigned char)(LIST_TOP + lobby_slot), LOBBY_HOST);
    }

    status_line("MOUNTING HOST...");
    if (!fuji_mount_host_slot(lobby_slot)) {
        fail("HOST");
        return;
    }

    host = lobby_slot;
    boot_lobby();               /* only failure comes back */
    wait_frames(120);
    hosts_draw();               /* shows the slot it may have claimed */
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
            if (cur == LOBBY_ROW)
                enter_lobby();
            else
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
