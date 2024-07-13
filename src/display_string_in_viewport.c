#ifdef USE_EDITSTRING

#include <conio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "display_string_in_viewport.h"

void display_string_in_viewport(uint8_t x, uint8_t y, char* str, int current_length, int viewport_width, int cursor_pos, bool is_password) {
    int i, char_index, start_pos, half_viewport;
    start_pos = 0;
    half_viewport = viewport_width / 2;

    if (cursor_pos > half_viewport && current_length >= viewport_width) {
        start_pos = cursor_pos - half_viewport;
        if (cursor_pos >= current_length) {
            start_pos = current_length - viewport_width + 1; // Adjust to show the cursor at the end
        } else if (start_pos + viewport_width > current_length) {
            start_pos = current_length - viewport_width; // Prevent start_pos from going too far
        }
    }

    gotoxy(x, y);
    for (i = 0; i < viewport_width; i++) {
        char_index = i + start_pos;
        if (char_index == cursor_pos) {
            revers(1); // Invert the character for cursor position
        }

        if (char_index < current_length) {
            if (!is_password)
                cputc(str[char_index]);
            else
                cputc('*');
        } else {
            cputc(' ');
        }
        if (char_index == cursor_pos) {
            revers(0); // Revert to normal video mode after printing the cursor character
        }
    }
}

#endif /* USE_EDITSTRING */