#ifdef BUILD_APPLE2
#include <stdint.h>
#include "../typedefs.h"

typedef struct {
  uint8_t slot:4;
  uint8_t drive:4;
  uint8_t fuji;
} DiskII_SlotDrive;

extern DiskII_SlotDrive diskii_slotdrive[];

extern void diskii_find();

extern bool diskii_found();

#endif /* BUILD_APPLE2 */
