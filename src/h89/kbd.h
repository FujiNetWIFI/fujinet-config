/**
 * @file kbd.h
 * @brief H19 keyboard, with its escape sequences decoded - Heathkit H89.
 */

#ifndef H89_KBD_H
#define H89_KBD_H

#include <stdint.h>

/** The next key (see key_codes.h), or 0 when none is waiting */
uint8_t kbd_get(void);

/** Wait for a key, sending the screen first */
uint8_t kbd_wait(void);

#endif /* H89_KBD_H */
