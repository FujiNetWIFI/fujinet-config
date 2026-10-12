#ifdef BUILD_H89

/**
 * @file con.c
 * @brief Raw console I/O - Heathkit H89, HDOS or CP/M.
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 *
 * Console bytes go through the OS raw: HDOS's .SCOUT/.SCIN with the console
 * driver in raw character mode, or the CP/M BIOS's CONOUT/CONST/CONIN
 * straight from its jump table (the BDOS console calls expand tabs, watch for
 * ^S and hold back characters). Both OSes buffer the keyboard on its
 * interrupt, so nothing typed between polls is lost.
 */

#include <string.h>
#include "con.h"

#ifdef __HDOS__

/* HDOS system calls: RST 38H, then the function byte. HDOS is 8080 code
   and leaves IX and IY alone. */
void con_out(uint8_t c) __z88dk_fastcall __naked
{
#asm
    ld      a,l
    rst     38h
    defb    2           ; .SCOUT
    ret
#endasm
}

int con_in(void) __naked
{
#asm
    rst     38h
    defb    1           ; .SCIN - carry set when nothing is waiting
    ld      hl,-1
    ret     c
    ld      l,a
    ld      h,0
    ret
#endasm
}

/* .CONSL: A = index of the console byte, B = the bits to set, C = the mask
   of the bits to change */
uint8_t h89_consl_index, h89_consl_bits, h89_consl_mask;

static void consl(void) __naked
{
#asm
    ld      a,(_h89_consl_bits)
    ld      b,a
    ld      a,(_h89_consl_mask)
    ld      c,a
    ld      a,(_h89_consl_index)
    rst     38h
    defb    6           ; .CONSL
    ret
#endasm
}

static void consl_set(uint8_t index, uint8_t bits, uint8_t mask)
{
  h89_consl_index = index;
  h89_consl_bits = bits;
  h89_consl_mask = mask;
  consl();
}

/* The five console bytes (S.CSLMD..S.CONFL) in HDOS 2.0's system values */
#define S_CSLMD ((volatile uint8_t *)0x20D6)
#define I_CSLMD 0
#define I_CONTY 1
#define I_CUSOR 2
#define I_CONWI 3
#define I_CONFL 4

#define CSL_ECH 0x80 /* suppress echo */
#define CSL_RAW 0x04 /* raw mode I/O: no ^C, ^S, ^O or ^P processing */
#define CSL_WRP 0x02 /* wrap lines at the width */
#define CSL_CHR 0x01 /* character mode */
#define CTP_MLI 0x20 /* map lower case to upper on input */
#define CTP_MLO 0x10 /* map lower case to upper on output */
#define CTP_BKM 0x02 /* map backspace to rubout on input */
#define CO_FLG  0x01 /* ^O (discard output) flag */
#define CS_FLG  0x80 /* ^S (hold output) flag */

static uint8_t saved_console[5];

void con_init(void)
{
  memcpy(saved_console, (void *)S_CSLMD, sizeof(saved_console));

  /* Raw character mode with no echo, no wrap and no width (HDOS would count
     escape sequences as printing columns), and no case mapping: graphics
     characters go out as lower case letters, and host names and passwords
     are typed in both cases. */
  consl_set(I_CSLMD, CSL_ECH | CSL_RAW | CSL_CHR, CSL_ECH | CSL_RAW | CSL_WRP | CSL_CHR);
  consl_set(I_CONTY, 0, CTP_MLI | CTP_MLO | CTP_BKM);
  consl_set(I_CONWI, 255, 0xFF);
  consl_set(I_CONFL, 0, CO_FLG | CS_FLG);
}

void con_exit(void)
{
  consl_set(I_CSLMD, saved_console[I_CSLMD], 0xFF);
  consl_set(I_CONTY, saved_console[I_CONTY], 0xFF);
  consl_set(I_CONWI, saved_console[I_CONWI], 0xFF);
  consl_set(I_CUSOR, 0, 0xFF); /* the terminal reset homed the cursor */
}

#else /* CP/M */

/* The BIOS jump table: (0001) points at its WBOOT entry, base+3. CONST is
   +3 from there, CONIN +6, CONOUT +9. The BIOS may use any register, so IX
   and IY are kept for the C side. */
void con_out(uint8_t c) __z88dk_fastcall __naked
{
#asm
    EXTERN  l_dcal
    push    ix
    push    iy
    ld      c,l
    ld      hl,(1)
    ld      de,9
    add     hl,de
    call    l_dcal      ; CONOUT
    pop     iy
    pop     ix
    ret
#endasm
}

int con_in(void) __naked
{
#asm
    EXTERN  l_dcal
    push    ix
    push    iy
    ld      hl,(1)
    ld      de,3
    add     hl,de
    call    l_dcal      ; CONST: A = FF when a character is waiting
    or      a
    ld      hl,-1
    jr      z,con_in_done
    ld      hl,(1)
    ld      de,6
    add     hl,de
    call    l_dcal      ; CONIN
    ld      l,a
    ld      h,0
con_in_done:
    pop     iy
    pop     ix
    ret
#endasm
}

void con_init(void)
{
}

void con_exit(void)
{
}

#endif /* __HDOS__ */

#endif /* BUILD_H89 */
