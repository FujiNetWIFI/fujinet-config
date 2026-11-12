#ifdef BUILD_ADAM

#include <fujinet-fuji.h>
#include "../pause.h"

void pause(unsigned char delay)
{
  fuji_wait_jiffies(delay);
}

#endif /* BUILD_ADAM */
