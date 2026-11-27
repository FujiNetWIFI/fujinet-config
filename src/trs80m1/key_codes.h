/**
 * @file key_codes.h
 * @brief Model I key codes, as the Level II ROM's keyboard routine returns them.
 *
 * There is no ESC: BREAK and CLEAR both come out as KEY_ESCAPE (kbd.c).
 */

#ifndef TRS80M1_KEY_CODES_H
#define TRS80M1_KEY_CODES_H

#define KEY_BREAK         0x01
#define KEY_BACKSPACE     0x08  /* the left arrow */
#define KEY_LEFT_ARROW    0x08
#define KEY_RIGHT_ARROW   0x09
#define KEY_DOWN_ARROW    0x0A
#define KEY_RETURN        0x0D
#define KEY_CLEAR         0x1F
#define KEY_SPACE         0x20
#define KEY_UP_ARROW      0x5B

#define KEY_ESCAPE        KEY_BREAK
#define KEY_ABORT         KEY_ESCAPE

#endif /* TRS80M1_KEY_CODES_H */
