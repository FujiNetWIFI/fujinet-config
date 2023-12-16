#ifdef _CMOC_VERSION_
/**
 * Bar routines
 */

#ifndef BAR_H
#define BAR_H

#define true 1
#define false 0

#include "stdbool.h"
#include "globals.h"

/**
 * Draw bar at y
 */
void bar_draw(int y, bool clear);

/**
 * Clear the currently displayed bar from screen
 * @param old true = old position, false = current position
 */
void bar_clear(bool old);

/**
 * Set up bar and start display on row
 * @param y Y column for bar display
 * @param c # of columns for left
 * @param m number of items
 * @param i item index to start display
 */
void bar_set(unsigned char y, unsigned char c, unsigned char m, unsigned char i);

/**
 * Move bar upward until index 0
 */
void bar_up();

/**
 * Move bar downward until index m
 */
void bar_down();

/**
 * Jump to location 
 * @param i new y offset
 */
void bar_jump(int i);

/**
 * Get current bar position
 * @return bar index
 */
int bar_get();

/**
 * Set Bar Color
 * @param f foreground color 0-15
 * @param b background color 0-15
 */
void bar_color(unsigned char f,unsigned char b);

void bar_update(void);

void bar_show(unsigned char y);
void bar_set_color(unsigned char c);

void bar_up();
void bar_down();

#endif /* BAR_H */
#endif /* _CMOC_VERSION_ */
