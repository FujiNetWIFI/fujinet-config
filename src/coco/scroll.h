#ifndef SCROLL_H
#define SCROLL_H
#include <cmoc.h>
#include <coco.h>
// Timer runs at 60 Hz
#define SCROLL_DELAY_TICKS  30   // 30 ticks = ~0.5 sec
#define IDLE_TIMEOUT_COUNT  10   // 10 × 0.5s = ~5 sec
#define SCREEN_WIDTH        32

extern word lastTimer;
extern word idleCounter;
extern int scrollOffset;
extern int scrollDir; // 1 = right, -1 = left

static void scroll_draw_line_at_y(const char *line, byte y, int offset);
static void scroll_step(void);
void scroll_reset(bool init);

#endif