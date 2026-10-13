/**
 * @file key_codes.h
 * @brief H19 key codes for the Heathkit H89.
 *
 * Plain keys arrive as ASCII. The keypad (in keypad shifted mode, which
 * screen_init sets) and the function keys arrive as escape sequences, which
 * kbd.c turns into the single-byte codes from 0x80 up.
 */

#ifndef H89_KEY_CODES_H
#define H89_KEY_CODES_H

#define KEY_CTRL_C        0x03
#define KEY_BACKSPACE     0x08
#define KEY_TAB           0x09
#define KEY_LINEFEED      0x0A
#define KEY_RETURN        0x0D
#define KEY_ESCAPE        0x1B
#define KEY_SPACE         0x20
#define KEY_DELETE        0x7F

#define KEY_UP_ARROW      0x80  /* ESC A, keypad 8 */
#define KEY_DOWN_ARROW    0x81  /* ESC B, keypad 2 */
#define KEY_RIGHT_ARROW   0x82  /* ESC C, keypad 6 */
#define KEY_LEFT_ARROW    0x83  /* ESC D, keypad 4 */
#define KEY_HOME          0x84  /* ESC H, keypad 5; IC (keypad 7) too */
#define KEY_END           0x85  /* IL, keypad 1: where a PC keypad has End */
#define KEY_PAGE_UP       0x86  /* DC, keypad 9: PgUp */
#define KEY_PAGE_DOWN     0x87  /* DL, keypad 3: PgDn */
#define KEY_ERASE         0x88  /* ERASE */
#define KEY_CLEAR         0x89  /* SHIFT ERASE */
#define KEY_F1            0x8A
#define KEY_F2            0x8B
#define KEY_F3            0x8C
#define KEY_F4            0x8D
#define KEY_F5            0x8E
#define KEY_BLUE          0x8F
#define KEY_RED           0x90
#define KEY_GRAY          0x91

#define KEY_ABORT         KEY_ESCAPE

#endif /* H89_KEY_CODES_H */
