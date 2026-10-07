/* fujiin.c -- see fujiin.h. Reads TIA and RIOT; writes only RIOT. */

#include "fujiin.h"
#include "maria.h"
#include "sfx.h"

#define REG(a)     (*(volatile unsigned char *)(a))
#define INPT0      REG(0x08)    /* button 2, two-button mode: bit 7 set */
#define INPT1      REG(0x09)    /* button 1, two-button mode: bit 7 set */
#define INPT4      REG(0x0C)    /* a 2600 joystick's button: bit 7 clear */
#define SWCHA      REG(0x280)   /* bits 7-4: right, left, down, up; low */
#define SWCHB      REG(0x282)   /* bit 0 RESET, 1 SELECT, 3 PAUSE; low */
#define CTLSWB     REG(0x283)

#define PB2        0x04         /* left port's two-button mode, driven low */

#define REPEAT_FIRST 20     /* frames before a held direction repeats */
#define REPEAT_NEXT  6      /* frames between repeats after that */

#define B_FIRE    0x01
#define B_BACK    0x02
#define B_SELECT  0x04
#define B_START   0x08

static unsigned char last_dir;
static unsigned char last_btn;
static unsigned char hold;

void in_init(void)
{
    CTLSWB = PB2;
    SWCHB = 0;
}

unsigned char in_frames(void)
{
    return mt_frames;
}

static unsigned char buttons(void)
{
    unsigned char b = 0, sw = (unsigned char)~SWCHB;

    if ((INPT1 & 0x80) || !(INPT4 & 0x80))
        b |= B_FIRE;
    if ((INPT0 & 0x80) || (sw & 0x01))
        b |= B_BACK;
    if (sw & 0x02)
        b |= B_SELECT;
    if (sw & 0x08)
        b |= B_START;
    return b;
}

static unsigned char read_event(void)
{
    unsigned char j, dir = 0, btn;

    mt_sync();
    j = (unsigned char)~SWCHA;

    if (j & 0x10)      dir = IN_UP;
    else if (j & 0x20) dir = IN_DOWN;
    else if (j & 0x40) dir = IN_LEFT;
    else if (j & 0x80) dir = IN_RIGHT;

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
        if (btn & B_FIRE)   return IN_FIRE;
        if (btn & B_BACK)   return IN_BACK;
        if (btn & B_SELECT) return IN_SELECT;
        return IN_START;
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
