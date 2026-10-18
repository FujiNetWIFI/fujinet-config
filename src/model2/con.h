/**
 * @file con.h
 * @brief Raw console I/O and P&T special functions - TRS-80 Model II, P&T CP/M.
 */

#ifndef MODEL2_CON_H
#define MODEL2_CON_H

#include <stdint.h>

/** Send one byte to the P&T video driver, untouched */
void con_out(uint8_t c) __z88dk_fastcall;

/** The next keyboard byte, or -1 when none is waiting */
int con_in(void);

/** P&T's real time clock, in 1/100 s since boot (special function 14) */
uint32_t con_ticks(void);

/** Write c straight into video memory at offset off (row * 80 + column),
    with no cursor movement */
void con_vram_put(uint16_t off, uint8_t c);

#endif /* MODEL2_CON_H */
