#ifdef BUILD_C64
/**
 * FujiNet Config for C64
 *
 * Cursor routines
 */

#ifndef CURSOR_H
#define CURSOR_H

#include <stdbool.h>

void cursor(bool t);
void cursor_pos(unsigned char x, unsigned char y);

#endif /* CURSOR_H */
#endif /* BUILD_C64 */
