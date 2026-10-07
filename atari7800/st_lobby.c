/* st_lobby.c -- PLAY GAME LOBBY on the hosts screen: mount and boot the
 * FujiNet Game Lobby without walking the browser. It uses the host slot that
 * already holds LOBBY_HOST (any case); if none does, it writes it into the
 * first empty slot, or into slot 8 when all eight are taken.
 */

#include "fujidisp.h"
#include "fujiraw.h"
#include "state.h"

static const char lobby_host[] = LOBBY_HOST;

static bool slot_matches(volatile unsigned char *name)
{
    unsigned char j;

    for (j = 0; lobby_host[j] != 0; j++) {
        unsigned char c = name[j];

        if (c >= 'A' && c <= 'Z')
            c = (unsigned char)(c + 32);
        if (c != (unsigned char)lobby_host[j])
            return false;
    }
    return name[j] == 0;
}

/* On success this never comes back -- boot_mount_swap() hands over. */
static void lobby_go(void)
{
    volatile unsigned char *r;
    unsigned char slot = 0xFF, empty = 0xFF;
    unsigned char i;

    status_line("FINDING LOBBY HOST...");
    if (!FUJICALL(FUJICMD_READ_HOST_SLOTS)) {
        fail("HOSTS");
        return;
    }
    for (i = 0; i < HOST_SLOTS; i++) {
        r = FN_REPLY + (unsigned int)i * HOST_STRIDE;
        if (slot_matches(r)) {
            slot = i;
            break;
        }
        if (*r == 0 && empty == 0xFF)
            empty = i;
    }
    if (slot == 0xFF) {
        slot = empty != 0xFF ? empty : HOST_SLOTS - 1;
        status_line("ADDING LOBBY HOST...");
        if (!fnraw_write_host_slot(slot, lobby_host)) {
            fail("WRITE");
            return;
        }
    }

    status_line("MOUNTING HOST...");
    if (!fuji_mount_host_slot(slot)) {
        fail("HOST");
        return;
    }
    host = slot;

    boot_screen((volatile unsigned char *)"GAME LOBBY");
    status_line("SETTING PATH...");
    if (!fnraw_set_device_path(DEVICE_SLOT, slot, MODE_READ, LOBBY_PATH)) {
        fail("PATH");
        return;
    }
    boot_mount_swap();
}

void st_lobby(void)
{
    lobby_go();
    wait_frames(120);           /* whatever failed is on the status line */
    cur = HOST_SLOTS;
    state = ST_HOSTS;
}
