; p0.inc -- page 0: the boot. The cartridge, the splash, then the WiFi check
; (page 5), one way.

    PAGE_HEAD
    db e_boot&0FFh              ; entry 0: the boot (fixed.inc start)
e_boot:
    jmp boot

boot:
    call vdc_reset
    call ram_clear
    call ints_on
    call scr_text
    call pal_check
    mov a,#FN_R_MAGIC0&0FFh     ; a FujiNet cartridge, speaking protocol 2?
    call mb_get
    xrl a,#'F'
    jnz bt_nocart
    mov a,#FN_R_MAGIC1&0FFh
    call mb_get
    xrl a,#'N'
    jnz bt_nocart
    mov a,#FN_R_PROTO_VER&0FFh
    call mb_get
    add a,#256-FN_PROTO_VER
    jnc bt_nocart
    call splash
    GOTO PG_WIFI,E_CHECK

bt_nocart:
    mov r4,#L_TOP
    mov a,#s_title&0FFh
    call say_e
    mov r4,#2
    mov a,#s_nocart1&0FFh
    call row_e
    mov r4,#3
    mov a,#s_nocart2&0FFh
    call row_e
    mov a,#SND_ERROR
    call beep
bt_stop:
    call in_poll
    jmp bt_stop

; ram_clear: external RAM from TMP0 on to 0, KLAST and KEY to none.
; Clobbers A, r0, r2.
ram_clear:
    mov r0,#TMP0
    mov r2,#80h-TMP0
    clr a
rcx_l:
    movx @r0,a
    inc r0
    djnz r2,rcx_l
    mov a,#K_NONE
    mov r0,#KLAST
    movx @r0,a
    mov r0,#KEY
    movx @r0,a
    ret

; pal_check: a PAL console (MODE_PAL): its frame is a third longer, so a
; frame of the main program's loop counts more passes -- with the bands
; running, as they always are. Loops a frame in TMP1:TMP2 (low, high).
; Clobbers A, r0-r5.
pal_check:
    call frame_wait             ; (a screen just set up: a frame to settle)
    call frame_wait
    call frame_wait
    mov r1,#FRAMES
    mov a,@r1
    mov r2,a
    mov r4,#0
    mov r5,#0
pc_l:                           ; 8 cycles a pass
    inc r4
    mov a,r4
    jnz pc_n
    inc r5
pc_n:
    mov a,@r1
    xrl a,r2
    jz pc_l
    mov r0,#TMP1
    mov a,r4
    movx @r0,a
    inc r0
    mov a,r5
    movx @r0,a
    mov a,r4                    ; PALPASS or more: PAL
    add a,#(10000h-PALPASS)&0FFh
    mov a,r5
    addc a,#(10000h-PALPASS)>>8
    jnc pc_x
    mov r1,#MODE
    mov a,@r1
    orl a,#MODE_PAL
    mov @r1,a
pc_x:
    ret

ints_on:
    clr a
    mov t,a
    strt cnt
    en tcnti
    en i
    ret

    if $ > 900h
    error "page 0's page $8 has run out"
    endif

; ===========================================================================
    org 900h
    include "local.inc"

; ===========================================================================
    org 0A00h
    include "input.inc"
    include "vdcreset.inc"

; ===========================================================================
    org 0B00h
    STRPAGE e
s_title:    db "    FUJINET",0
s_sys:      db " ODYSSEY 2",0
s_config:   db "   CONFIG",0
s_ver:      db "  V",CFG_VER,0
s_presskey: db "   PRESS A KEY",0
s_nocart1:  db " NO FUJINET",0
s_nocart2:  db " CARTRIDGE",0
    if $ > 0C00h
    error "page 0's page $B has run out"
    endif

; ===========================================================================
; ===========================================================================
    org 0D00h
    include "snd.inc"
    include "beep.inc"

; ===========================================================================
    org 0E00h
; splash: the mountain -- the grid in fill mode, rows 1-3 (a band blanks
; the foreground between rows, never the grid), snow on its peak (sprites 0
; and 2, white, in row 1) -- the name, the jingle; a key or the button cuts
; it short. Clobbers A, r0-r7.
splash:
    mov r5,#BG_BLACK|GRID_LBLUE
    call color
    mov r3,#0                   ; the grid's ten columns
spl_g:
    mov a,r3
    add a,#mount&0FFh
    movp a,@a
    mov r5,a
    mov r0,#TMP1
    mov a,r3
    movx @r0,a
    call grid_col
    mov r0,#TMP1
    movx a,@r0
    inc a
    mov r3,a
    xrl a,#10
    jnz spl_g
    mov a,#P1_VDC               ; the snow: the VDC takes shapes any time
    outl p1,a
    mov r0,#vdc_spr0_shape
    mov r1,#snow&0FFh
    mov r2,#8
spl_s0:
    mov a,r1
    movp a,@a
    movx @r0,a
    inc r0
    inc r1
    djnz r2,spl_s0
    mov r0,#vdc_spr2_shape
    mov r2,#8
spl_s2:
    mov a,r1
    movp a,@a
    movx @r0,a
    inc r0
    inc r1
    djnz r2,spl_s2
    mov a,#P1_MAIN
    outl p1,a
    mov r3,#0
    mov r4,#Y_ROW0+ROW_PITCH
    mov r5,#X_SNOW
    mov r2,#C_WHITE<<3
    call spr_at
    mov r3,#2
    mov r4,#Y_ROW0+ROW_PITCH
    mov r5,#X_SNOW+8
    mov r2,#C_WHITE<<3
    call spr_at
    mov r4,#L_TOP
    mov a,#s_title&0FFh
    call say_e
    mov a,#C_TAN
    call col_set
    mov r4,#4
    mov a,#s_sys&0FFh
    call rowc_e
    mov r4,#5
    mov a,#s_config&0FFh
    call row_e
    mov a,#C_DGREY
    call col_set
    mov r4,#7
    mov a,#s_ver&0FFh
    call rowc_e
    mov r4,#L_BOT
    mov a,#s_presskey&0FFh
    call say_e
    mov r0,#TMP0                ; the frame count
    clr a
    movx @r0,a
spl_l:
    call in_poll
    mov r0,#KEY
    movx a,@r0
    cpl a
    jnz spl_x                   ; a key
    mov r0,#JOYEV
    movx a,@r0
    jb4 spl_x                   ; the button
    mov r0,#TMP0
    movx a,@r0
    inc a
    movx @r0,a
    mov r5,a
    mov r1,#jingle&0FFh         ; a note due this frame?
spl_j:
    mov a,r1
    movp a,@a
    jz spl_t
    xrl a,r5
    jz spl_n
    inc r1
    inc r1
    jmp spl_j
spl_n:
    inc r1
    mov a,r1
    movp a,@a
    call beep
spl_t:
    mov a,r5
    xrl a,#SPLASHF
    jnz spl_l
spl_x:
    call spr_hide
    clr a
    jmp grid_rows

SPLASHF equ 150                 ; frames the splash stays
X_SNOW  equ 80                  ; the snow: two sprites over the peak
jingle:                         ; (frame, sound), up the fifths
    db 2, SND_N1, 9, SND_N2, 16, SND_N3, 23, SND_BOOT, 0
mount:                          ; the grid's columns: bit r, row r filled
    db 00h, 00h, 08h, 0Ch, 0Eh, 0Eh, 0Ch, 08h, 00h, 00h
snow:                           ; sprites 0 and 2: the peak's cap, 16 wide
    db 80h, 0C0h, 0E0h, 0F0h, 0F8h, 0D8h, 088h, 00h
    db 01h, 03h, 07h, 0Fh, 1Fh, 1Bh, 011h, 00h
    if $ > 0F00h
    error "page 0's page $E has run out"
    endif

    PAGE_TAIL
