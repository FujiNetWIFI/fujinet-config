/**
 * FujiNet for #Adam configuration program
 *
 * Show adapter info
 */

#include "show_info.h"

#ifdef BUILD_ADAM
#include "adam/screen.h"
#include "adam/input.h"
#include "adam/globals.h"
#include "adam/io.h"
#endif /* BUILD_ADAM */

#ifdef BUILD_APPLE2
#include "apple2/screen.h"
#include "apple2/input.h"
#include "apple2/globals.h"
#include "apple2/io.h"
#endif /* BUILD_APPLE2 */

#ifdef BUILD_C64
#include "c64/screen.h"
#include "c64/input.h"
#include "c64/globals.h"
#include "c64/io.h"
#endif /* BUILD_C64 */

SISubState si_subState;

void show_info(void)
{
  screen_show_info(io_get_adapter_config());

  while (si_subState==SI_SHOWINFO)
    si_subState=input_show_info();
}
