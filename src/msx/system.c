#include "../system.h"

char response[256];

void system_boot(void)
{
  return;
}

void system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                       uint32_t selected_size, const char *path)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    // last_rc = fujinet_create_new(selected_host_slot, selected_device_slot, selected_size, path);
#endif
}

#define RG9SAV 0xFFE8

// default value
uint8_t system_fps = 60;

void system_set_fps()
{
  unsigned char *rg9sav_addr = RG9SAV;
  system_fps = *rg9sav_addr & 0b10 ? 50 : 60;
}
