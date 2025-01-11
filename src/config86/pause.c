/**
 * @brief Pause
 */

#ifdef BUILD_CONFIG86

#include <dos.h>

void pause(unsigned char delay)
{
    sleep(delay);
}

#endif /* BUILD_CONFIG86 */
