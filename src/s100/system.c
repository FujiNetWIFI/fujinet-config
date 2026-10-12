#ifdef BUILD_S100
/**
 * System routines: booting, new images, delays.
 */

#include <string.h>
#include "../typedefs.h"
#include "../system.h"
#include "../die.h"
#include "../pause.h"
#include "s100.h"

char response[256];

/* The card's boot page: page+3 loads the boot loader (image 1), which boots FDC drive 0 */
void system_boot(void)
{
    void (*go)(void) = (void (*)(void))(((uint16_t)s100.page << 8) + 3);

#ifndef S100_ROM
    /* Quit, or no page to jump to: a PHANTOM* page is off once CP/M runs */
    if (s100_quit || !s100.page || (s100.flags & S100_INFOF_PHANTOM))
    {
        if (!s100_quit)
            con_print("The boot page is off under CP/M: warm booting instead.\r\n");
        go = (void (*)(void))0;
    }
#endif
    go();
}

void system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                       uint32_t selected_size, const char *path)
{
    static NewDisk nd;

    nd.numSectors = selected_size;
    nd.sectorSize = 137;
    nd.hostSlot = selected_host_slot;
    nd.deviceSlot = selected_device_slot;
    strncpy(nd.filename, path, sizeof(nd.filename) - 1);
    fuji_create_new(&nd);
}

void pause(unsigned char delay)
{
    con_wait_ms(delay * 17); /* delay is in 60ths of a second */
}

void die(void)
{
#ifndef S100_ROM
    ((void (*)(void))0)();
#endif
    for (;;)
        ;
}

void quit(void)
{
}

#endif /* BUILD_S100 */
