#include <joystick.h>
#include <nes.h>
#include <time.h>

#include <fujinet-nes.h>

#include "fujiin.h"

#define REPEAT_FIRST 20     /* frames before a held direction repeats */
#define REPEAT_NEXT  6      /* frames between repeats after that */

static unsigned char last_dir;
static unsigned char last_btn;
static unsigned char hold;
static bool keyboard;
static char last_char;

void in_init(void)
{
    joy_install(joy_static_stddrv);
    keyboard = (bool)(fuji_nes_kbd_detect() != FUJI_NES_KBD_NONE);
}

bool in_has_keyboard(void)
{
    return keyboard;
}

char in_char(void)
{
    return last_char;
}

unsigned char in_frames(void)
{
    return (unsigned char)clock();
}

static unsigned char read_event(bool text)
{
    unsigned char j, dir = 0, btn;
    char c;

    waitvsync();
    j = joy_read(JOY_1);

    if (keyboard && (c = fuji_nes_kbd_getc()) != 0) {
        switch (c) {
        case FUJI_NES_KEY_UP:    return IN_UP;
        case FUJI_NES_KEY_DOWN:  return IN_DOWN;
        case FUJI_NES_KEY_LEFT:  return IN_LEFT;
        case FUJI_NES_KEY_RIGHT: return IN_RIGHT;
        case FUJI_NES_KEY_ENTER: return text ? IN_ENTER : IN_FIRE;
        case FUJI_NES_KEY_ESC:   return text ? IN_ESC : IN_BACK;
        case FUJI_NES_KEY_BS:    return text ? IN_BS : IN_BACK;
        default:
            if (text && c >= 0x20 && c < 0x7F) {
                last_char = c;
                return IN_CHAR;
            }
            break;
        }
    }

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

unsigned char in_read(void)
{
    return read_event(false);
}

unsigned char in_read_text(void)
{
    return read_event(true);
}
