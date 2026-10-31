; p2.inc -- page 2: the hosts. Eight slots from READ_HOST_SLOTS (8 x 32 in
; the reply, which stays there while the screen is up: the bar's long name
; scrolls straight out of it). Open one (the button, ENT, 1-8), rename it
; (E: the editor, then WRITE_HOST_SLOTS: the slots re-read and streamed back
; with the new name in), the lobby (L), info (I), WiFi (W), help (H, ?).
; Copying (FL_COPY), this is where the copy goes: CLR gives up.

HNAMEW  equ NCOLS-2             ; a name's characters on its row
E_HEDIT equ 1                   ; entry 1: the editor is done

    PAGE_HEAD
    db e_hosts&0FFh             ; E_RUN
    db e_hedit&0FFh             ; E_HEDIT
e_hosts:
    jmp hs_run
e_hedit:
    jmp hs_edited

; hs_loop: the stick or the keys move the bar; the long name scrolls.
hs_loop:
    call in_poll
    call hs_marq
    mov r0,#JOYEV
    movx a,@r0
    jb0 hs_up
    jb1 hs_down
    jb4 hs_open_j
    mov r0,#KEY
    movx a,@r0
    mov r5,a
    xrl a,#K_ENT
    jz hs_open_j
    mov a,r5
    xrl a,#K_E
    jz hs_edit_j
    mov a,r5
    xrl a,#K_CLR
    jz hs_clr
    mov a,r5
    xrl a,#K_H
    jz hs_help
    mov a,r5
    xrl a,#K_QUEST
    jz hs_help
    mov a,r5
    xrl a,#K_L
    jz hs_lobby
    mov a,r5
    xrl a,#K_I
    jz hs_info
    mov a,r5
    xrl a,#K_W
    jz hs_wifi
    mov a,r5                    ; 1-8: that slot, opened
    jz hs_loop
    add a,#256-9
    jc hs_loop
    mov a,r5
    dec a
    call hs_sel
    jmp hs_open
hs_open_j:
    jmp hs_open
hs_edit_j:
    jmp hs_edit

hs_up:
    mov r0,#SEL
    movx a,@r0
    jz hs_loop
    dec a
    call hs_sel
    jmp hs_loop
hs_down:
    mov r0,#SEL
    movx a,@r0
    xrl a,#NROWS-1
    jz hs_loop
    movx a,@r0
    inc a
    call hs_sel
    jmp hs_loop

hs_clr:                         ; copying: give it up
    mov r0,#FLAGS
    movx a,@r0
    jb0 hs_uncopy
    jmp hs_loop
hs_uncopy:
    anl a,#~FL_COPY
    movx @r0,a
    mov a,#SND_BACK
    call beep
    jmp hs_run
hs_help:
    HELP HC_HOSTS,PG_HOSTS,E_RUN
hs_lobby:
    call hs_nocp
    GOTO PG_MISC,E_LOBBY
hs_info:
    GOTO PG_MISC,E_INFO
hs_wifi:
    call hs_nocp
    GOTO PG_WIFI,E_RUN

; hs_nocp: no longer copying. Clobbers A, r0.
hs_nocp:
    mov r0,#FLAGS
    movx a,@r0
    anl a,#~FL_COPY
    movx @r0,a
    ret

; hs_sel: the bar to slot A: the row it leaves drawn from the start, the
; scroll from the start. Clobbers A, r0-r7.
hs_sel:
    mov r0,#TMP0
    movx @r0,a
    mov r0,#SEL
    movx a,@r0
    mov r4,a
    mov a,#SND_MOVE
    call beep
    mov r0,#MQOFF               ; the old row, from its name's start
    clr a
    movx @r0,a
    call hs_row
    mov r0,#TMP0
    movx a,@r0
    mov r0,#SEL
    movx @r0,a
    mov r0,#MQCLK
    mov a,#MQ_WAIT
    movx @r0,a
    inc r0                      ; MQDIR
    mov a,#1
    movx @r0,a
    mov r0,#SEL
    movx a,@r0
    jmp bar_row

    if $ > 900h
    error "the hosts' page $8 has run out"
    endif

; ===========================================================================
    org 900h

; hs_run: the screen: READ_HOST_SLOTS, the eight rows, the bar.
hs_run:
    mov r0,#HOST                ; the bar on the host last opened
    movx a,@r0
    anl a,#NROWS-1
    mov r0,#SEL
    movx @r0,a
    call scr_text
    mov r4,#L_TOP
    mov r0,#FLAGS
    movx a,@r0
    jb0 hr_copy
    mov a,#s_hosts&0FFh
    call say_h
    mov r4,#L_BOT
    mov a,#s_hlegend&0FFh
    call sayk_h
    jmp hr_list
hr_copy:
    mov a,#s_copyto&0FFh
    call say_h
    mov r4,#L_BOT
    mov a,#s_clegend&0FFh
    call sayk_h
hr_list:
    call hs_read
    mov r0,#MQOFF
    clr a
    movx @r0,a
    mov r4,#0
hr_row:
    mov r0,#TMP0
    mov a,r4
    movx @r0,a
    call hs_row
    mov r0,#TMP0
    movx a,@r0
    inc a
    mov r4,a
    xrl a,#NROWS
    jnz hr_row
    mov r0,#MQCLK
    mov a,#MQ_WAIT
    movx @r0,a
    inc r0
    mov a,#1
    movx @r0,a
    mov r0,#SEL
    movx a,@r0
    call bar_row
    jmp hs_loop

; hs_read: READ_HOST_SLOTS into the reply. A = 0 if it worked. Clobbers A,
; r0-r7.
hs_read:
    mov r3,#FC_RDHOSTS
    mov r4,#0
    call fuji_begin
    jmp fn_go

; hs_row: row r4 = slot r4: its number (tan), its name from MQOFF if it's
; the bar's, else from the start; "--" grey when empty. Clobbers A, r0-r7.
hs_row:
    mov a,r4
    mov r0,#TMP1
    movx @r0,a
    call lb_clear
    mov r0,#TMP1
    movx a,@r0
    add a,#'1'|80h
    mov r0,#LINEBUF
    movx @r0,a
    mov r0,#TMP1                ; the name: reply 32 x slot
    movx a,@r0
    swap a
    rl a
    mov r2,a
    mov r3,#0
    call st_byte
    jz hw_empty
    mov r0,#TMP1                ; the bar's row: from MQOFF
    movx a,@r0
    mov r6,a
    mov r0,#SEL
    movx a,@r0
    xrl a,r6
    jnz hw_name
    mov r0,#MQOFF
    movx a,@r0
    add a,r2
    mov r2,a
hw_name:
    mov r1,#LINEBUF+2
    mov r7,#HNAMEW
    call rx_lb
    mov a,#C_WHITE
    jmp hw_out
hw_empty:
    mov r0,#LINEBUF+2
    mov a,#'-'
    movx @r0,a
    inc r0
    movx @r0,a
    mov a,#C_DGREY
hw_out:
    mov r0,#TXCOL
    movx @r0,a
    inc r0
    mov a,#C_TAN
    movx @r0,a
    mov r0,#RICON
    clr a
    movx @r0,a
    mov r0,#TMP1
    movx a,@r0
    mov r4,a
    jmp row_line

; hs_marq: the bar's name, if it's longer than its row: after MQ_WAIT
; frames still, a character every MQ_STEP, to its end and back.
; Clobbers A, r0-r7.
hs_marq:
    mov r0,#MQCLK
    movx a,@r0
    dec a
    movx @r0,a
    jz hm_step
    ret
hm_step:
    mov a,#MQ_STEP
    movx @r0,a
    mov r0,#SEL                 ; its length: up to the NUL, 32 at most
    movx a,@r0
    swap a
    rl a
    mov r2,a
    mov r3,#0
    mov r5,#0
hm_len:
    call st_byte
    jz hm_have
    inc r2
    inc r5
    mov a,r5
    xrl a,#32
    jnz hm_len
hm_have:
    mov a,r5                    ; r5 - HNAMEW: the last offset (C: room)
    add a,#256-HNAMEW-1
    jc hm_long
    ret
hm_long:
    inc a
    mov r6,a                    ; the last offset
    mov r0,#MQDIR
    movx a,@r0
    mov r7,a
    mov r0,#MQOFF
    movx a,@r0
    add a,r7
    movx @r0,a
    jz hm_end                   ; back at the start
    xrl a,r6
    jz hm_end                   ; at the end
    jmp hm_draw
hm_end:
    mov r0,#MQDIR               ; turn round, and rest there
    movx a,@r0
    cpl a
    inc a
    movx @r0,a
    mov r0,#MQCLK
    mov a,#MQ_WAIT
    movx @r0,a
hm_draw:
    mov r0,#SEL
    movx a,@r0
    mov r4,a
    jmp hs_row

    if $ > 0A00h
    error "the hosts' page $9 has run out"
    endif

; ===========================================================================
    org 0A00h

; hs_open: the slot on the bar: an empty one is named first. Otherwise
; MOUNT_HOST, and its folder from the top (SET_HOST_PREFIX "/"): the
; browser.
hs_open:
    mov a,#SND_SELECT
    call beep
    mov r0,#SEL
    movx a,@r0
    swap a
    rl a
    mov r2,a
    mov r3,#0
    call st_byte
    jz hs_edit
    mov r0,#SEL
    movx a,@r0
    mov r0,#HOST
    movx @r0,a
    mov r4,#3
    mov a,#s_mount&0FFh
    call row_h
    mov r3,#FC_MOUNTHOST
    mov r4,#1
    call fuji_begin
    mov r0,#HOST
    movx a,@r0
    call fn_par1
    call fn_go
    jnz ho_fail
    mov r3,#FC_HPREFIXSET
    mov r4,#1
    call fuji_begin
    mov r0,#HOST
    movx a,@r0
    call fn_par1
    mov a,#'/'
    call fn_tx
    mov r6,#255
    call fn_zeros
    call fn_go
    mov r0,#FLAGS               ; a new host: no filter (copying, the one
    movx a,@r0                  ; the source was marked under stays)
    jb0 ho_go
    anl a,#~FL_FILT
    movx @r0,a
ho_go:
    GOTO PG_FILES,E_RUN
ho_fail:
    mov a,#SND_ERROR
    call beep
    mov r4,#3
    mov a,#s_nomount&0FFh
    call row_h
    call hs_pause
    jmp hs_run

; hs_edit: the bar's name into the editor; back at hs_edited.
hs_edit:
    mov r0,#SEL
    movx a,@r0
    swap a
    rl a
    mov r2,a
    mov r3,#0
    mov r1,#EDBUF
    mov r7,#32
    call rx_lb
    mov a,r1
    add a,#256-EDBUF
    mov r0,#EDLEN
    movx @r0,a
    mov r0,#EDKIND
    mov a,#ED_HOST
    movx @r0,a
    inc r0                      ; EDMAX
    mov a,#32
    movx @r0,a
    mov r0,#RETPG
    mov a,#PG_HOSTS
    movx @r0,a
    inc r0
    mov a,#E_HEDIT
    movx @r0,a
    GOTO PG_EDIT,E_RUN

; hs_edited: the editor is back. Accepted: the slots read again and written
; back with the bar's name replaced.
hs_edited:
    mov r0,#EDRES
    movx a,@r0
    jz he_x
    call hs_read
    jnz he_fail
    mov r3,#FC_WRHOSTS
    mov r4,#0
    call fuji_begin
    mov r0,#TMP0                ; the slot being streamed
    clr a
    movx @r0,a
he_slot:
    mov r0,#SEL
    movx a,@r0
    mov r6,a
    mov r0,#TMP0
    movx a,@r0
    xrl a,r6
    jz he_new
    movx a,@r0                  ; another slot: its 32 bytes as they were
    swap a
    rl a
    mov r2,a
    mov r3,#0
    mov r7,#32
he_old:
    call st_next
    call fn_tx
    djnz r7,he_old
    jmp he_next
he_new:
    mov r0,#EDLEN
    movx a,@r0
    mov r5,a
    mov r1,#EDBUF
    mov r7,#32
    call xr_tx
he_next:
    mov r0,#TMP0
    movx a,@r0
    inc a
    movx @r0,a
    xrl a,#NROWS
    jnz he_slot
    call fn_go
    jz he_x
he_fail:
    mov a,#SND_ERROR
    call beep
he_x:
    jmp hs_run

; hs_pause: about a second and a half. Clobbers A, r0-r7.
hs_pause:
    mov r0,#TMP2
    mov a,#90
    movx @r0,a
hp_l:
    call in_poll
    mov r0,#TMP2
    movx a,@r0
    dec a
    movx @r0,a
    jnz hp_l
    ret

    include "local.inc"
    if $ > 0C00h
    error "the hosts' page $B has run out"
    endif

; ===========================================================================
    org 0C00h
    STRPAGE h
; sayk_h: line r4 = the string at offset A, white, its bit-7 keys tan.
sayk_h:
    call lbs0_h
    mov a,#C_WHITE
    mov r0,#TXCOL
    movx @r0,a
    inc r0
    mov a,#C_TAN
    movx @r0,a
    jmp say_line_c
s_hosts:    db " FUJINET  HOSTS",0
s_copyto:   db "  COPY TO HOST",0
s_hlegend:  db 'E'|80h," EDIT ",'L'|80h," LOBBY ",'?'|80h,0
s_clegend:  db " PICK  ",'C'|80h,'L'|80h,'R'|80h," STOP",0
s_mount:    db " MOUNTING",0
s_nomount:  db " NO HOST",0
    if $ > 0D00h
    error "the hosts' page $C has run out"
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
