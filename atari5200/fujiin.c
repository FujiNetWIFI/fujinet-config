/* fujiin.c -- see fujiin.h. The stick comes from the VBI's POT shadows, the
 * keypad from the lib's keypad IRQ (fuji_a5200_key). */

#include <fujinet-atari5200.h>

#include "fujiin.h"
#include "antic.h"
#include "sfx.h"

#define REG(a)     (*(volatile unsigned char *)(a))
#define PADDL0     REG(0x11)    /* stick X, 0-228, 114 at rest */
#define PADDL1     REG(0x12)    /* stick Y */
#define TRIG0      REG(0xC010)  /* top fire: 0 while held */
#define SKSTAT     REG(0xE80F)  /* bit 3 clear: the lower fire button */

/* Past these the stick is pushed; between them it is at rest. */
#define STICK_LO   0x40
#define STICK_HI   0xA4

#define REPEAT_FIRST 20     /* frames before a held direction repeats */
#define REPEAT_NEXT  6      /* frames between repeats after that */

#define B_FIRE    0x01
#define B_BACK    0x02

static unsigned char last_dir;
static unsigned char last_btn;
static unsigned char hold;

void in_init(void)
{
    fuji_a5200_key = 0;
}

unsigned char in_frames(void)
{
    return mt_frames;
}

static unsigned char stick(void)
{
    unsigned char x = PADDL0, y = PADDL1;

    if (y < STICK_LO) return IN_UP;
    if (y > STICK_HI) return IN_DOWN;
    if (x < STICK_LO) return IN_LEFT;
    if (x > STICK_HI) return IN_RIGHT;
    return 0;
}

static unsigned char buttons(void)
{
    unsigned char b = 0;

    if (TRIG0 == 0)
        b |= B_FIRE;
    if (!(SKSTAT & 0x08))
        b |= B_BACK;
    return b;
}

/* A keypad key, as an event: each press is one, the IRQ having latched it. */
static unsigned char keypad(void)
{
    unsigned char k = fuji_a5200_key;

    if (k == 0)
        return IN_NONE;
    fuji_a5200_key = 0;
    switch (k) {
    case FUJI_A5200_KEY_DIGIT(2): return IN_UP;
    case FUJI_A5200_KEY_DIGIT(8): return IN_DOWN;
    case FUJI_A5200_KEY_DIGIT(4): return IN_LEFT;
    case FUJI_A5200_KEY_DIGIT(6): return IN_RIGHT;
    case FUJI_A5200_KEY_HASH:     return IN_FIRE;
    case FUJI_A5200_KEY_STAR:
    case FUJI_A5200_KEY_RESET:    return IN_BACK;
    case FUJI_A5200_KEY_PAUSE:    return IN_SELECT;
    case FUJI_A5200_KEY_START:    return IN_START;
    default:                      return IN_NONE;
    }
}

static unsigned char read_event(void)
{
    unsigned char dir, btn, k;

    mt_sync();
    k = keypad();
    if (k != IN_NONE)
        return k;

    dir = stick();
    if (dir != 0) {
        if (dir != last_dir) {
            last_dir = dir;
            hold = REPEAT_FIRST;
            return dir;
        }
        if (hold != 0 && --hold == 0) {
            hold = REPEAT_NEXT;
            return dir;
        }
    } else {
        last_dir = 0;
    }

    btn = buttons();
    if (btn != 0 && last_btn == 0) {
        last_btn = btn;
        return (btn & B_FIRE) ? IN_FIRE : IN_BACK;
    }
    if (btn == 0)
        last_btn = 0;

    return IN_NONE;
}

/* Every keypress clicks, auto-repeats included. */
unsigned char in_read(void)
{
    unsigned char ev = read_event();

    if (ev != IN_NONE)
        sfx_click();
    return ev;
}
