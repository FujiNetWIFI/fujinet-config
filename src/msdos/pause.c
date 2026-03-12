/**
 * @file pause.c
 * @brief MS-DOS busy-wait pause implementation.
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 */

#include <time.h>
#include "../pause.h"

#define TICKS_PER_SEC 60

/**
 * @brief Busy-wait for a given number of clock ticks (at 60 ticks/sec).
 * @param delay Number of 1/60-second ticks to wait.
 */
void pause(unsigned char delay)
{
    clock_t start = clock();
    clock_t ticks = (clock_t)delay * CLOCKS_PER_SEC / TICKS_PER_SEC;

    while ((clock() - start) < ticks)
        ;
}
