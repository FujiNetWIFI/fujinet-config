/**
 * @file con.h
 * @brief Raw console I/O for the Heathkit H89 under HDOS or CP/M.
 */

#ifndef H89_CON_H
#define H89_CON_H

#include <stdint.h>

/** The H89's 2 ms clock tick counter (HDOS's, or the CP/M BIOS's) */
#ifdef __HDOS__
#define TICKS (*(volatile uint16_t *)0x201B)
#else
#define TICKS (*(volatile uint16_t *)0x000B)
#endif
#define TICKS_PER_SECOND 500

/** Put the OS's console into raw mode, and back */
void con_init(void);
void con_exit(void);

/** Send one byte to the terminal, untouched */
void con_out(uint8_t c) __z88dk_fastcall;

/** The next keyboard byte, or -1 when none is waiting */
int con_in(void);

#endif /* H89_CON_H */
