#include "../system.h"

char response[256];
unsigned char system_create_type;

void system_boot(void)
{
#asm
    jmp 0x8000
#endasm
}

bool system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                       uint32_t selected_size, const char *path)
{
	// TODO implement
	return false;
}
