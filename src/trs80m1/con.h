/**
 * @file con.h
 * @brief The keyboard, interrupts and the boot jump - TRS-80 Model I.
 */

#ifndef TRS80M1_CON_H
#define TRS80M1_CON_H

#include <stdint.h>

/** Turn interrupts off, and keep them off: the card's INT* is a level */
void con_di(void);

/** The ROM's keyboard scan: a key code, or 0 when none is down */
uint8_t con_key(void);

/** Reboot from drive 0: stop the FD1771 and start the ROM over */
void con_reboot(void);

#endif /* TRS80M1_CON_H */
