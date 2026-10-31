; disp.inc -- characters to the VDC's pointers: gptr, and tx_build for the
; text lines T0 and T1. Main program, P1 = P1_MAIN, register bank 0.
;
; One 256-byte page, placed by the includer: gptr's and tx_build's MOVPs
; read charmap and txorder out of the page they run in.
;
; A character in LINEBUF: $20-$7F ASCII; $00-$1F glyph code+32 (the
; pictures: CH_*); bit 7 set: drawn in TXCOL2 whatever it is.

LCCOL   equ C_CYAN              ; lower case (charmap class 1)
SUBCOL  equ C_VIOLET            ; a look-alike for one the font lacks (2)

; gptr: the character pointer for code A (bit 7 clear) on the line whose bias
; is r5 (-(Y/2)): low byte in r3, bit 8 in r1; its colour in r2 (TXCOL,
; LCCOL, SUBCOL). The VDC adds Y/2 back while it draws, so the register
; holds glyph*8 - Y/2, nine bits. Clobbers A, r0.
gptr:
    add a,#256-20h
    jc gp_tab
    add a,#20h+32               ; $00-$1F: glyph code+32, the line's colour
    mov r0,a
    mov r2,#0
    jmp gp_g
gp_tab:
    add a,#charmap&0FFh
    movp a,@a
    mov r0,a
    rl a
    rl a
    anl a,#3
    mov r2,a                    ; the class
    mov a,r0
    anl a,#3Fh
    mov r0,a                    ; the glyph, 0-63
gp_g:
    rl a                        ; glyph*8: bits 7-3 of the low byte ...
    rl a
    rl a
    anl a,#0F8h
    add a,r5                    ; ... less Y/2; C = no borrow
    mov r3,a
    mov a,r0                    ; ... and bit 8 is the glyph's bit 5,
    swap a                      ; less the borrow
    rr a
    anl a,#1
    jc gp_nb
    xrl a,#1
gp_nb:
    mov r1,a
    mov a,r2                    ; the class's colour
    jz gp_c0
    dec a
    jz gp_c1
    mov r2,#SUBCOL
    ret
gp_c1:
    mov r2,#LCCOL
    ret
gp_c0:
    mov r0,#TXCOL
    movx a,@r0
    mov r2,a
    ret

; gctl: A = the control byte for code r7 (bit 7: TXCOL2) whose gptr has
; run: the colour << 1 | pointer bit 8. Clobbers A, r0.
gctl:
    mov a,r7
    jb7 gc_2
    mov a,r2
    jmp gc_c
gc_2:
    mov r0,#TXCOL2
    movx a,@r0
gc_c:
    rl a
    orl a,r1
    ret

; tx_build: LINEBUF into the text table at r2:r3, for the line at Y = r4.
; The table's order is the writer's: quad A takes columns 0, 2, 4, 6, B 1, 3,
; 5, 7, C and D the next eight (txorder). Clobbers A, r0-r7.
tx_build:
    mov a,r4
    rr a
    cpl a
    inc a
    mov r5,a                    ; the bias: -(Y/2)
    mov a,r3
    call poke_at
    mov a,r4
    call poke_b                 ; Y
    mov r6,#0
txb_l:
    mov a,r6
    add a,#txorder&0FFh
    movp a,@a
    add a,#LINEBUF
    mov r0,a
    movx a,@r0
    mov r7,a
    anl a,#7Fh
    call gptr
    mov a,r3
    call poke_b                 ; pointer, low byte
    call gctl
    call poke_b                 ; colour and pointer bit 8
    inc r6
    mov a,r6
    xrl a,#TXT_LEN
    jnz txb_l
    ret

; lb_clear: LINEBUF to spaces. Clobbers A, r0, r2.
lb_clear:
    mov r0,#LINEBUF
    mov r2,#TXT_LEN
    mov a,#' '
lbc_l:
    movx @r0,a
    inc r0
    djnz r2,lbc_l
    ret

txorder:
    db 0,2,4,6, 1,3,5,7, 8,10,12,14, 9,11,13,15
    include "charmap.inc"
