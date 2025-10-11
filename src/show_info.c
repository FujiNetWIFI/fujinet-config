/**
 * Show adapter info
 */

#include "show_info.h"
#include "typedefs.h"
#include "screen.h"
#include "io.h"
#include "input.h"
#include "constants.h"

SISubState si_subState;

void show_info(void)
{
  si_subState=SI_SHOWINFO;
  
  screen_show_info_extended(io_get_device_enabled_status(PRINTER),io_get_adapter_config_extended());

  while (si_subState==SI_SHOWINFO)
    si_subState=input_show_info();
  
  si_subState = SI_SHOWINFO;
}
