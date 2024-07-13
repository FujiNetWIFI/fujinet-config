#ifdef BUILD_PMD85
/**
 * Bar routines
 */

#ifndef BAR_H
#define BAR_H

#include <stdbool.h>

/**
 * Clear the currently displayed bar from screen
 * @param oldRow true = old position, false = current position
 */
void bar_clear(bool oldRow);

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
void bar_up(void);

/**
 * Move bar downward until index m
 */
void bar_down(void);

/**
 * Jump to location
 * @param i new y offset
 */
void bar_jump(unsigned char i);

void bar_update(void);

/**
 * Get current bar position
 * @return bar index
 */
unsigned char bar_get(void);

#endif /* BAR_H */
#endif /* BUILD_PMD85 */
