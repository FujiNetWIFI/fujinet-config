/**
 * @file kbd.h
 * @brief Keyboard - TRS-80 Model II.
 */

#ifndef MODEL2_KBD_H
#define MODEL2_KBD_H

#include <stdint.h>

/** The next key (see key_codes.h), or 0 when none is waiting */
uint8_t kbd_get(void);

/** Wait for a key, sending the screen first */
uint8_t kbd_wait(void);

#endif /* MODEL2_KBD_H */
