#ifdef BUILD_TRS80M1

/**
 * @file con.c
 * @brief The keyboard and the reboot - TRS-80 Model I.
 *
 * CONFIG runs with interrupts off, since the ROM's interrupt vector at
 * 4012h only does EI;RET and the card's INT* stays low until it is served.
 * The ROM's keyboard scan never enables them.
 */

#include "con.h"

void con_di(void) __naked
{
#asm
    di
    ret
#endasm
}

/* 002Bh: scan the keyboard once, A = key or 0 (0049h would wait). IX is the compiler's. */
uint8_t con_key(void) __naked
{
#asm
    push    ix
    call    002Bh
    pop     ix
    ld      l,a
    ld      h,0
    ret
#endasm
}

/* D0h is the FD1771's Force Interrupt, so the ROM's boot starts from a quiet
   controller, and drive 0 now serves the image mounted in slot 1 */
void con_reboot(void) __naked
{
#asm
    di
    ld      a,0D0h
    ld      (37ECh),a
    jp      0
#endasm
}

#endif /* BUILD_TRS80M1 */
