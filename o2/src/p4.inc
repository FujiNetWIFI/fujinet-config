; p4.inc -- page 4: the boot. SET_DEVICE_FULLPATH (slot 0: the bar's file,
; its name streamed from the browser's block, still in the reply), then
; MOUNT_IMAGE -- posted, not waited for: FujiNet pushes the whole image into
; the cart before it answers, and the cart's status page counts the push
; (BOOT_STATE, BOOT_PCT) while the mailbox waits. So each frame the bar is
; drawn from it; the answer ends it. Then the cart has the game staged, and
; swaps it in at the console's next fetch of $400: a reset by hand
; (fixed.inc boot_game) starts it.
;
; The bar is grid row 4 in fill mode, nine cells, the next cell's fill
; shown by sprites 2 and 3 side by side (16 steps a cell): 144 in all.

BT_ROW  equ 4                   ; the bar's grid row, and its text row
BT_Y    equ Y_ROW0+BT_ROW*ROW_PITCH
BT_COLS equ 9
BT_STALL equ 4500               ; frames without progress: 75 seconds

    PAGE_HEAD
    db e_boot&0FFh              ; E_RUN: the browser's file
    db e_mount&0FFh             ; E_MOUNT: the path is set (the lobby)
e_boot:
    jmp bt_file
e_mount:
    jmp bt_lobby

; bt_file: the screen, the file's name; SET_DEVICE_FULLPATH from the reply.
bt_file:
    call bt_screen
    call lb_clear
    mov r0,#BNAML               ; its name, on row 2
    movx a,@r0
    mov r2,a
    inc r0
    movx a,@r0
    mov r3,a
    mov r1,#LINEBUF
    mov r7,#NCOLS
    call rx_lb
    mov r4,#2
    call row_str
    mov r3,#FC_DEVPATH
    mov r4,#3
    call fuji_begin
    clr a                       ; slot 0
    call fn_par1
    mov r0,#HOST
    movx a,@r0
    call fn_par1
    mov a,#1                    ; read
    call fn_par1
    mov r0,#BNAML
    movx a,@r0
    mov r2,a
    inc r0
    movx a,@r0
    mov r3,a
    mov r7,#255
    mov r5,#0                   ; its length
bf_tx:
    call st_next
    jz bf_end
    call fn_tx
    inc r5
    djnz r7,bf_tx
bf_end:
    mov a,r5                    ; NUL-padded to 256
    cpl a
    inc a
    mov r6,a
    call fn_zeros
    call fn_go
    jz bt_go
    mov a,#s_nopath&0FFh
    jmp bt_fail

; bt_lobby: the lobby's path is set.
bt_lobby:
    call bt_screen
    mov r4,#2
    mov a,#s_lobby&0FFh
    call row_b
    ; fall into bt_go

; bt_go: MOUNT_IMAGE posted; the bar each frame until the answer.
bt_go:
    mov r3,#FC_MOUNTIMG
    mov r4,#2
    call fuji_begin
    clr a                       ; slot 0
    call fn_par1
    mov a,#1                    ; read
    call fn_par1
    call fn_post
    mov r0,#BLAST               ; the bar: nothing drawn yet
    mov a,#0FFh
    movx @r0,a
    call bt_stall
bg_l:
    call in_poll
    mov a,#FN_R_BOOT_STATE&0FFh
    call mb_get
    mov r0,#BSTATE
    movx @r0,a
    xrl a,#FN_BOOT_XFER
    jnz bg_ack
    mov a,#FN_R_BOOT_PCT&0FFh   ; the push under way: its percent
    call mb_get
    call bt_bar
bg_ack:
    call fn_done
    jz bg_done
    mov r0,#BWAITL              ; the watchdog: frames without progress
    movx a,@r0
    dec a
    movx @r0,a
    cpl a
    jnz bg_l
    inc r0
    movx a,@r0
    dec a
    movx @r0,a
    cpl a
    jnz bg_l
    mov a,#s_timeout&0FFh
    jmp bt_fail

; bg_done: the answer. ACK with the image staged (READY): the game. ACK
; with nothing pushed (IDLE: FujiNet mounted it as a disk): not a game.
; NAK: the cart's reason, if it has one.
bg_done:
    call fn_ack
    jnz bd_nak
    mov a,#FN_R_BOOT_STATE&0FFh
    call mb_get
    xrl a,#FN_BOOT_READY
    jz bt_ready
    mov a,#s_notgame&0FFh
    jmp bt_fail
bd_nak:
    mov a,#FN_R_BOOT_ERR&0FFh
    call mb_get
    mov r5,a
    xrl a,#1
    jz bd_big
    mov a,r5
    xrl a,#4
    jz bd_busy
    mov a,#s_failed&0FFh
    jmp bt_fail
bd_big:
    mov a,#s_toobig&0FFh
    jmp bt_fail
bd_busy:
    mov a,#s_busy&0FFh
    jmp bt_fail

; bt_ready: the bar full, the fanfare, then the game.
bt_ready:
    mov a,#100
    call bt_bar
    mov r4,#L_BOT
    mov a,#s_ready&0FFh
    call say_b
    mov r0,#TMP0
    clr a
    movx @r0,a
br_l:
    call in_poll
    mov r0,#TMP0
    movx a,@r0
    inc a
    movx @r0,a
    mov r5,a
    mov r1,#fanfare&0FFh        ; a note due this frame?
br_j:
    mov a,r1
    movp a,@a
    jz br_t
    xrl a,r5
    jz br_n
    inc r1
    inc r1
    jmp br_j
br_n:
    inc r1
    mov a,r1
    movp a,@a
    call beep
br_t:
    mov a,r5
    xrl a,#60
    jnz br_l
    FCALL boot_game             ; (never comes back)

fanfare:                        ; (frame, sound): the jingle again
    db 1, SND_N1, 6, SND_N2, 11, SND_N3, 16, SND_BOOT, 0

    if $ > 900h
    error "the boot's page $8 has run out"
    endif

; ===========================================================================
    org 900h

; bt_screen: the boot's screen: its title, the empty bar in its colour, the
; sprites for the bar's last cell. Clobbers A, r0-r7.
bt_screen:
    call scr_text
    mov r5,#BG_BLACK|GRID_LGREEN
    call color
    mov r4,#L_TOP
    mov a,#s_boot&0FFh
    call say_b
    mov r4,#L_BOT
    mov a,#s_loading&0FFh
    call say_b
    mov r0,#BCELLS
    clr a
    movx @r0,a
    mov r3,#2                   ; sprites 2 and 3: the bar's row, green
    mov r4,#BT_Y
    mov r5,#8
    mov r2,#C_GREEN<<3
    call spr_at
    mov r3,#3
    mov r4,#BT_Y
    mov r5,#16
    mov r2,#C_GREEN<<3
    call spr_at
    clr a
    jmp bt_sub

; bt_stall: the watchdog wound up again. Clobbers A, r0.
bt_stall:
    mov r0,#BWAITL
    mov a,#BT_STALL&0FFh
    movx @r0,a
    inc r0
    mov a,#(BT_STALL>>8)+1
    movx @r0,a
    ret

; bt_bar: the bar at A percent (0-100), redrawn only where it has moved; the
; percent on row 6, a tick each tenth. Clobbers A, r0-r7.
bt_bar:
    mov r0,#BLAST
    mov r6,a
    movx a,@r0
    xrl a,r6
    jnz bb_new
    ret
bb_new:
    mov a,r6
    movx @r0,a
    call bt_stall
    mov a,r6                    ; f = percent x 144 / 100: cells f / 16, the
    call pf_get                 ; last one's fill f mod 16
    mov r0,#TMP1
    movx @r0,a
bb_cells:
    mov r0,#TMP1                ; the cells up to f / 16
    movx a,@r0
    swap a
    anl a,#0Fh
    mov r6,a
    mov r0,#BCELLS
    movx a,@r0
    xrl a,r6
    jz bb_sub
    movx a,@r0
    mov r3,a
    inc a
    movx @r0,a
    mov r5,#1<<BT_ROW
    call grid_col
    mov a,#SND_TICK
    call beep
    jmp bb_cells
bb_sub:
    call lb_clear               ; the percent, row 6
    mov r0,#BLAST
    movx a,@r0
    mov r5,#'0'-1               ; hundreds
bb_100:
    inc r5
    add a,#256-100
    jc bb_100
    add a,#100
    mov r6,#'0'-1
bb_10:
    inc r6
    add a,#256-10
    jc bb_10
    add a,#10+'0'
    mov r0,#LINEBUF+7
    movx @r0,a
    dec r0
    mov a,r6
    movx @r0,a
    dec r0
    mov a,r5
    xrl a,#'0'
    jz bb_pct
    mov a,r5
    movx @r0,a
bb_pct:
    mov r0,#LINEBUF+8
    mov a,#'%'|80h
    movx @r0,a
    call col_white
    mov r0,#RICON
    clr a
    movx @r0,a
    mov r4,#6
    call row_line
    mov r0,#TMP1                ; then the last cell's fill
    movx a,@r0
    anl a,#0Fh
    ; fall into bt_sub

; bt_sub: the bar's last cell filled A / 16: sprites 2 and 3 at the cell
; after the full ones, A pixels from the left; none past the last cell.
; Clobbers A, r0-r7.
bt_sub:
    mov r6,a
    mov r0,#BCELLS              ; the cell's X: 8 + 16 x cells
    movx a,@r0
    mov r7,a
    xrl a,#BT_COLS
    jnz bs_on
    mov r6,#0
bs_on:
    mov a,r7
    swap a
    add a,#8
    mov r5,a
    mov r3,#2
    mov r4,#BT_Y
    mov r2,#C_GREEN<<3
    mov r0,#TMP2
    mov a,r6
    movx @r0,a
    call spr_at
    mov r0,#TMP2
    movx a,@r0
    mov r6,a
    mov r3,#3
    mov a,r5
    add a,#8
    mov r5,a
    mov r4,#BT_Y
    mov r2,#C_GREEN<<3
    call spr_at
    mov r0,#TMP2                ; the shapes: n pixels, 8 to a sprite
    movx a,@r0
    mov r6,a
    add a,#256-8
    jc bs_two
    mov a,r6                    ; under 8: sprite 2 has them, 3 none
    call bs_px
    mov r5,a
    mov r4,#0
    jmp bs_put
bs_two:
    call bs_px
    mov r4,a
    mov r5,#0FFh
bs_put:
    mov a,#P1_VDC
    outl p1,a
    mov r0,#vdc_spr2_shape
    mov r2,#7
bs_s2:
    mov a,r5
    movx @r0,a
    inc r0
    djnz r2,bs_s2
    clr a
    movx @r0,a
    mov r0,#vdc_spr3_shape
    mov r2,#7
bs_s3:
    mov a,r4
    movx @r0,a
    inc r0
    djnz r2,bs_s3
    clr a
    movx @r0,a
    mov a,#P1_MAIN
    outl p1,a
    ret

    if $ > 0A00h
    error "the boot's page $9 has run out"
    endif

; ===========================================================================
    org 0A00h
; pf_get: A = pctfill[A].
pf_get:
    add a,#pctfill&0FFh
    movp a,@a
    ret
pctfill:                        ; percent x 144 / 100: the bar's 144 steps
K   set 0
    rept 101
    db K*144/100
K   set K+1
    endm

    include "local.inc"
    if $ > 0B00h
    error "the boot's page $A has run out"
    endif

; ===========================================================================
    org 0B00h
    STRPAGE b
s_boot:     db "     LOADING",0
s_loading:  db "   PLEASE WAIT",0
s_ready:    db "     READY",0
s_lobby:    db "GAME LOBBY",0
s_anykey:   db "   PRESS A KEY",0
s_nopath:   db " BAD PATH",0
s_timeout:  db " NO ANSWER",0
s_notgame:  db " NOT A GAME",0
s_toobig:   db " TOO BIG",0
s_busy:     db " POWER OFF+ON",0
s_failed:   db " LOAD FAILED",0

; bt_fail: the reason (the string at offset A of this page) on row 6, then a
; key: the folder again.
bt_fail:
    mov r6,a
    mov a,#SND_ERROR
    call beep
    mov a,r6
    mov r4,#6
    call row_b
    mov r4,#L_BOT
    mov a,#s_anykey&0FFh
    call say_b
bf_wait:
    call in_poll
    mov r0,#KEY
    movx a,@r0
    cpl a
    jnz bf_back
    mov r0,#JOYEV
    movx a,@r0
    jb4 bf_back
    jmp bf_wait
bf_back:
    GOTO PG_FILES,E_PAGE

; bs_px: A = A pixels from the left (bit 0 the leftmost), 0-8.
bs_px:
    add a,#pxtab&0FFh
    movp a,@a
    ret
pxtab:
    db 00h, 01h, 03h, 07h, 0Fh, 1Fh, 3Fh, 7Fh, 0FFh

    if $ > 0C00h
    error "the boot's page $B has run out"
    endif

; ===========================================================================
    org 0D00h
    include "mail.inc"
    include "beep.inc"

; ===========================================================================
    org 0E00h
    include "input.inc"
    include "snd.inc"

    PAGE_TAIL
