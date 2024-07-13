/**
 * FujiNet for #Adam configuration program
 *
 * Perform Copy operation
 */

#ifdef _CMOC_VERSION_
#include <cmoc.h>
#include "coco/stdbool.h"
#include "coco/screen.h"
#include "coco/io.h"
#include "coco/globals.h"
#include "coco/input.h"
#include "coco/bar.h"
#else
#include <string.h>
#endif /* _CMOC_VERSION_ */
#include "fuji_typedefs.h"

#ifdef BUILD_ADAM
#include "adam/screen.h"
#include "adam/io.h"
#include "adam/globals.h"
#include "adam/input.h"
#include "adam/bar.h"
#endif /* BUILD_ADAM */

#ifdef BUILD_APPLE2
#include "apple2/screen.h"
#include "apple2/io.h"
#include "apple2/globals.h"
#include "apple2/input.h"
#include "apple2/bar.h"
#endif /* BUILD_APPLE2 */

#ifdef BUILD_ATARI
#include "atari/screen.h"
#include "atari/io.h"
#include "atari/globals.h"
#include "atari/input.h"
#include "atari/bar.h"
#endif /* BUILD_ATARI */

#ifdef BUILD_C64
#include "c64/screen.h"
#include "c64/io.h"
#include "c64/globals.h"
#include "c64/input.h"
#include "c64/bar.h"
#endif /* BUILD_C64 */

#ifdef BUILD_PC8801
#include "pc8801/screen.h"
#include "pc8801/io.h"
#include "pc8801/globals.h"
#include "pc8801/input.h"
#include "pc8801/bar.h"
#endif /* BUILD_PC8801 */

#ifdef BUILD_PC6001
#include "pc6001/screen.h"
#include "pc6001/io.h"
#include "pc6001/globals.h"
#include "pc6001/input.h"
#include "pc6001/bar.h"
#endif /* BUILD_Pc6001 */

#ifdef BUILD_PMD85
#include "pmd85/screen.h"
#include "pmd85/io.h"
#include "pmd85/globals.h"
#include "pmd85/input.h"
#include "pmd85/bar.h"
#endif /* BUILD_Pmd85 */

#ifdef BUILD_RC2014
#include "rc2014/screen.h"
#include "rc2014/io.h"
#include "rc2014/globals.h"
#include "rc2014/input.h"
#include "rc2014/bar.h"
#include <conio.h>
#endif /* BUILD_RC2014 */

#include "perform_copy.h"

extern char source_filename[128];

unsigned char copy_host_slot;
char copySpec[256];

extern bool copy_mode;

void perform_copy(void)
{
  strcpy(copySpec, source_path);
  strcat(copySpec, "|");
  strcat(copySpec, path);
  strcat(copySpec,source_filename);
  
  screen_perform_copy((char *)hostSlots[copy_host_slot],(char *)source_path,(char *)hostSlots[selected_host_slot],(char *)path);

  io_copy_file(copy_host_slot, selected_host_slot);

  copy_mode = false;
  
  state=SELECT_FILE;
  backFromCopy = true;
}
