/**
 * @file mount_and_boot.c
 * @brief MS-DOS mount-all and boot implementation.
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 */

#include <stdlib.h>
#include <fujinet-fuji.h>

/**
 * @brief Mount all FujiNet device slots and exit the configurator.
 */
void mount_and_boot(void)
{
  fuji_mount_all();
  exit(0);
}
