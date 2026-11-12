#include <conio.h>
#include "../system.h"

char response[256];

void system_boot(void)
{
  fuji_enable_user_rom();
}

bool system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                       uint32_t selected_size, const char *path)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#endif
    return false;
}
