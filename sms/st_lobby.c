/* st_lobby.c -- GAME LOBBY on the hosts screen's Pause menu: mount and boot the FujiNet Game
 * Lobby ROM, so a user reaches the lobby without walking the browser. Same
 * policy as intv/st_lobby.bas: case-insensitively find ec.tnfs.io among the
 * eight slots, or claim the last one if absent.
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

/* On success this never comes back -- boot_mount_swap() takes the swap. */
static void lobby_go(void)
{
    volatile unsigned char *r;
    unsigned char slot = 0xFF;
    unsigned char i;

    status_now("FINDING LOBBY HOST...");
    if (!FUJICALL(FUJICMD_READ_HOST_SLOTS)) {
        fail("EHOSTS");
        return;
    }
    for (i = 0; i < HOST_SLOTS; i++) {
        r = FN_REPLY + (unsigned int)i * HOST_STRIDE;
        if (slot_matches(r)) {
            slot = i;
            break;
        }
    }
    if (slot == 0xFF) {
        slot = HOST_SLOTS - 1;
        if (!fnraw_write_host_slot(slot, lobby_host)) {
            fail("EWRITE");
            return;
        }
    }

    status_now("MOUNTING LOBBY HOST...");
    if (!fuji_mount_host_slot(slot)) {
        fail("EHOST");
        return;
    }
    host = slot;

    boot_begin((const volatile unsigned char *)LOBBY_PATH);
    if (!fnraw_set_device_path(DEVICE_SLOT, slot, MODE_READ, LOBBY_PATH)) {
        boot_cancel();
        fail("EPATH");
        return;
    }
    boot_mount_swap();
}

void st_lobby(void)
{
    lobby_go();
    wait_frames(120);           /* whatever failed is in the message window */
    state = ST_HOSTS;
}
