#include "../system.h"

char response[256];
unsigned char system_create_type;

void system_boot(void)
{
    fuji_sleep_ms(1000);
#asm
    ld a,(0x8c1b)       ; PMD 85-1 ROM?
    cp 0xb7
    jp nz,0x8000        ; no, jump to RESET, it will auto-boot ROM Module

    ; PMD 85-1, boot ROM Module here, if present

    call 0x8c00         ; transfer data from ROM Module to RAM
    dw 0,0x10d,0xc1b2   ; 14 bytes from start to address C1B2h
    ld a,(0xc1b2)       ; is it bootable?
    cp 0xcd
    jp z,0xc1b2         ; if yes, jump to just loaded ROM Module boot code
    jp 0x8000           ; not (bootable) ROM Module -> RESET
#endasm
}

bool system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                       uint32_t selected_size, const char *path)
{
	// TODO implement
	return false;
}
