/**
 * @file kbd.h
 * @brief Keyboard - TRS-80 Model I.
 */

#ifndef TRS80M1_KBD_H
#define TRS80M1_KBD_H

#include <stdint.h>

/** The next key (see key_codes.h), or 0 when none is down */
uint8_t kbd_get(void);

/** Wait for a key, sending the screen first */
uint8_t kbd_wait(void);

#endif /* TRS80M1_KBD_H */
