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
s_o2cfg:    db "ODYSSEY 2 CONFIG",0
s_nocart1:  db " NO FUJINET",0
s_nocart2:  db " CARTRIDGE",0
    if $ > 0C00h
    error "page 0's page $B has run out"
    endif

; ===========================================================================
    org 0C00h
; poke_at: aim the mailbox's POKE at the fixed half's address r2:A (r2 =
; $4-$7), POKE selected (screen.inc's own). Clobbers A, r0, r1.
poke_at:
    mov r1,a
    mov r0,#FN_W_REGSEL
    mov a,#FN_REG_POKE_LO
    movx @r0,a
    inc r0
    mov a,r1
    movx @r0,a
    dec r0
    mov a,#FN_REG_POKE_HI
    movx @r0,a
    inc r0
    mov a,r2
    add a,#0FCh                 ; - 4: offset from $400
    movx @r0,a
    mov r0,#FN_W_REGSEL
    mov a,#FN_REG_POKE
    movx @r0,a
    ret

; poke_list: the list at offset r4 of this page into the fixed half: each
; entry (page, offset, count, the bytes), a 0 page the end. Clobbers A,
; r0-r2, r4, r6.
poke_list:
    mov a,r4
    movp a,@a
    jz pl_x
    mov r2,a
    inc r4
    mov a,r4
    movp a,@a
    inc r4
    call poke_at
    mov a,r4
    movp a,@a
    mov r6,a
    inc r4
pl_b:
    mov a,r4
    movp a,@a
    mov r0,#FN_W_REGDATA
    movx @r0,a
    inc r4
    djnz r6,pl_b
    jmp poke_list
pl_x:
    ret

; The logo on the grid: its rows on horizontal lines 1, 3, 5 and 7, its
; columns on vertical lines 0, 2, 4, 6 and 8 (two cells a step). A disc on
; vertical line j and horizontal line k is at X 1+16j, Y 9+24k: MAME draws
; a sprite 5 units right of its X and line j at 13+16j, and line k's three
; lines from 24+24k, mid-disc.
LG_X1   equ 1+16*2
LG_X2   equ 1+16*4
LG_X3   equ 1+16*6
LG_Y0   equ 9+24*1
LG_Y1   equ 9+24*3
LG_Y2   equ 9+24*5
LG_ATTR equ C_WHITE<<3|4        ; white, zoomed
BL_LSPR equ LG_Y1+32+5          ; h_lspr: between the row of three and the
                                ; disc below it
lg_on:
    db BANDS>>8, (BANDS+3)&0FFh, 9  ; the bands after h_top's
    db J_LGRID, 0, 256-(BL_LSPR-BL_GRID)
    db J_LSPR, LG_Y2, 256-(BL_T1-BL_LSPR)
    db J_LTXT, T1&0FFh, 0
    db SCRCOL>>8, SCRCOL&0FFh, 1, BG_BLACK|GRID_WHITE
    db GRIDTAB>>8, GRIDTAB&0FFh, 10 ; the vertical traces: bit r, in row r
    db 0Ch, 00h, 3Fh, 00h, 0F1h, 00h, 0FFh, 00h, 0C0h, 00h
    db SPRTAB>>8, SPRTAB&0FFh, 12   ; the discs (Y, X, attribute):
    db LG_Y1, LG_X3, LG_ATTR        ; the right one (then the bottom one)
    db LG_Y0, LG_X2, LG_ATTR        ; the top one: h_top puts row 0's Y
    db LG_Y1, LG_X1, LG_ATTR        ; the left one
    db LG_Y1, LG_X2, LG_ATTR        ; the ring
    db ROWS>>8, (ROWS+ROW_SPR)&0FFh, 1, LG_Y0
    db 0
lg_off:                         ; sprites 0 and 1 gone from the bands too
    db BANDS>>8, (BANDS+7)&0FFh, 1, SPR_OFF
    db ROWS>>8, (ROWS+ROW_SPR)&0FFh, 1, SPR_OFF
    db 0
    if $ > 0D00h
    error "page 0's page $C has run out"
    endif

; ===========================================================================
    org 0D00h
    include "snd.inc"
    include "beep.inc"

; ===========================================================================
    org 0E00h
; splash: the FujiNet logo -- its traces the grid in lines, its discs four
; zoomed sprites (h_lspr moves sprite 0 down a row for the fifth), on the
; splash's own bands (fixed.inc) -- the name, the jingle; a key or the
; button cuts it short. Clobbers A, r0-r7.
splash:
    call frame_sync             ; (the band list changes between two frames)
    mov r4,#lg_on&0FFh
    call poke_list
    mov a,#0FFh
    call spl_h
    mov a,#P1_VDC               ; the discs' shapes: the VDC takes them any
    outl p1,a                   ; time
    mov r0,#vdc_spr0_shape
    mov r1,#disc&0FFh
spl_s:
    mov a,r0
    xrl a,#vdc_spr3_shape
    jnz spl_s1
    mov r1,#ring&0FFh           ; sprite 3: the ring
spl_s1:
    mov a,r0
    anl a,#7
    add a,r1
    movp a,@a
    movx @r0,a
    inc r0
    mov a,r0
    xrl a,#vdc_spr3_shape+8
    jnz spl_s
    mov a,#P1_MAIN
    outl p1,a
    mov r4,#L_TOP
    mov a,#s_title&0FFh
    call say_e
    mov a,#C_TAN
    call col_set
    mov r4,#L_BOT
    mov a,#s_o2cfg&0FFh
    call sayc_e
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
    mov r4,#lg_off&0FFh         ; (scr_text puts the rest back)
    call poke_list
    clr a
    call spl_h
    call spr_hide
    clr a
    jmp grid_rows

; spl_h: the logo's horizontal traces, grid registers $C0-$C8, ANDed with A
; (0FFh: the logo; 0: none, as the other screens have). They take writes
; only with the grid off: after the frame's last band. Clobbers A, r0-r3.
spl_h:
    mov r3,a
    call frame_sync
    mov a,#P1_VDC
    outl p1,a
    mov r0,#vdc_gridh0
    mov r1,#hgrid&0FFh
sh_l:
    mov a,r1
    movp a,@a
    anl a,r3
    movx @r0,a
    inc r0
    inc r1
    mov a,r0
    xrl a,#vdc_gridh8+1
    jnz sh_l
    mov a,#P1_MAIN
    outl p1,a
    ret

SPLASHF equ 150                 ; frames the splash stays
jingle:                         ; (frame, sound), up the fifths
    db 2, SND_N1, 9, SND_N2, 16, SND_N3, 23, SND_BOOT, 0
hgrid:                          ; $C0-$C8, the cells between vertical lines:
    db 08h, 2Ah, 22h, 0A2h, 0A2h ; bit k, a trace on horizontal line k
    db 0A2h, 0AAh, 0A0h, 80h
disc:                           ; sprites 0-2: 8 x 8, zoomed 16 units x 32 lines
    db 3Ch, 7Eh, 0FFh, 0FFh, 0FFh, 0FFh, 7Eh, 3Ch
ring:                           ; sprite 3: the hollow one, in the middle
    db 3Ch, 42h, 81h, 81h, 81h, 81h, 42h, 3Ch
    if $ > 0F00h
    error "page 0's page $E has run out"
    endif

    PAGE_TAIL
