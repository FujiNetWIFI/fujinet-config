; edmag.inc -- the library's magnifier for the editor (p5.inc): a character
; the 8244's font lacks -- lower case, most punctuation -- shown as it is,
; zoomed, in sprite 2 at the top right (T0 and row 0, where no band blanks
; it). The shapes are tools/mkglyphs.py's, in two pages (glyphsa, glyphsb:
; each read by MOVP from its own); gtab is in this one.

; mag: sprite 2 = character r5 zoomed; 0FFh, or one the font has: none.
; Clobbers A, r0-r7.
mag:
    mov a,r5
    inc a
    jz mg_off
    mov a,r5
    add a,#256-20h
    jnc mg_off
    add a,#gtab&0FFh
    movp a,@a
    inc a
    jz mg_off
    dec a
    call gl_shape
    mov r3,#2
    mov r4,#ED_MAGY
    mov r5,#ED_MAGX
    mov r2,#C_WHITE<<3|4
    jmp spr_at
mg_off:
    mov r3,#2
    mov r4,#SPR_OFF
    mov r5,#0
    mov r2,#0
    jmp spr_at

; gl_shape: sprite 2's shape = glyph A. Clobbers A, r0-r2, r5, r6.
gl_shape:
    mov r6,a
    add a,#256-GLYPHA           ; C: the second page's
    jc gs_x7
    mov a,r6
gs_x7:
    mov r5,a                    ; x 7 = x 8 - x
    rl a
    rl a
    rl a
    cpl a
    add a,r5
    cpl a
    mov r1,a
    mov a,#P1_VDC
    outl p1,a
    mov r0,#vdc_spr2_shape
    mov r2,#7
gs_l:
    mov a,r6
    add a,#256-GLYPHA
    mov a,r1
    jc gs_b
    call gla_get
    jmp gs_put
gs_b:
    call glb_get
gs_put:
    movx @r0,a
    inc r0
    inc r1
    djnz r2,gs_l
    clr a
    movx @r0,a
    mov a,#P1_MAIN
    outl p1,a
    ret

    include "glyphs.inc"
