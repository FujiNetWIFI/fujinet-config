/**
 * Bar routines
 */

#ifndef BAR_H
#define BAR_H

/**
 * Clear the currently displayed bar from screen
 */
void bar_clear(void);

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
void bar_jump(unsigned char i);

/**
 * Get current bar position
 * @return bar index
 */
unsigned char bar_get();

#endif /* BAR_H */
