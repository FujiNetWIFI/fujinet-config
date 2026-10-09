/* fujiin.c -- the joypad in port 1, and Pause. */

#include <arch/sms.h>

#include "fujiin.h"
#include "fujisnd.h"

#define REPEAT_DELAY 20
#define REPEAT_EVERY 4

static unsigned char frames;
static unsigned char last_pad;
static unsigned char last_pause;
static unsigned char held_since;

static void poll_frame(void)
{
    if (IO_VDP_STATUS & 0x80) {
        frames++;
        snd_tick();
    }
}

void in_init(void)
{
    frames = 0;
    last_pad = (unsigned char)(~IO_DC & 0x3F);
    last_pause = pause_flag;
}

unsigned char in_frames(void)
{
    poll_frame();
    return frames;
}

static unsigned char dir_event(unsigned char pad)
{
    if (pad & 0x01) return IN_UP;
    if (pad & 0x02) return IN_DOWN;
    if (pad & 0x04) return IN_LEFT;
    if (pad & 0x08) return IN_RIGHT;
    return IN_NONE;
}

unsigned char in_read(void)
{
    unsigned char pad, pressed;

    poll_frame();
    if (pause_flag != last_pause) {
        last_pause = pause_flag;
        return IN_MENU;
    }
    pad = (unsigned char)(~IO_DC & 0x3F);
    pressed = (unsigned char)(pad & ~last_pad);
    last_pad = pad;
    if (pressed) {
        held_since = frames;
        if (pressed & 0x10) return IN_FIRE;
        if (pressed & 0x20) return IN_KEYSTAR;
        return dir_event(pressed);
    }
    if ((pad & 0x0F) && (unsigned char)(frames - held_since) >= REPEAT_DELAY) {
        held_since = (unsigned char)(frames - REPEAT_DELAY + REPEAT_EVERY);
        return dir_event(pad);
    }
    return IN_NONE;
}
