#ifndef DISPLAY_STRING_H
#define DISPLAY_STRING_H

#include <stdint.h>

void display_string_in_viewport(uint8_t x, uint8_t y, char* str, int current_lentgth, int viewport_width, int cursor_pos, bool is_password);

#endif // DISPLAY_STRING_H