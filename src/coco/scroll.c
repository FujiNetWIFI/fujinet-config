#include <cmoc.h>
#include <coco.h>
#include <fujinet-fuji.h>
#include "../constants.h"
#include "../globals.h"
#include "../screen.h"
#include "../select_file.h"
#include "scroll.h"


word lastTimer;
word idleCounter = IDLE_TIMEOUT_COUNT;
int scrollOffset = 0;
int scrollDir = 1; // 1 = right, -1 = left
byte got_scrolltext;


// Draw one line of up to 32 chars from given offset (bounce mode)
void scroll_draw_line_at_y(const char *line, byte y, int offset)
{
    char buf[SCREEN_WIDTH + 1];
    int len = strlen(line);

    memset(buf, ' ', SCREEN_WIDTH);
    buf[SCREEN_WIDTH] = 0;

    if (offset < len)
    {
        int copyLen = len - offset;
        if (copyLen > SCREEN_WIDTH)
            copyLen = SCREEN_WIDTH;
        memcpy(buf, line + offset, copyLen);
    }

    locate(0, y+3); // Need to add offset for file choice screen
    putstr(buf, SCREEN_WIDTH);
    bar_draw(y+3, false);
}


// Scroll the currently highlighted line by 1 character (bounce mode)
void scroll_step(void)
{
    byte y = (byte) bar_get();
    if (!got_scrolltext)
    {
        // Get the full text to scroll
        select_get_filename(255);
        got_scrolltext = true;
    }

    int len = strlen(response);

	if (len <= SCREEN_WIDTH)
        return; // no need to scroll

    scrollOffset += scrollDir;
    if (scrollOffset < 0)
    {
        scrollOffset = 0;
        scrollDir = 1;
    }
    else if (scrollOffset > len - SCREEN_WIDTH)
    {
        scrollOffset = len - SCREEN_WIDTH;
        scrollDir = -1;
    }

    scroll_draw_line_at_y(response, y, scrollOffset);
}

// Reset scrolling when user interacts,
// Or initial zetup
void scroll_reset(bool init)
{
	byte y = (byte)bar_get();
	scrollOffset = 0;
	scrollDir = 1;
	idleCounter = IDLE_TIMEOUT_COUNT;
	if (init)
	{
		lastTimer = 0;
	}
	else
	{
		if (got_scrolltext && strlen(response) > SCREEN_WIDTH)
		{
			// Get the short version of the text again
			select_get_filename(DIR_MAX_LEN);

			screen_select_file_display_entry(y, response, 0);
		}
	}
	got_scrolltext = false;
}
