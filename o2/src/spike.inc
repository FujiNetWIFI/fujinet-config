; spike.inc -- page 0 of a SPIKE build: the engine's test pattern. Every row
; full -- twelve characters, mixed case and look-alikes, a folder on the
; odd rows -- the bar on one row (the stick moves it), T0 and T1. No
; network. For measuring a band's slack: PADN pads a row band (build.sh),
; and tools/rowcheck.py counts the frames that lost a line.

    PAGE_HEAD
    db e_boot&0FFh
e_boot:
    jmp boot

boot:
    call vdc_reset
    call ints_on
    call scr_text
    call folder_shape
    mov r4,#L_TOP
    mov a,#s_t0&0FFh
    call say_b
    mov r4,#L_BOT
    mov a,#s_t1&0FFh
    call say_b
    mov r4,#0
sp_row:
    mov r0,#TMP1
    mov a,r4
    movx @r0,a
    mov r0,#RICON               ; a folder on the odd rows
    anl a,#1
    movx @r0,a
    call spk_line
    mov r0,#TMP1
    movx a,@r0
    inc a
    mov r4,a
    xrl a,#NROWS
    jnz sp_row
    mov r0,#TMP1                ; the bar
    mov a,#3
    movx @r0,a
    call bar_row
sp_loop:
    call frame_sync
    if SPKNOIN=0
    call key_poll
    call joy_poll
    endif
    mov r0,#JOYEV
    movx a,@r0
    mov r5,a
    mov r0,#TMP1
    movx a,@r0
    mov r6,a
    mov a,r5
    jb0 sp_up
    jb1 sp_dn
    jmp sp_loop
sp_up:
    mov a,r6
    dec a
    jmp sp_bar
sp_dn:
    mov a,r6
    inc a
sp_bar:
    anl a,#7
    movx @r0,a
    call bar_row
    jmp sp_loop

ints_on:
    clr a
    mov t,a
    strt cnt
    en tcnti
    en i
    ret

; folder_shape: sprite 1 is a folder, tan. The VDC takes shapes with the
; display on. Clobbers A, r0, r1, r2.
folder_shape:
    mov a,#P1_VDC
    outl p1,a
    mov r0,#vdc_spr1_shape
    mov r1,#fold&0FFh
    mov r2,#8
fsh_l:
    mov a,r1
    movp a,@a
    movx @r0,a
    inc r0
    inc r1
    djnz r2,fsh_l
    mov a,#P1_MAIN
    outl p1,a
    mov r3,#1
    mov r4,#SPR_OFF
    mov r5,#X_ICON
    mov r2,#C_TAN<<3
    jmp spr_at
fold:                           ; bit 0 is the leftmost pixel
    db 07h, 7Fh, 7Fh, 7Fh, 7Fh, 7Fh, 00h, 00h

; spk_line: row r4 = spk_rows' line r4. Clobbers A, r0-r7.
spk_line:
    mov a,r4
    rl a
    rl a
    mov r5,a
    rl a
    add a,r5                    ; 12r
    add a,#spk_rows&0FFh
    mov r1,a
    mov r0,#LINEBUF
    mov r2,#12
spl_l:
    mov a,r1
    movp a,@a
    movx @r0,a
    inc r0
    inc r1
    djnz r2,spl_l
    call col_white
    jmp row_line

spk_rows:
    db "1 SD        "
    db "2 tnfs.io/x "
    db "Games/      "
    db "K.C. Munchki"
    db "!#%&()*,;<>@"
    db "[]^_`{|}~\\$:"
    db "a-z: abcdefg"
    db "0123456789?="

; ===========================================================================
    org 900h
    include "disp.inc"

; ===========================================================================
    org 0A00h
    include "input.inc"
    include "vdcreset.inc"

; ===========================================================================
    org 0B00h
    include "text.inc"
    STRPAGE b

s_t0:   db "FUJINET  CONFIG",0
s_t1:   db "E EDIT ? HELP",0
    if $ > 0C00h
    error "spike's page $B has run out"
    endif

; ===========================================================================
    org 0C00h
    include "screen.inc"

    PAGE_TAIL
