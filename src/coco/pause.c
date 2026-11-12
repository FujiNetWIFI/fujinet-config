/**
 * @brief Pause
 */

#ifdef _CMOC_VERSION_

#include <cmoc.h>
#include <fujinet-fuji.h>
#include "../pause.h"

void pause(unsigned char delay)
{
  fuji_wait_jiffies(delay);
}

#endif /* _CMOC_VERSION_ */
