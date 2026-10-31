; vdcreset.inc -- the boot's: the VDC to a known state. Main program.

; vdc_reset: display off and every object parked, the grid empty. Main
; program, at boot. Clobbers A, r0, r2.
vdc_reset:
    mov a,#P1_VDC
    outl p1,a
    mov r0,#vdc_control
    clr a
    movx @r0,a
    mov r0,#0                   ; $00-$9F: sprites, characters, quads, shapes
    mov r2,#0A0h
vr_obj:
    movx @r0,a
    inc r0
    djnz r2,vr_obj
    mov r0,#vdc_soundctrl
    movx @r0,a
    mov r0,#vdc_gridh0          ; $C0-$E9: the grid
    mov r2,#vdc_gridv9+1-vdc_gridh0
vr_grid:
    movx @r0,a
    inc r0
    djnz r2,vr_grid
    mov a,#P1_MAIN
    outl p1,a
    ret

