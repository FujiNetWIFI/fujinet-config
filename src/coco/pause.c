/**
 * @brief Pause
 */

#ifdef _CMOC_VERSION_

#include <cmoc.h>
#include <coco.h>

void pause(unsigned char delay)
{
  sleep(delay/60);
}

#endif /* _CMOC_VERSION */
