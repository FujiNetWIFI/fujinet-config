/**
 * @file key_codes.h
 * @brief Model II key codes under P&T CP/M.
 *
 * Every key arrives as one byte. kbd.c turns F1 and F2 into up and down.
 */

#ifndef MODEL2_KEY_CODES_H
#define MODEL2_KEY_CODES_H

#define KEY_F1            0x01
#define KEY_F2            0x02
#define KEY_CTRL_C        0x03  /* BREAK */
#define KEY_BACKSPACE     0x08
#define KEY_TAB           0x09
#define KEY_RETURN        0x0D  /* either ENTER */
#define KEY_ESCAPE        0x1B
#define KEY_LEFT_ARROW    0x1C
#define KEY_RIGHT_ARROW   0x1D
#define KEY_UP_ARROW      0x1E
#define KEY_DOWN_ARROW    0x1F
#define KEY_SPACE         0x20
#define KEY_DELETE        0x7F  /* CTRL - */

#define KEY_ABORT         KEY_ESCAPE

#endif /* MODEL2_KEY_CODES_H */
