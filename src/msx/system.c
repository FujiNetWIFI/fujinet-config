#include <conio.h>
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

<<<<<<< HEAD
#define RG9SAV 0xFFE8

// default value
uint8_t system_fps = 60;

void system_set_fps()
{
  unsigned char *rg9sav_addr = RG9SAV;
  system_fps = *rg9sav_addr & 0b10 ? 50 : 60;
=======
// TODO: Make sure this is always between 0x4000 and 0x7FFF
void system_drop_into_basic(void)
{
  vdp_set_mode(1);
  clrscr();

  __asm
    CALSLT	equ	001Ch
    ENASLT  equ 0024h
    DTA	    equ	0080h
    CALBAS	equ	0159h
    BASENT	equ	04022h
    REBOOT	equ	0F340h
    TEMPST	equ	0F67Ah
    MASTER	equ	0F348h

  	ld	a,1	; Not 0 to ignore the AUTOEXEC.BAS if present
  	ld	(REBOOT),a

  	ld	a,0
  	ld	(DTA),a	; No file name

    ld  a,3
    ld  h,0b01000000
    jp  ENASLT

  	ld	ix,(TEMPST)	; Erases 3 bytes
  	ld	(ix),0	; from the
  	ld	(ix+1),0	; area reserved for
  	ld	(ix+2),0	; the BASIC program

  	ld	ix,BASENT
  	ld	iy,(MASTER-1)	; Slot of the Master Disk-ROM
  	jp	CALSLT
  __endasm;
>>>>>>> 2b8918f (msx: wip logo bg)
}
