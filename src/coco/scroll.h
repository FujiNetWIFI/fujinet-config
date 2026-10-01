#ifndef SCROLL_H
#define SCROLL_H

#include "coco_screen.h"

#define SCROLL_DELAY_TICKS  30
#define IDLE_TIMEOUT_COUNT  10
#define SCREEN_WIDTH        FILE_W

extern word lastTimer;
extern word idleCounter;
extern int scrollOffset;
extern int scrollDir;

void scroll_step(void);
void scroll_reset(bool init);

#endif
