#ifdef BUILD_ATARI
#include <atari.h>
#include <conio.h>
#include <peekpoke.h>
#include <string.h>
#include <fujinet-network.h>
#include "bbs_terminal.h"
#include "atari_screen.h"
#include "../globals.h"

/* Raw ATASCII: no CR/LF translation and no Telnet negotiation. */
#define BBS_URI "N1:TCP://ataribbs.ddns.net:6502"
#define WIDTH 40
#define HEIGHT 24
#define CELLS (WIDTH * HEIGHT)
static unsigned char terminal_dlist[32];
/* Keep each read short so keyboard input is serviced between chunks. */
static unsigned char rx[64];
/* Exported by fujinet-lib's Atari SIO transport. Unlike network_read_nb,
 * this uses the already obtained status and returns actual read errors. */
extern unsigned char sio_read(unsigned char unit, void *buffer,
                              unsigned int length);
static unsigned char tabs[WIDTH];
static unsigned char cx, cy;
static bool escaped;
static bool cursor_shown;
static unsigned int cursor_position;
static unsigned char cursor_value;
extern unsigned char bbs_getkey(void);
static unsigned char * const pixels = (unsigned char *)DISPLAY_MEMORY;

static void hide_cursor(void)
{
    if (cursor_shown) pixels[cursor_position] = cursor_value;
    cursor_shown = false;
}

static void show_cursor(void)
{
    if (!(OS.rtclok[2] & 32))
    {
        hide_cursor();
        return;
    }
    /* Leave an already visible cursor alone during keyboard/network calls. */
    if (!cursor_shown)
    {
        cursor_position = (unsigned int)cy * WIDTH + cx;
        cursor_value = pixels[cursor_position];
        pixels[cursor_position] ^= 0x80;
        cursor_shown = true;
    }
}

static unsigned char screen_code(unsigned char c)
{
    unsigned char inverse = c & 0x80;
    c &= 0x7f;
    if (c < 32) c += 64;
    else if (c < 96) c -= 32;
    return c | inverse;
}

static void clear_terminal(void)
{
    memset(pixels, 0, CELLS);
    cx = cy = 0;
}

static void next_line(void)
{
    cx = 0;
    if (++cy == HEIGHT)
    {
        memmove(pixels, pixels + WIDTH, CELLS - WIDTH);
        memset(pixels + CELLS - WIDTH, 0, WIDTH);
        cy = HEIGHT - 1;
    }
}

/* Interpret ATASCII editing controls; escaped controls become glyphs. */
static void terminal_byte(unsigned char c)
{
    unsigned int row = (unsigned int)cy * WIDTH;
    unsigned char next;
    if (escaped)
        escaped = false;
    else
    {
        switch (c)
        {
        case 27: escaped = true; return;
        case 28: if (cy) --cy; return;
        case 29: if (cy < HEIGHT - 1) ++cy; return;
        case 30: if (cx) --cx; return;
        case 31: if (cx < WIDTH - 1) ++cx; return;
        case 125: clear_terminal(); return;
        case 126:
            if (cx) --cx;
            else if (cy) { --cy; cx = WIDTH - 1; }
            pixels[(unsigned int)cy * WIDTH + cx] = 0;
            return;
        case 127: /* Tab to the next eight-column stop. */
            next = cx + 1;
            while (next < WIDTH && !tabs[next]) ++next;
            if (next == WIDTH) next_line(); else cx = next;
            return;
        case 155: next_line(); return;
        case 156: /* Delete line. */
            memmove(pixels + row, pixels + row + WIDTH,
                    CELLS - row - WIDTH);
            memset(pixels + CELLS - WIDTH, 0, WIDTH);
            cx = 0; return;
        case 157: /* Insert line. */
            memmove(pixels + row + WIDTH, pixels + row,
                    CELLS - row - WIDTH);
            memset(pixels + row, 0, WIDTH);
            cx = 0; return;
        case 158: tabs[cx] = 0; return;
        case 159: tabs[cx] = 1; return;
        case 253: return; /* Bell: silent in this first terminal version. */
        case 254:
            memmove(pixels + row + cx, pixels + row + cx + 1,
                    WIDTH - cx - 1);
            pixels[row + WIDTH - 1] = 0; return;
        case 255:
            memmove(pixels + row + cx + 1, pixels + row + cx,
                    WIDTH - cx - 1);
            pixels[row + cx] = 0; return;
        }
    }
    pixels[row + cx] = screen_code(c);
    if (++cx == WIDTH) next_line();
}

static void terminal_text(const char *s)
{
    while (*s) terminal_byte((unsigned char)*s++);
}

static void terminal_screen(void)
{
    unsigned char i;
    OS.sdmctl = 0;
    /* Three blank lines, 24 ANTIC mode-2 rows, jump back to this list. */
    memset(terminal_dlist, 2, sizeof(terminal_dlist));
    terminal_dlist[0] = terminal_dlist[1] = terminal_dlist[2] = 0x70;
    terminal_dlist[3] = 0x42;
    terminal_dlist[4] = DISPLAY_MEMORY & 255;
    terminal_dlist[5] = DISPLAY_MEMORY >> 8;
    terminal_dlist[29] = 0x41;
    terminal_dlist[30] = (unsigned int)terminal_dlist & 255;
    terminal_dlist[31] = (unsigned int)terminal_dlist >> 8;
    OS.sdlst = terminal_dlist;
    OS.savmsc = pixels;
    OS.rowcrs = OS.colcrs = OS.dindex = 0;
    OS.crsinh = 1;
    OS.chbas = 0xe0; /* Standard OS ATASCII character set. */
    OS.color1 = 0x0e;
    OS.color2 = OS.color4 = 0x90;
    POKE(0xd01d, 0); /* Hide CONFIG's player/missile selection bar. */
    clear_terminal();
    escaped = cursor_shown = false;
    for (i = 0; i < WIDTH; ++i) tabs[i] = (i && !(i & 7));
    OS.sdmctl = 0x22;
}

static bool exit_pressed(void)
{
    return (PEEK(0xd01f) & 4) == 0; /* OPTION is never sent to the BBS. */
}

void bbs_terminal(void)
{
    unsigned char saved_host = selected_host_slot;
    unsigned char saved_device = selected_device_slot;
    unsigned char saved_cursor = OS.crsinh;
    unsigned char saved_charbase = OS.chbas;
    unsigned char saved_sound = OS.soundr;
    unsigned char *saved_screen = OS.savmsc;
    unsigned char saved_row = OS.rowcrs, saved_mode = OS.dindex;
    unsigned int saved_column = OS.colcrs;
    unsigned char key, connected, device_error, result;
    unsigned int waiting = 0;
    unsigned char last_poll, now;
    int received, i;
    const char *message = 0;
    bool user_exit = false;

    OS.soundr = 0; /* SIO transfer sounds, not a modem audio channel. */
    terminal_screen();
    terminal_text("ATARI BBS GATEWAY\x9bOPTION: disconnect and return to CONFIG\x9b" "Connecting...\x9b");
    while (exit_pressed()) {} /* Release any OPTION held at entry. */
    result = network_init();
    if (result != FN_ERR_OK) { message = "FUJINET NETWORK INIT FAILED"; goto finish; }
    result = network_open(BBS_URI, 12, 0);
    if (result != FN_ERR_OK) { message = "BBS CONNECTION FAILED"; goto finish; }
    clear_terminal();
    last_poll = OS.rtclok[2] - 3;
    while (true)
    {
        if (exit_pressed())
        {
            user_exit = true;
            break;
        }
        if (kbhit())
        {
            key = bbs_getkey(); /* Native ATASCII, including RETURN = $9B. */
            if (network_write(BBS_URI, &key, 1) != FN_ERR_OK)
            { message = "BBS WRITE FAILED"; break; }
            /* Promptly check for a response after sending keyboard input. */
            last_poll = OS.rtclok[2] - 3;
            /* Remote echo only: avoid displaying each key twice. */
        }
        now = OS.rtclok[2];
        /* Poll idle connections at most once per three video frames.
         * Drain the reported buffer without a second status per read. */
        if (!waiting && (unsigned char)(now - last_poll) >= 3)
        {
            last_poll = now;
            result = network_status(BBS_URI, &waiting, &connected, &device_error);
            if (result != FN_ERR_OK) { message = "BBS STATUS FAILED"; break; }
            if (!waiting)
            {
                if (!connected || device_error == 136)
                { message = "BBS DISCONNECTED"; break; }
                if (device_error != 0 && device_error != 1)
                { message = "BBS NETWORK ERROR"; break; }
            }
        }
        if (waiting)
        {
            received = waiting < sizeof(rx) ? waiting : sizeof(rx);
            if (sio_read(1, rx, received) != FN_ERR_OK)
            { message = "BBS READ FAILED"; break; }
            waiting -= received;
            hide_cursor(); /* Remove overlay only before changing the screen. */
            for (i = 0; i < received; ++i) terminal_byte(rx[i]);
            /* Refresh immediately after draining a burst, so long screens
             * are not slowed by the idle polling interval. */
            if (!waiting) last_poll = OS.rtclok[2] - 3;
        }
        show_cursor();
    }
finish:
    hide_cursor();
    /* Close even after a failed open to release partial network state. */
    if (network_close(BBS_URI) != FN_ERR_OK && !message)
        message = "BBS CLOSE FAILED";
    /* An OPTION request remains an exit request even if CLOSE fails.
     * Never require a second press to acknowledge that cleanup error. */
    if (message && !user_exit)
    {
        terminal_byte(155);
        terminal_text(message);
        terminal_text("\x9bPress OPTION to return to CONFIG");
        while (!exit_pressed()) {}
    }
    while (exit_pressed()) {}
    OS.soundr = saved_sound;
    OS.savmsc = saved_screen;
    OS.rowcrs = saved_row;
    OS.colcrs = saved_column;
    OS.dindex = saved_mode;
    OS.crsinh = saved_cursor;
    OS.chbas = saved_charbase;
    screen_init();
    selected_host_slot = saved_host;
    selected_device_slot = saved_device;
    state = HOSTS_AND_DEVICES;
    /* The caller remains inside its pane's input loop. It will not redraw
     * the main screen on its own until the pane changes, so do it here. */
    screen_hosts_and_devices(hostSlots, deviceSlots, deviceEnabled);
    if (hd_subState == HD_DEVICES)
        screen_hosts_and_devices_devices();
    else
        screen_hosts_and_devices_hosts();
    /* No disk, host-slot, boot-mode or reboot commands are issued here. */
}
#endif
