//
// Created by jskists on 12/10/2022.
//

#ifndef FUJINET_RC2014_CONSOLE_HAL_H
#define FUJINET_RC2014_CONSOLE_HAL_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Initialised the Console hardware
 */
void console_init(void);

/**
 * Output byte to Console device
 *
 * @param ch byte to output
 */
void console_tx(uint8_t ch) __z88dk_fastcall;

/**
 * Input byte from Console device
 *
 * Blocks waiting for character
 *
 * @return character inputted
 */
uint8_t console_rx(void) __z88dk_fastcall;


/**
 * Examine number of characters available in queue
 *
 * @return characters waiting
 */
uint8_t console_rx_avail(void) __z88dk_fastcall;

/**
 * Output string to Console device
 *
 * @param s string (NUL terminated) to output
 */
void console_puts(uint8_t *s) __z88dk_fastcall;


#endif //FUJINET_RC2014_CONSOLE_HAL_H
