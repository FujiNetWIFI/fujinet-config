#ifdef BUILD_ATARI
/**
 * FujiNet Configurator
 *
 * Function to die, wait for keypress and then do coldstart
 */

#include <atari.h>
#include <conio.h>
#include "../io.h"
#include "../die.h"

/**
 * Stop, wait for keypress, then coldstart
 */
void die(void)
{
    while (!kbhit())
    {
    }
    cold_start();
}

void quit(void)
{

}

/**
 * Wait a moment.
 */
void wait_a_moment(void)
{
    rtclr();
    while (OS.rtclok[2] < 128)
    {
    }
    if (kbhit())
        cgetc();
}
#endif
