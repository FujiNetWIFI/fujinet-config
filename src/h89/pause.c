#ifdef BUILD_H89

/**
 * @file pause.c
 * @brief Waiting - Heathkit H89.
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 */

#include <fujinet-fuji.h>
#include "../pause.h"
#include "h19.h"

void pause(unsigned char delay)
{
  h19_flush();
  fuji_wait_jiffies(delay);
}

#endif /* BUILD_H89 */
