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

void system_drop_into_basic(void)
{
  vdp_set_mode(0);

  __asm
    CALSLT	equ	001Ch
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

  	;ld	ix,(TEMPST)	; Erases 3 bytes
  	;ld	(ix),0	; from the
  	;ld	(ix+1),0	; area reserved for
  	;ld	(ix+2),0	; the BASIC program

  	ld	ix,BASENT
  	ld	iy,(MASTER-1)	; Slot of the Master Disk-ROM
    ; ld	iy,MASTER
  	jp	CALSLT
  __endasm;
}
