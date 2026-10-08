/* fujiin.c -- the joypad in port 1, and Pause. */

#include <arch/sms.h>

#include "fujidisp.h"
#include "fujiin.h"
#include "fujisnd.h"

#define REPEAT_DELAY 24
#define REPEAT_EVERY 5

static unsigned char frames;
static unsigned char last_pad;
static unsigned char last_pause;
static unsigned char held_since;
static unsigned char pending;   /* presses the poll saw that in_read hasn't */
static bool repeat = true;

/* New presses are latched here, so one made while CONFIG is typing a line
 * or wiping a window open is still there for the next in_read(). */
static void sample(void)
{
    unsigned char pad = (unsigned char)(~IO_DC & 0x3F);
    unsigned char pressed = (unsigned char)(pad & ~last_pad);

    if (pressed) {
        pending |= pressed;
        held_since = frames;
    }
    last_pad = pad;
}

static void poll_frame(void)
{
    if (IO_VDP_STATUS & 0x80) {
        frames++;
        snd_tick();
        cur_tick();
        sample();
    }
}

void in_init(void)
{
    frames = 0;
    last_pad = (unsigned char)(~IO_DC & 0x3F);
    last_pause = pause_flag;
    pending = 0;
}

unsigned char in_frames(void)
{
    poll_frame();
    return frames;
}

void in_repeat(bool on)
{
    repeat = on;
}

void in_wait(unsigned char n)
{
    unsigned char start = in_frames();

    while ((unsigned char)(in_frames() - start) < n)
        ;
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
    unsigned char pad, take;

    poll_frame();
    if (pause_flag != last_pause) {
        last_pause = pause_flag;
        return IN_MENU;
    }
    sample();
    if (pending) {
        if (pending & 0x10) {
            pending &= (unsigned char)~0x10;
            return IN_FIRE;
        }
        if (pending & 0x20) {
            pending &= (unsigned char)~0x20;
            return IN_KEYSTAR;
        }
        take = dir_event(pending);
        pending = 0;            /* one direction per press is plenty */
        return take;
    }
    pad = last_pad;
    if (repeat && (pad & 0x0F) &&
        (unsigned char)(frames - held_since) >= REPEAT_DELAY) {
        held_since = (unsigned char)(frames - REPEAT_DELAY + REPEAT_EVERY);
        return dir_event(pad);
    }
    return IN_NONE;
}
