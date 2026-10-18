#ifdef BUILD_MODEL2

/**
 * @file con.c
 * @brief Raw console I/O and P&T special functions - TRS-80 Model II, P&T CP/M.
 *
 * Console bytes go straight to the BIOS jump table: the BDOS console calls
 * expand tabs and act on ^S and ^C. P&T's BIOS keeps IX, IY and the
 * alternate registers but may change any of the rest.
 */

#include "con.h"

/* (0001) points at WBOOT, base+3: CONST is +3 from there, CONIN +6, CONOUT +9 */
void con_out(uint8_t c) __z88dk_fastcall __naked
{
#asm
    EXTERN  l_dcal
    ld      c,l
    ld      hl,(1)
    ld      de,9
    add     hl,de
    jp      l_dcal      ; CONOUT
#endasm
}

int con_in(void) __naked
{
#asm
    EXTERN  l_dcal
    ld      hl,(1)
    ld      de,3
    add     hl,de
    call    l_dcal      ; CONST: A = FF when a key is waiting
    or      a
    ld      hl,-1
    ret     z
    ld      hl,(1)
    ld      de,6
    add     hl,de
    call    l_dcal      ; CONIN
    ld      l,a
    ld      h,0
    ret
#endasm
}

/* Special functions: B = the function number, CALL 40h; results come back
   in the registers */
uint32_t con_ticks(void) __naked
{
#asm
    ld      b,14
    call    40h         ; DE = low word, HL = high word
    ex      de,hl
    ret
#endasm
}

#endif /* BUILD_MODEL2 */
