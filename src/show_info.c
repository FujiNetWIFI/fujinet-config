/**
 * FujiNet for #Adam configuration program
 *
 * Show adapter info
 */

#include "show_info.h"

#ifdef _CMOC_VERSION_
#include "coco/screen.h"
#include "coco/input.h"
#include "coco/globals.h"
#include "coco/io.h"
#define PRINTER 2 // ?
#endif /* _CMOC_VERSION_ */

#ifdef BUILD_ADAM
#include "adam/screen.h"
#include "adam/input.h"
#include "adam/globals.h"
#include "adam/io.h"
#define PRINTER 2
#endif /* BUILD_ADAM */

#ifdef BUILD_APPLE2
#include "apple2/screen.h"
#include "apple2/input.h"
#include "apple2/globals.h"
#include "apple2/io.h"
#define PRINTER 0 // TBD
#endif /* BUILD_APPLE2 */

#ifdef BUILD_ATARI
#include "atari/screen.h"
#include "atari/input.h"
#include "atari/globals.h"
#include "atari/io.h"
#define PRINTER 2 // ?
#endif /* BUILD_ATARI */

#ifdef BUILD_C64
#include "c64/screen.h"
#include "c64/input.h"
#include "c64/globals.h"
#include "c64/io.h"
#define PRINTER 0
#endif /* BUILD_C64 */

#ifdef BUILD_PC8801
#include "pc8801/screen.h"
#include "pc8801/input.h"
#include "pc8801/globals.h"
#include "pc8801/io.h"
#endif /* BUILD_PC8801 */

#ifdef BUILD_PC6001
#include "pc6001/screen.h"
#include "pc6001/input.h"
#include "pc6001/globals.h"
#include "pc6001/io.h"
#endif /* BUILD_PC6001 */

#ifdef BUILD_PMD85
#include "pmd85/screen.h"
#include "pmd85/input.h"
#include "pmd85/globals.h"
#include "pmd85/io.h"
#define PRINTER 2 // ?
#endif /* BUILD_PMD85 */

#ifdef BUILD_RC2014
#include "rc2014/screen.h"
#include "rc2014/input.h"
#include "rc2014/globals.h"
#include "rc2014/io.h"
#define PRINTER 2 // ?
#endif /* BUILD_RC2014 */

SISubState si_subState;

void show_info(void)
{
  si_subState=SI_SHOWINFO;
  
  screen_show_info(io_get_device_enabled_status(PRINTER),io_get_adapter_config());

  while (si_subState==SI_SHOWINFO)
    si_subState=input_show_info();
  
  si_subState = SI_SHOWINFO;
}
