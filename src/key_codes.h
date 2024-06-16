#ifndef KEY_CODES_H
#define KEY_CODES_H

#define SCR_WIDTH       40
#define SCR_HEIGHT      22
#define SCR_BYTES_W     40
#define SCR_BWX2        80

#if defined(__ATARI__)

#include <atari.h>

#define KEY_ESCAPE         CH_ESC
#define KEY_RETURN         CH_ENTER
#define KEY_BACKSPACE      CH_DEL
#define KEY_DELETE         CH_DELCHR
#define KEY_INSERT         0xFF
#define KEY_LEFT_ARROW     CH_CURS_LEFT
#define KEY_RIGHT_ARROW    CH_CURS_RIGHT
#define KEY_UP_ARROW       CH_CURS_UP
#define KEY_DOWN_ARROW     CH_CURS_DOWN
#define KEY_ASCII_LOW      0x20
#define KEY_ASCII_HIGH     0x7D
#define KEY_HOME           0x01
#define KEY_END            0x05
#define KEY_KILL           0x0B
#define KEY_TAB            CH_TAB

#define KEY_1              0x31
#define KEY_2              0x32
#define KEY_3              0x33
#define KEY_4              0x34
#define KEY_5              0x35
#define KEY_6              0x36
#define KEY_7              0x37
#define KEY_8              0x38
#define KEY_SPACE          0x20

#elif defined(__CBM__)

#include <cbm.h>

// back arrow <-
#define KEY_ESCAPE         0x5F
#define KEY_RETURN         CH_ENTER
#define KEY_BACKSPACE      CH_DEL
// Up arrow
#define KEY_DELETE         0x5E
#define KEY_INSERT         CH_INS
#define KEY_LEFT_ARROW     CH_CURS_LEFT
#define KEY_RIGHT_ARROW    CH_CURS_RIGHT
#define KEY_UP_ARROW       CH_CURS_UP
#define KEY_DOWN_ARROW     CH_CURS_DOWN
#define KEY_ASCII_LOW      0x20
#define KEY_ASCII_HIGH     0x7D
// ctrl-a
#define KEY_HOME           0x01
// ctrl-e
#define KEY_END            0x05
// ctrl-k
#define KEY_KILL           0x0B
// C= q
#define KEY_TAB            0xAB

#define KEY_1              0x31
#define KEY_2              0x32
#define KEY_3              0x33
#define KEY_4              0x34
#define KEY_5              0x35
#define KEY_6              0x36
#define KEY_7              0x37
#define KEY_8              0x38
#define KEY_SPACE          0x20

#elif defined(__APPLE2__)

#include <apple2.h>

#endif // defined(...)

#endif // KEY_CODES_H