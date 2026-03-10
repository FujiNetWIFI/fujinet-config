#include "../system.h"

#include <stdlib.h>
#include <i86.h>
#include "screen.h"

char response[256];

void system_boot(void)
{
    static union REGS r;
    r.h.ah = 0x00;
    r.h.al = screen_mode;
    int86(0x10, (union REGS *)&r, (union REGS *)&r);
    exit(0);
}

void system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                       uint32_t selected_size, const char *path)
{
}
