#include <cmoc.h>
#include "../../check_wifi.h"
#include "../../connect_wifi.h"
#include "../../set_wifi.h"
#include "../../show_info.h"
#include "../coco_screen.h"
#include "../../typedefs.h"
#include "../launch/entry.h"

extern void runm(const char *filename);

State state;
bool backToFiles = false;
bool backFromCopy = false;

static bool ui_up = false;

static void ui_init(void)
{
  if (!ui_up)
  {
    screen_init();
    ui_up = true;
  }
}

int main(void)
{
  state = entry_take() == ENTRY_INFO ? SHOW_INFO : CHECK_WIFI;
  if (state != CHECK_WIFI)
    ui_init();

  while (true)
  {
    switch (state)
    {
    case CHECK_WIFI:
      check_wifi();
      break;
    case CONNECT_WIFI:
      ui_init();
      connect_wifi();
      break;
    case SET_WIFI:
      ui_init();
      set_wifi();
      break;
    case SHOW_INFO:
      ui_init();
      show_info();
      break;
    default:
      if (ui_up)
      {
        screen_loading("Hosts and Devices");
        screen_handoff();
      }
      runm("MAIN");
      break;
    }
  }

  return 0;
}
