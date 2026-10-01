#include <joystick.h>
#include <nes.h>
#include <time.h>

#include "fujiin.h"

#define REPEAT_FIRST 20     /* frames before a held direction repeats */
#define REPEAT_NEXT  6      /* frames between repeats after that */

static unsigned char last_dir;
static unsigned char last_btn;
static unsigned char hold;

void in_init(void)
{
    joy_install(joy_static_stddrv);
}

unsigned char in_frames(void)
{
    return (unsigned char)clock();
}

unsigned char in_read(void)
{
    unsigned char j, dir = 0, btn;

    waitvsync();
    j = joy_read(JOY_1);

    if (JOY_UP(j))         dir = IN_UP;
    else if (JOY_DOWN(j))  dir = IN_DOWN;
    else if (JOY_LEFT(j))  dir = IN_LEFT;
    else if (JOY_RIGHT(j)) dir = IN_RIGHT;

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

    btn = (unsigned char)(j & (JOY_BTN_A_MASK | JOY_BTN_B_MASK | JOY_SELECT_MASK | JOY_START_MASK));
    if (btn != 0 && last_btn == 0) {
        last_btn = btn;
        if (btn & JOY_BTN_A_MASK)  return IN_FIRE;
        if (btn & JOY_BTN_B_MASK)  return IN_BACK;
        if (btn & JOY_SELECT_MASK) return IN_SELECT;
        return IN_START;
    }
    if (btn == 0)
        last_btn = 0;

    return IN_NONE;
}
