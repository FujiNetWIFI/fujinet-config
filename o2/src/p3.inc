; p3.inc -- page 3: the browser's page of a folder. The folder is open (the
; folders' page, p9.inc, opened it); SET_DIRECTORY_POSITION to the page's
; first entry, then one READ_DIR_ENTRY in block mode brings a reply's worth
; of entries -- whole names, a folder bit each -- and that reply stays put
; while the page is up: the rows, the bar's scrolling name, the boot's path
; and the folders' "in" all read it there. A .cfg (a game's settings, which
; travels with it) is never listed.
;
; The bar moves with the stick (off the bottom or the top: the next page or
; the one before); left and right page; the button, ENT or 1-8 opens a
; folder or boots a file; CLR goes up a folder (from the top: the hosts); F
; filters; C marks a file to copy (copying: C copies here); H or ? helps.

E_FRUN  equ 0                   ; (E_RUN: a folder from its top)

    PAGE_HEAD
    db e_frun&0FFh              ; E_RUN
    db e_fpage&0FFh             ; E_PAGE
    db e_fedit&0FFh             ; E_FEDIT
    db e_fshow&0FFh             ; E_SHOW
e_frun:
    GOTO PG_DIR,E_NEW
e_fpage:
    GOTO PG_DIR,E_REOPEN
e_fedit:
    GOTO PG_DIR,E_FEDONE
e_fshow:
    jmp fl_show

; fl_loop: a frame's input, and the bar's long name a step.
fl_loop:
    call in_poll
    call fl_marq
    mov r0,#JOYEV
    movx a,@r0
    jb0 fl_up
    jb1 fl_down
    jb2 fl_left_j
    jb3 fl_right_j
    jb4 fl_open_j
    mov r0,#KEY
    movx a,@r0
    mov r5,a
    xrl a,#K_ENT
    jz fl_open_j
    mov a,r5
    xrl a,#K_CLR
    jz fl_parent
    mov a,r5
    xrl a,#K_F
    jz fl_filter
    mov a,r5
    xrl a,#K_C
    jz fl_copy
    mov a,r5
    xrl a,#K_H
    jz fl_help
    mov a,r5
    xrl a,#K_QUEST
    jz fl_help
    mov a,r5                    ; 1-8: that row, if it has one: opened
    jz fl_loop
    add a,#256-9
    jc fl_loop
    mov a,r5
    dec a
    mov r6,a
    mov r0,#NVIS
    movx a,@r0
    mov r5,a
    mov a,r6
    cpl a
    add a,r5                    ; C: a row there is
    jnc fl_loop
    mov a,r6
    call fl_sel
    jmp fl_open
fl_left_j:
    jmp fl_left
fl_right_j:
    jmp fl_right
fl_open_j:
    jmp fl_open

fl_parent:
    mov a,#SND_BACK
    call beep
    GOTO PG_DIR,E_PARENT
fl_filter:
    mov r0,#FLAGS               ; (copying: the filter is the source's)
    movx a,@r0
    jb0 fl_loop
    GOTO PG_DIR,E_FILTER
fl_copy:
    mov r0,#FLAGS
    movx a,@r0
    jb0 fl_paste
    call fl_isdir               ; a folder doesn't copy
    jnz fl_nope
    GOTO PG_DIR,E_MARK
fl_paste:
    GOTO PG_COPY,E_RUN
fl_nope:
    mov a,#SND_ERROR
    call beep
    jmp fl_loop
fl_help:
    HELP HC_FILES,PG_FILES,E_PAGE

fl_up:
    mov r0,#SEL
    movx a,@r0
    jz fl_pgup
    dec a
    call fl_sel
    jmp fl_loop
fl_pgup:                        ; up off the top: the page before, its last
    mov r0,#POSL
    movx a,@r0
    mov r6,a
    inc r0
    movx a,@r0
    orl a,r6
    jz fl_loop
    mov r0,#SEL
    mov a,#NROWS-1
    movx @r0,a
    jmp fl_prev
fl_down:
    mov r0,#NVIS
    movx a,@r0
    jz fl_loop
    dec a
    mov r6,a
    mov r0,#SEL
    movx a,@r0
    xrl a,r6
    jz fl_pgdn
    movx a,@r0
    inc a
    call fl_sel
    jmp fl_loop
fl_pgdn:                        ; down off the bottom: the next page's first
    mov r0,#NXTH
    movx a,@r0
    jb7 fl_loop
    mov r0,#SEL
    clr a
    movx @r0,a
    jmp fl_next

; fl_isdir: A = the bar's row's folder bit (nonzero: a folder). Clobbers A,
; r0.
fl_isdir:
    mov r0,#SEL
    movx a,@r0
    rl a
    add a,#ROWT+1
    mov r0,a
    movx a,@r0
    anl a,#80h
    ret

; fl_name: r3:r2 = the reply offset of row A's name. Clobbers A, r0.
fl_name:
    rl a
    add a,#ROWT
    mov r0,a
    movx a,@r0
    mov r2,a
    inc r0
    movx a,@r0
    anl a,#3
    mov r3,a
    ret

    if $ > 900h
    error "the browser's page $8 has run out"
    endif

; ===========================================================================
    org 900h

fl_left:
    mov r0,#POSL
    movx a,@r0
    mov r6,a
    inc r0
    movx a,@r0
    orl a,r6
    JZL fl_loop
    jmp fl_prev
fl_right:
    mov r0,#NXTH
    movx a,@r0
    JB7L fl_loop
    mov r0,#SEL
    clr a
    movx @r0,a
    jmp fl_next

; fl_next: the next page: this one's first onto RING (the oldest falls off
; a full one), POS = NXT.
fl_next:
    mov r0,#PGN
    movx a,@r0
    xrl a,#RINGN
    jnz fn_push
    mov r0,#RING+2              ; full: every one down a place
    mov r1,#RING
    mov r2,#2*RINGN-2
fn_shift:
    movx a,@r0
    movx @r1,a
    inc r0
    inc r1
    djnz r2,fn_shift
    mov r0,#PGN
    mov a,#RINGN-1
    movx @r0,a
fn_push:
    mov r0,#PGN
    movx a,@r0
    rl a
    add a,#RING
    mov r1,a
    mov r0,#POSL
    movx a,@r0
    movx @r1,a
    inc r1
    inc r0
    movx a,@r0
    movx @r1,a
    mov r0,#PGN
    movx a,@r0
    inc a
    movx @r0,a
    mov r0,#NXTL                ; POS = NXT
    movx a,@r0
    mov r0,#POSL
    movx @r0,a
    mov r0,#NXTH
    movx a,@r0
    mov r0,#POSH
    movx @r0,a
    mov a,#SND_MOVE
    call beep
    jmp fl_show

; fl_prev: the page before: off RING, or (past what RING keeps) 8 back.
fl_prev:
    mov r0,#PGN
    movx a,@r0
    jz fp_back8
    dec a
    movx @r0,a
    rl a
    add a,#RING
    mov r1,a
    movx a,@r1
    mov r0,#POSL
    movx @r0,a
    inc r1
    movx a,@r1
    mov r0,#POSH
    movx @r0,a
    jmp fp_go
fp_back8:
    mov r0,#POSL
    movx a,@r0
    add a,#256-NROWS
    movx @r0,a
    inc r0
    movx a,@r0
    jc fp_hi
    dec a                       ; the borrow
fp_hi:
    movx @r0,a
    jb7 fp_zero                 ; under 0: the top
    jmp fp_go
fp_zero:
    clr a
    movx @r0,a
    dec r0
    movx @r0,a
fp_go:
    mov a,#SND_MOVE
    call beep
    jmp fl_show

; fl_open: the bar's row. A folder: in. A file: copying, nothing; else boot
; it (its name stays in the reply for the boot to stream).
fl_open:
    mov r0,#NVIS
    movx a,@r0
    JZL fl_loop
    mov a,#SND_SELECT
    call beep
    call fl_isdir
    jz fo_file
    GOTO PG_DIR,E_ENTER
fo_file:
    mov r0,#FLAGS
    movx a,@r0
    JB0L fl_loop
    mov r0,#SEL
    movx a,@r0
    call fl_name
    mov r0,#BNAML
    mov a,r2
    movx @r0,a
    inc r0
    mov a,r3
    movx @r0,a
    GOTO PG_MOUNT,E_RUN

; fl_marq: the bar's name, if it's longer than its row: after MQ_WAIT
; frames still, a character every MQ_STEP, to its end and back.
; Clobbers A, r0-r7.
fl_marq:
    mov r0,#NVIS
    movx a,@r0
    jz fm_x
    mov r0,#MQCLK
    movx a,@r0
    dec a
    movx @r0,a
    jz fm_step
fm_x:
    ret
fm_step:
    mov a,#MQ_STEP
    movx @r0,a
    mov r0,#MQLEN               ; MQLEN - 12: the last offset (C: room)
    movx a,@r0
    add a,#256-NCOLS-1
    jnc fm_x
    inc a
    mov r6,a
    mov r0,#MQDIR
    movx a,@r0
    mov r7,a
    mov r0,#MQOFF
    movx a,@r0
    add a,r7
    movx @r0,a
    jz fm_end
    xrl a,r6
    jz fm_end
    jmp fm_draw
fm_end:
    mov r0,#MQDIR               ; turn round, and rest there
    movx a,@r0
    cpl a
    inc a
    movx @r0,a
    mov r0,#MQCLK
    mov a,#MQ_WAIT
    movx @r0,a
fm_draw:
    mov r0,#SEL
    movx a,@r0
    mov r4,a
    jmp fl_row

    if $ > 0A00h
    error "the browser's page $9 has run out"
    endif

; ===========================================================================
    org 0A00h

; fl_show: the page at POS: SET_DIRECTORY_POSITION, the block, its rows,
; the bar, the scrollbar; then the loop.
fl_show:
    mov r3,#FC_DIRPOS
    mov r4,#1
    call fuji_begin
    mov r0,#POSL
    movx a,@r0
    mov r6,a
    inc r0
    movx a,@r0
    mov r5,a
    call fn_par2
    call fn_go
    jnz fs_empty                ; past the end (an emptied folder): nothing
    mov r3,#FC_READDIR
    mov r4,#2
    call fuji_begin
    mov a,#DIRPAGES
    call fn_par1
    mov a,#DIRBLOCK
    call fn_par1
    call fn_go
    jnz fs_empty
    call fl_parse
    jmp fs_rows
fs_empty:
    mov r0,#NVIS
    clr a
    movx @r0,a
    mov r0,#NXTH
    mov a,#80h
    movx @r0,a
fs_rows:
    mov r0,#NVIS                ; the bar on a row there is
    movx a,@r0
    jz fs_bar0
    dec a
    mov r6,a
    mov r0,#SEL
    movx a,@r0
    cpl a
    add a,r6                    ; C: SEL before the last
    jc fs_draw
    mov a,r6
    movx @r0,a
    jmp fs_draw
fs_bar0:
    mov r0,#SEL
    movx @r0,a
fs_draw:
    mov r0,#MQOFF
    clr a
    movx @r0,a
    mov r4,#0
fs_row:
    mov r0,#TMP2
    mov a,r4
    movx @r0,a
    call fl_row
    mov r0,#TMP2
    movx a,@r0
    inc a
    mov r4,a
    xrl a,#NROWS
    jnz fs_row
    mov r0,#NVIS
    movx a,@r0
    jnz fs_some
    mov r4,#3
    mov a,#s_empty&0FFh
    call row_f
    mov a,#0FFh
    call bar_row
    jmp fs_thumb
fs_some:
    mov r0,#SEL
    movx a,@r0
    call fl_sel0
fs_thumb:
    call fl_thumb
    jmp fl_loop

; fl_sel: the bar to row A: the row it leaves redrawn from its start.
; fl_sel0: the bar to row A (the rows are drawn). Clobbers A, r0-r7.
fl_sel:
    mov r0,#TMP2
    movx @r0,a
    mov a,#SND_MOVE
    call beep
    mov r0,#MQOFF
    clr a
    movx @r0,a
    mov r0,#SEL
    movx a,@r0
    mov r4,a
    call fl_row
    mov r0,#TMP2
    movx a,@r0
fl_sel0:
    mov r0,#SEL
    movx @r0,a
    mov r0,#MQCLK
    mov a,#MQ_WAIT
    movx @r0,a
    inc r0                      ; MQDIR
    mov a,#1
    movx @r0,a
    mov r0,#SEL                 ; its name's length, for the scroll
    movx a,@r0
    call fl_name
    mov r5,#0
fsl_len:
    call st_byte
    jz fsl_have
    inc r2
    mov a,r2
    jnz fsl_nc
    inc r3
fsl_nc:
    inc r5
    mov a,r5
    xrl a,#128
    jnz fsl_len
fsl_have:
    mov r0,#MQLEN
    mov a,r5
    movx @r0,a
    mov r0,#SEL
    movx a,@r0
    jmp bar_row

; fl_row: row r4 = row r4's entry from MQOFF if it's the bar's, else from
; its start, a folder beside a folder; blank past the last. Clobbers A,
; r0-r7.
fl_row:
    mov r0,#TMP1
    mov a,r4
    movx @r0,a
    call lb_clear
    mov r0,#RICON
    clr a
    movx @r0,a
    mov r0,#NVIS                ; past the last: blank
    movx a,@r0
    mov r5,a
    mov r0,#TMP1
    movx a,@r0
    mov r6,a
    cpl a
    add a,r5                    ; C: a row there is
    jnc fr_out
    mov a,r6
    rl a
    add a,#ROWT+1
    mov r0,a
    movx a,@r0
    anl a,#80h
    mov r0,#RICON
    movx @r0,a
    mov a,r6
    call fl_name
    mov r0,#SEL
    movx a,@r0
    xrl a,r6
    jnz fr_name
    mov r0,#MQOFF
    movx a,@r0
    add a,r2
    mov r2,a
    clr a
    addc a,r3
    mov r3,a
fr_name:
    mov r1,#LINEBUF
    mov r7,#NCOLS
    call rx_lb
fr_out:
    call col_white
    mov r0,#TMP1
    movx a,@r0
    mov r4,a
    jmp row_line

    if $ > 0B00h
    error "the browser's page $A has run out"
    endif

; ===========================================================================
    org 0B00h

; fl_parse: the block in the reply onto the page: ROWT, NVIS, NXT. A group
; is (flags: bit 7 the last; entries; size, 2; index), then its one entry:
; 4 bytes of date and flags (byte 1 bit 7: a folder), 3 of size, 1 media
; type, the name and its NUL. A .cfg isn't a row. Clobbers A, r0-r7.
fl_parse:
    clr a
    mov r0,#NVIS
    movx @r0,a
    mov r0,#TMP1                ; entries read (hidden ones too)
    movx @r0,a
    mov r0,#NXTH
    mov a,#80h
    movx @r0,a
    mov r3,#0                   ; the groups
    mov r2,#3
    call st_byte
    mov r0,#TMP0
    movx @r0,a
    mov r0,#GOFL
    mov a,#4
    movx @r0,a
    inc r0
    clr a
    movx @r0,a
pa_grp:
    mov r0,#TMP0
    movx a,@r0
    jnz pa_more
    jmp pa_cont                 ; the reply's end, the folder's not
pa_more:
    dec a
    movx @r0,a
    call pa_at                  ; r3:r2 = the group
    call st_next                ; flags
    mov r0,#TMP2
    movx @r0,a
    call st_next                ; entries
    jnz pa_ent
    jmp pa_eof
pa_ent:
    call st_next                ; size
    mov r0,#GSZL
    movx @r0,a
    call st_next
    mov r0,#GSZH
    movx @r0,a
    call st_next                ; (the index)
    call st_next                ; (entry byte 0)
    call st_byte                ; entry byte 1: the flags
    anl a,#80h
    mov r5,a                    ; the folder bit
    jnz pa_show                 ; a folder is listed whatever its name
    call pa_cfg
    jz pa_hide
pa_show:
    mov r0,#NVIS                ; ROWT[NVIS] = the name (group + 13), the
    movx a,@r0                  ; folder bit, how far into the reply
    rl a
    add a,#ROWT
    mov r1,a
    mov r0,#GOFL
    movx a,@r0
    add a,#13
    movx @r1,a
    inc r0
    movx a,@r0
    addc a,#0
    orl a,r5
    mov r5,a
    mov r0,#TMP1                ; the entry's place, bits 2-6
    movx a,@r0
    rl a
    rl a
    anl a,#7Ch
    orl a,r5
    inc r1
    movx @r1,a
    mov r0,#NVIS
    movx a,@r0
    inc a
    movx @r0,a
pa_hide:
    mov r0,#TMP1
    movx a,@r0
    inc a
    movx @r0,a
    mov r0,#TMP2                ; the last group: the folder's end
    movx a,@r0
    jb7 pa_eof
    mov r0,#GSZL                ; on to the next group: + 5 + size
    movx a,@r0
    add a,#5
    mov r6,a
    inc r0
    movx a,@r0
    addc a,#0
    mov r5,a
    mov r0,#GOFL
    movx a,@r0
    add a,r6
    movx @r0,a
    inc r0
    movx a,@r0
    addc a,r5
    movx @r0,a
    mov r0,#NVIS
    movx a,@r0
    xrl a,#NROWS
    jz pa_full
    jmp pa_grp
pa_full:                        ; a page full: more, unless the next group
    mov r0,#TMP0                ; is the empty last one
    movx a,@r0
    jz pa_cont
    call pa_at
    call st_next
    call st_byte
    jz pa_eof
    jmp pa_cont
pa_cont:
    mov r0,#TMP1                ; NXT = POS + entries read
    movx a,@r0
    mov r6,a
    mov r0,#POSL
    movx a,@r0
    add a,r6
    mov r0,#NXTL
    movx @r0,a
    mov r0,#POSH
    movx a,@r0
    addc a,#0
    mov r0,#NXTH
    movx @r0,a
pa_eof:
    ret

; pa_at: r3:r2 = the group's offset. Clobbers A, r0.
pa_at:
    mov r0,#GOFL
    movx a,@r0
    mov r2,a
    inc r0
    movx a,@r0
    mov r3,a
    ret

; pa_cfg: A = 0 if the group's name ends ".cfg" (any case): its last four
; are at group + size .. + 3. Clobbers A, r0, r2, r3, r6.
pa_cfg:
    mov r0,#GSZH                ; a name of 4 or more: size 13 or more
    movx a,@r0
    jnz pc_long
    mov r0,#GSZL
    movx a,@r0
    add a,#256-13
    jnc pc_no
pc_long:
    mov r0,#GOFL
    movx a,@r0
    mov r6,a
    mov r0,#GSZL
    movx a,@r0
    add a,r6
    mov r2,a
    mov r0,#GOFH
    movx a,@r0
    mov r6,a
    mov r0,#GSZH
    movx a,@r0
    addc a,r6
    mov r3,a
    call st_next
    xrl a,#'.'
    jnz pc_no
    call st_next
    orl a,#20h
    xrl a,#'c'
    jnz pc_no
    call st_next
    orl a,#20h
    xrl a,#'f'
    jnz pc_no
    call st_next
    orl a,#20h
    xrl a,#'g'
    ret
pc_no:
    mov a,#1
    ret

    if $ > 0C00h
    error "the browser's page $B has run out"
    endif

; ===========================================================================
    org 0C00h
    include "local.inc"
    STRPAGE f
s_empty:    db "  NO FILES",0

; fl_thumb: the scrollbar: grid column 9's cell at row POS x 8 / count,
; none while the folder is uncounted or fits one page. Clobbers A, r0-r7.
fl_thumb:
    mov r5,#0
    mov r0,#CNTH
    movx a,@r0
    jb7 ft_put
    mov r4,a                    ; r4:r3 = the count, r2:r1 = POS x 8
    mov r0,#CNTL
    movx a,@r0
    mov r3,a
    mov a,r4
    jnz ft_big
    mov a,r3
    add a,#256-NROWS-1
    jnc ft_put                  ; one page
ft_big:
    mov r0,#POSL
    movx a,@r0
    mov r1,a
    mov r0,#POSH
    movx a,@r0
    mov r2,a
    mov r6,#3
ft_x8:
    mov a,r1
    add a,r1
    mov r1,a
    mov a,r2
    addc a,r2
    mov r2,a
    djnz r6,ft_x8
    mov r6,#0                   ; the quotient: count subtracted while it goes
ft_div:
    mov a,r1                    ; r2:r1 - r4:r3 (the borrow: C clear)
    cpl a
    add a,r3
    cpl a
    mov r7,a
    mov a,r2
    cpl a
    addc a,r4
    cpl a
    jc ft_done                  ; (C: r4:r3 > r2:r1)
    mov r2,a
    mov a,r7
    mov r1,a
    inc r6
    mov a,r6
    xrl a,#NROWS-1
    jnz ft_div
ft_done:
    mov r5,#80h                 ; 1 << quotient
    inc r6
ft_sh:
    mov a,r5
    rl a
    mov r5,a
    djnz r6,ft_sh
ft_put:
    mov r3,#9
    jmp grid_col

    if $ > 0D00h
    error "the browser's page $C has run out"
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
