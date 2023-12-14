/**
 * @brief Pause
 */

#include <cmoc.h>
#include <coco.h>

void pause(unsigned char delay)
{
  sleep(delay/60);
}
