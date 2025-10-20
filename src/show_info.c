/**
 * Show adapter info
 */

#include "show_info.h"
#include "typedefs.h"
#include "screen.h"
#include "input.h"
#include "constants.h"
#include "globals.h"

SISubState si_subState;

void show_info(void)
{
  si_subState=SI_SHOWINFO;
  
  fuji_get_adapter_config_extended(&adapterConfigExt);
  screen_show_info_extended(fuji_get_device_enabled_status(PRINTER), &adapterConfigExt);

  while (si_subState==SI_SHOWINFO)
    si_subState=input_show_info();
  
  si_subState = SI_SHOWINFO;
}
