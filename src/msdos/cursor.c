#ifdef BUILD_MSDOS
/**
 * @file cursor.c
 * @brief MS-DOS hardware text cursor show/hide routines.
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 */

#include <dos.h>
#include "cursor.h"

/**
 * @brief Show or hide the hardware text cursor.
 * @param on true to show, false to hide.
 */
void cursor(bool on)
{
    static union REGS r;
    r.h.ah = 0x01;
    if (on) {
        r.h.ch = 0x06;  /* normal: start scan line 6 */
        r.h.cl = 0x07;  /* end scan line 7           */
    } else {
        r.h.ch = 0x20;  /* bit 5 set = cursor hidden */
        r.h.cl = 0x00;
    }
    int86(0x10,(union REGS *)&r,(union REGS *)&r);
}

#endif /* BUILD_MSDOS */
