; p5.inc -- page 5: the editor. EDBUF (EDLEN of it, at most EDMAX) on rows
; 1-6, twelve a row; the caller's prompt (EDKIND) in T0; on row 0 the shift
; and the length; a chart of what the keys type now on row 7 and T1 (those
; three are the library's: edtext.inc). Then back to the caller (RETPG,
; RETEN) with EDRES: 1 accepted, 0 cancelled.
;
; The keyboard (netcat's layout, so an O2 has one): plain keys type lower
; case, digits, space and ? + . - =; x is the shift -- upper case, a PC's
; shifted digits, the punctuation the console lacks -- and x x locks it
; (x again unlocks); divide is the second shift, for the rest. CLR rubs out
; (x CLR: the character at the cursor, divide CLR: everything), ENT accepts
; (x ENT: cancels), the stick moves the cursor, the button accepts. ? on an
; empty field is the help (closed with ?, it types one).
;
; The 8244's font has neither lower case nor most punctuation: a character
; it lacks shows as its look-alike in another colour (disp.inc), and the
; one before the cursor shows as it is, zoomed in sprite 2 at the top right
; (the library's magnifier).

EDPAGE  equ 1                   ; (local.inc: the editor's doors too)

    PAGE_HEAD
    db e_edit&0FFh              ; E_RUN
    db e_hret&0FFh              ; E_HRET
e_edit:
    jmp ed_start
e_hret:
    jmp ed_back

; ed_start: the cursor at the end, no shift; the screen; the loop.
ed_start:
    mov r0,#EDLEN
    movx a,@r0
    mov r0,#EDCUR
    movx @r0,a
    mov r0,#SHIFT
    clr a
    movx @r0,a
    call ed_screen
    jmp ed_loop

; ed_back: back from the help; closed with ?, that's typed.
ed_back:
    call ed_screen
    mov r0,#FLAGS
    movx a,@r0
    jb2 eb_q
    jmp ed_loop
eb_q:
    anl a,#~FL_HELPQ
    movx @r0,a
    mov a,#'?'
    call ed_ins
    ; fall into ed_loop

; ed_loop: a frame: the cursor blinks; the stick moves it, the button
; accepts; a key goes to ed_key.
ed_loop:
    call in_poll
    call ed_blink
    mov r0,#JOYEV
    movx a,@r0
    jb2 el_left
    jb3 el_right
    jb0 el_up
    jb1 el_down
    jb4 el_ok
    mov r0,#KEY
    movx a,@r0
    cpl a
    jz el_idle
    cpl a
    call ed_key
    jmp ed_loop
el_idle:
    call ed_lazy                ; a quiet frame: a line that's due
    jmp ed_loop
el_ok:
    jmp ed_accept
el_left:
    mov r6,#0FFh
    jmp el_move
el_right:
    mov r6,#1
    jmp el_move
el_up:
    mov r6,#256-NCOLS
    jmp el_move
el_down:
    mov r6,#NCOLS
el_move:                        ; EDCUR + r6, kept in 0..EDLEN
    mov r0,#EDCUR
    movx a,@r0
    add a,r6
    jb7 ed_loop                 ; under 0
    mov r5,a
    mov r0,#EDLEN
    movx a,@r0
    cpl a
    add a,r5                    ; C: past the end
    jc ed_loop
    mov a,r5
    mov r0,#EDCUR
    movx @r0,a
    mov a,#SND_MOVE
    call beep
    call ed_cursor
    jmp ed_loop

; ed_key: key A. Clobbers A, r0-r7.
ed_key:
    mov r5,a
    mov r0,#SHIFT
    movx a,@r0
    mov r4,a                    ; the shift it was typed under
    mov a,r5
    xrl a,#K_TIMES
    JZL ek_times
    mov a,r5
    xrl a,#K_DIV
    JZL ek_div
    mov a,r5
    xrl a,#K_CLR
    JZL ek_clr
    mov a,r5
    xrl a,#K_ENT
    JZL ek_ent
    mov a,r5                    ; ? plain on an empty field: the help
    xrl a,#K_QUEST
    jnz ek_type
    mov a,r4
    jnz ek_type
    mov r0,#EDLEN
    movx a,@r0
    jnz ek_type
    call spr_hide
    HELP HC_EDIT,PG_EDIT,E_HRET
ek_type:
    mov a,r5                    ; the layer's character for it
    call ek_char
    mov r5,a
    mov a,r4                    ; a one-key shift is spent
    xrl a,#SH_LOCK
    jz ek_put
    mov a,r4
    jz ek_put
    call ed_unshift
ek_put:
    mov a,r5
    jz ek_x
    jmp ed_ins
ek_x:
    ret

    if $ > 900h
    error "the editor's page $8 has run out"
    endif

; ===========================================================================
    org 900h

; ed_ins: character A in at the cursor, the rest along one; the cursor
; after it. Full: a buzz. Clobbers A, r0-r7.
ed_ins:
    mov r5,a
    mov r0,#EDMAX
    movx a,@r0
    mov r6,a
    mov r0,#EDLEN
    movx a,@r0
    xrl a,r6
    jnz ei_room
    mov a,#SND_ERROR
    jmp beep
ei_room:
    movx a,@r0                  ; from the end down to the cursor: one along
    mov r6,a
    inc a
    movx @r0,a
    mov r0,#EDCUR
    movx a,@r0
    mov r7,a
ei_sh:
    mov a,r6
    xrl a,r7
    jz ei_put
    mov a,r6
    add a,#EDBUF-1
    mov r0,a
    movx a,@r0
    inc r0
    movx @r0,a
    dec r6
    jmp ei_sh
ei_put:
    mov a,r7
    add a,#EDBUF
    mov r0,a
    mov a,r5
    movx @r0,a
    mov r0,#EDCUR
    mov a,r7
    inc a
    movx @r0,a
    mov a,#SND_KEY
    call beep
    mov a,r7                    ; the rows from the cursor's
    jmp ed_from

; ed_rub: the character before the cursor out. ed_del: the one at it.
; Clobbers A, r0-r7.
ed_rub:
    mov r0,#EDCUR
    movx a,@r0
    jz er_x
    dec a
    movx @r0,a
    jmp ed_del
er_x:
    jmp ed_refresh
ed_del:
    mov r0,#EDLEN
    movx a,@r0
    mov r6,a
    mov r0,#EDCUR
    movx a,@r0
    mov r7,a
    xrl a,r6
    jz er_x                     ; at the end: nothing there
    mov a,r7
    mov r5,a
ed_dl:
    mov a,r5                    ; down one, from the cursor to the end
    inc a
    xrl a,r6
    jz ed_dend
    mov a,r5
    add a,#EDBUF+1
    mov r0,a
    movx a,@r0
    dec r0
    movx @r0,a
    inc r5
    jmp ed_dl
ed_dend:
    mov r0,#EDLEN
    mov a,r6
    dec a
    movx @r0,a
    mov a,#SND_KEY
    call beep
    mov a,r7
    jmp ed_from

; ed_wipe: all of it. Clobbers A, r0-r7.
ed_wipe:
    clr a
    mov r0,#EDLEN
    movx @r0,a
    mov r0,#EDCUR
    movx @r0,a
    mov a,#SND_BACK
    call beep
    clr a
    ; fall into ed_from

; ed_from: the field's rows from the one holding character A on to EDLEN's
; (a rub-out's vacated character is there), and the status, due: ed_lazy
; draws them a row a frame, the keyboard read between. The cursor and the
; magnifier now. Clobbers A, r0-r7.
ed_from:
    call ed_div12
    mov a,r6
    inc a                       ; ED_ROW0
    mov r7,a                    ; the first row
    mov r0,#EDLEN
    movx a,@r0
    call ed_div12
    inc r6                      ; the last
    mov r5,#1                   ; bit 0: the status
ef_bits:                        ; and bits first..last
    mov a,r7
    mov r4,a
    mov a,#1
ef_sh:
    rl a
    djnz r4,ef_sh
    orl a,r5
    mov r5,a
    mov a,r7
    xrl a,r6
    jz ef_set
    mov a,r7
    xrl a,#ED_ROWS
    jz ef_set
    inc r7
    jmp ef_bits
ef_set:
    mov r0,#EDIRTY
    movx a,@r0
    orl a,r5
    movx @r0,a
    ; fall into ed_refresh

; ed_refresh: the cursor and the magnifier. Clobbers A, r0-r7.
ed_refresh:
    jmp ed_cursor

; ed_div12: r6 = A / 12. Clobbers A.
ed_div12:
    mov r6,#0
ed12_l:
    add a,#256-NCOLS
    jnc ed12_x
    inc r6
    jmp ed12_l
ed12_x:
    ret

; ed_unshift: a one-key shift spent: SHIFT none; the status and the chart
; due (ed_lazy). Clobbers A, r0.
ed_unshift:
    clr a
    mov r0,#SHIFT
    movx @r0,a
    ; fall into ed_chdue

; ed_chdue: the status and the chart's two lines due. Clobbers A, r0.
ed_chdue:
    mov r0,#EDIRTY
    movx a,@r0
    orl a,#81h
    movx @r0,a
    inc r0                      ; EDIRTY2
    mov a,#1
    movx @r0,a
    ret

; ed_row: row r4 of the field. Clobbers A, r0-r7.
ed_row:
    call lb_clear
    mov a,r4                    ; its first character: 12 x (r4 - 1)
    add a,#256-ED_ROW0
    mov r6,a
    rl a
    add a,r6                    ; x3
    rl a
    rl a                        ; x12
    mov r6,a
    mov r0,#EDLEN               ; EDLEN - first: none left (C clear) or
    movx a,@r0                  ; how many
    cpl a
    add a,r6
    cpl a
    jc erw_out
    jz erw_out
    mov r5,a
    add a,#256-NCOLS-1          ; at most 12
    jnc erw_go
    mov r5,#NCOLS
erw_go:
    mov r1,#LINEBUF
erw_l:
    mov a,r6
    add a,#EDBUF
    mov r0,a
    movx a,@r0
    movx @r1,a
    inc r1
    inc r6
    djnz r5,erw_l
erw_out:
    jmp row_str

; ed_shift: SHIFT = A: the status and the chart due, the cursor's colour
; now.
; Clobbers A, r0-r7.
ed_shift:
    mov r0,#SHIFT
    movx @r0,a
    call ed_chdue
    jmp ed_cursor

    if $ > 0A00h
    error "the editor's page $9 has run out"
    endif

; ===========================================================================
    org 0A00h

; ed_screen: the whole screen: the prompt, the field, the status, the chart,
; the cursor and the magnifier. Clobbers A, r0-r7.
ed_screen:
    call scr_text
    call ed_prompt
    mov a,#P1_VDC               ; the cursor: a bar at a character's left
    outl p1,a
    mov r0,#vdc_spr0_shape
    mov r2,#7
es_s:
    mov a,#01h
    movx @r0,a
    inc r0
    djnz r2,es_s
    clr a
    movx @r0,a
    mov a,#P1_MAIN
    outl p1,a
    mov r0,#EMAGC               ; the magnifier: nothing yet
    mov a,#0FFh
    movx @r0,a
    mov r0,#EDIRTY              ; everything due
    movx @r0,a
    inc r0
    mov a,#1
    movx @r0,a
    jmp ed_refresh

; ed_lazy: one line that's due: the status, a field row, the chart's row 7
; or its T1 (the first in that order). Clobbers A, r0-r7.
ed_lazy:
    mov r0,#EDIRTY
    movx a,@r0
    jz el_t1
    mov r6,a
    mov r4,#0                   ; the lowest bit: r4
    mov r5,#1
el_bit:
    mov a,r6
    anl a,r5
    jnz el_got
    inc r4
    mov a,r5
    rl a
    mov r5,a
    jmp el_bit
el_got:
    mov a,r5                    ; it's drawn
    cpl a
    anl a,r6
    movx @r0,a
    mov a,r4
    jz el_stat
    xrl a,#7
    jz el_c7
    jmp ed_row
el_stat:
    jmp ed_status
el_c7:
    mov r5,#0
    jmp ed_chart
el_t1:
    inc r0                      ; EDIRTY2: the chart's T1
    movx a,@r0
    jz el_x
    clr a
    movx @r0,a
    mov r5,#1
    jmp ed_chart
el_x:
    ret

; ed_cursor: sprite 0 at the cursor, coloured by the shift (it blinks:
; ed_blink); the magnifier. Clobbers A, r0-r7.
ed_cursor:
    mov r0,#EBLINK
    clr a
    movx @r0,a
    mov r0,#EDCUR               ; row 1 + EDCUR / 12, column EDCUR mod 12
    movx a,@r0
    mov r6,#Y_ROW0+ED_ROW0*ROW_PITCH
ec_div:
    add a,#256-NCOLS
    jnc ec_have
    mov r5,a
    mov a,r6
    add a,#ROW_PITCH
    mov r6,a
    mov a,r5
    jmp ec_div
ec_have:
    add a,#NCOLS                ; the column
    rl a
    rl a
    rl a
    add a,#X_ROWTXT
    mov r5,a
    mov a,r6
    mov r4,a
    mov r0,#SHIFT
    movx a,@r0
    add a,#curcol&0FFh
    movp a,@a
    mov r2,a
    mov r3,#0
    call spr_at
    ; fall into ed_mag

; ed_mag: the magnifier: the character before the cursor (or at it, at the
; start), when it changes. Clobbers A, r0-r7.
ed_mag:
    mov r5,#0FFh                ; none
    mov r0,#EDLEN
    movx a,@r0
    jz em_have
    mov r0,#EDCUR
    movx a,@r0
    jz em_at
    dec a
em_at:
    add a,#EDBUF
    mov r0,a
    movx a,@r0
    mov r5,a
em_have:
    mov r0,#EMAGC
    movx a,@r0
    xrl a,r5
    jz em_x                     ; showing it already
    mov a,r5
    movx @r0,a
    jmp mag
em_x:
    ret

curcol:                         ; the cursor, by SHIFT: colour << 3
    db C_WHITE<<3, C_TAN<<3, C_GREEN<<3, C_RED<<3

; ed_blink: the cursor on 16 frames, off 16. Clobbers A, r0-r7.
ed_blink:
    mov r0,#EBLINK
    movx a,@r0
    inc a
    movx @r0,a
    anl a,#0Fh
    jz eb_flip
    ret
eb_flip:
    movx a,@r0
    jb4 eb_off
    jmp ed_cursor
eb_off:
    mov r3,#0
    mov r4,#SPR_OFF
    mov r5,#0
    mov r2,#0
    jmp spr_at

    if $ > 0B00h
    error "the editor's page $A has run out"
    endif

; ===========================================================================
    org 0B00h
    include "local.inc"
    include "beep.inc"
    if $ > 0C00h
    error "the editor's page $B has run out"
    endif

; ===========================================================================
    org 0C00h
; ek_char: A = what key A types under SHIFT r4 (0: nothing). Clobbers A,
; r0.
ek_char:
    mov r0,a
    mov a,r4
    xrl a,#SH_SYM
    jz ekc_sym
    mov a,r4
    jz ekc_norm
    mov a,r0
    add a,#kshift&0FFh
    movp a,@a
    ret
ekc_sym:
    mov a,r0
    add a,#ksym&0FFh
    movp a,@a
    ret
ekc_norm:
    mov a,r0
    add a,#knorm&0FFh
    movp a,@a
    ret

; The layers, by key code (input.inc's K_*: row x 8 + column). x, divide,
; CLR and ENT never get here.
knorm:                          ; plain: lower case
    db "0123456789", 0, 0, " ?lp"
    db "+wertuio"
    db "qsdfghjk"
    db "azxcvbm."
    db "-", 0, 0, "=yn", 0, 0
kshift:                         ; x: upper case, a PC's shifted digits; ? "
    db ")!@#$%^&*(", 0, 0, " ", 22h, "LP"   ; + ; . , - _ = :
    db ";WERTUIO"
    db "QSDFGHJK"
    db "AZXCVBM,"
    db "_", 0, 0, ":YN", 0, 0
ksym:                           ; divide: 0 } 1 ` 2 ~ 9 { ? ' + ] . > - <
    db "}`~", 0, 0, 0, 0, 0, 0, "{", 0, 0, " ", 27h, 0, 0   ; = [
    db "]", 0, 0, 0, 0, 0, 0, 0
    db 0, 0, 0, 0, 0, 0, 0, 0
    db 0, 0, 0, 0, 0, 0, 0, ">"
    db "<", 0, 0, "[", 0, 0, 0, 0

    if $ > 0D00h
    error "the editor's page $C has run out"
    endif

; ===========================================================================
    org 0D00h
ek_times:                       ; x: shift; again: lock; locked: off.
    mov a,r4                    ; after divide: |
    xrl a,#SH_SYM
    jz ekt_bar
    mov a,r4
    mov r1,#SH_ONE
    jz ekt_set
    mov r1,#SH_LOCK
    xrl a,#SH_ONE
    jz ekt_set
    mov r1,#SH_NONE
ekt_set:
    mov a,r1
    mov r5,a
    mov a,#SND_MOVE
    call beep
    mov a,r5
    jmp ed_shift
ekt_bar:
    call ed_unshift
    mov a,#'|'
    jmp ed_ins

ek_div:                         ; divide: the symbols; again: /; after x:
                                ; backslash
    mov a,r4
    jnz ekd_typ
    mov a,#SND_MOVE
    call beep
    mov a,#SH_SYM
    jmp ed_shift
ekd_typ:
    mov r5,#'/'
    xrl a,#SH_SYM
    jz ekd_put
    mov r5,#5Ch                 ; backslash
ekd_put:
    mov a,r4
    xrl a,#SH_LOCK
    jz ekd_ins
    call ed_unshift
ekd_ins:
    mov a,r5
    jmp ed_ins

ek_clr:                         ; CLR: rub out; x CLR: the one at the cursor;
    mov a,r4                    ; divide CLR: all of it
    jz ek_rub
    xrl a,#SH_LOCK
    jz ek_rub
    call ed_unshift
    mov a,r4
    xrl a,#SH_ONE
    jz ek_del
    jmp ed_wipe
ek_rub:
    jmp ed_rub
ek_del:
    jmp ed_del

ek_ent:                         ; ENT: accept; x ENT: cancel
    mov a,r4
    xrl a,#SH_ONE
    jz ed_cancel
    jmp ed_accept

; ed_accept, ed_cancel: back to the caller with EDRES.
ed_cancel:
    mov a,#SND_BACK
    call beep
    clr a
    jmp ed_exit
ed_accept:
    mov a,#SND_SELECT
    call beep
    mov a,#1
ed_exit:
    mov r0,#EDRES
    movx @r0,a
    call spr_hide
    mov r0,#RETEN
    movx a,@r0
    mov r7,a
    mov r0,#RETPG
    movx a,@r0
    jmp far_go

    if $ > 0E00h
    error "the editor's page $D has run out"
    endif

; ===========================================================================
    org 0E00h
    include "input.inc"
    include "snd.inc"

    PAGE_TAIL
