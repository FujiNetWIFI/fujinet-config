#ifdef BUILD_PMD85
/**
 * FujiNet Config for Adam
 * 
 * Cursor routines
 */

#include "cursor.h"
#include "screen_util.h"
#include "colors.h"

static unsigned char cursor_x = 0, cursor_y = 0;
static bool cursor_visible = false;

void cursor(bool t)
{
    if (cursor_visible == t)
        return;

    cursor_visible = t;
    screen_set_region(cursor_x, cursor_y, 1, 1);
    if (cursor_visible)
        screen_fill_region(CURSOR_PATTERN_ON);
    else
        screen_fill_region(CURSOR_PATTERN_OFF);
}

void cursor_pos(unsigned char x, unsigned char y)
{
    bool on = cursor_visible;
    // remove cursor from old position
    if (on)
        cursor(0);
    // set new position
    cursor_x = x;
    cursor_y = y;
    // draw cursor on new position
    if (on)
        cursor(1);
}

#endif /* BUILD_PMD85 */
