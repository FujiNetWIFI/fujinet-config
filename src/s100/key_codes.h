#ifndef S100_KEY_CODES_H
#define S100_KEY_CODES_H

/* ASCII keys, as the terminal sends them */
#define KEY_ESCAPE      0x1B
#define KEY_ABORT       KEY_ESCAPE
#define KEY_RETURN      0x0D
#define KEY_TAB         0x09
#define KEY_BACKSPACE   0x08

/* Decoded from ANSI escape sequences or the WordStar ^E ^X ^S ^D diamond */
#define KEY_UP_ARROW    0x80
#define KEY_DOWN_ARROW  0x81
#define KEY_LEFT_ARROW  0x82
#define KEY_RIGHT_ARROW 0x83
#define KEY_HOME        0x84
#define KEY_END         0x85
#define KEY_DELETE      0x86
#define KEY_PAGE_UP     0x87
#define KEY_PAGE_DOWN   0x88

#endif /* S100_KEY_CODES_H */
