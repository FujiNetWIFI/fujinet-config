#ifdef BUILD_MODEL2

/**
 * @file pause.c
 * @brief Waiting - TRS-80 Model II.
 */

#include <fujinet-fuji.h>
#include "../pause.h"
#include "vid.h"

void pause(unsigned char delay)
{
  vid_flush();
  fuji_wait_jiffies(delay);
}

#endif /* BUILD_MODEL2 */
