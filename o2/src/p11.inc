; p11.inc -- page 11: the copy. The source (COPYH, COPYPL:COPYPH) read
; again: its host's folder is its prefix still -- unless it's the host the
; browser is in now, whose prefix is now the destination: that copy can't be
; named, so it is refused. Then COPY_FILE "name|/" (into the destination's
; prefix), and "name.cfg|/" after it, if there is one; then the source's
; folder.

    PAGE_HEAD
    db cp_run&0FFh              ; E_RUN

cp_run:
    call scr_text
    mov r4,#L_TOP
    mov a,#s_copy&0FFh
    call say_c
    mov r0,#COPYH
    movx a,@r0
    mov r6,a
    mov r0,#HOST
    movx a,@r0
    xrl a,r6
    jnz cp_go
    mov a,#s_samehost&0FFh
    jmp c_fail
cp_go:
    call cp_source
    jnz cp_fail
    call lb_clear               ; its name, row 2
    mov r3,#0
    mov r2,#4+13
    mov r1,#LINEBUF
    mov r7,#NCOLS
    call rx_lb
    mov r4,#2
    call row_str
    mov r4,#4
    mov a,#s_copying&0FFh
    call row_c
    clr a                       ; the file
    call cp_file
    jnz cp_fail
    mov a,#SND_OK
    call beep
    mov r4,#4
    mov a,#s_copied&0FFh
    call row_c
    call cp_source              ; its .cfg, if it has one
    jnz cp_done
    mov a,#'c'
    call cp_file
    jz cp_cfg
    call cp_source
    jnz cp_done
    mov a,#'C'
    call cp_file
    jnz cp_done
cp_cfg:
    mov r4,#5
    mov a,#s_cfgtoo&0FFh
    call row_c
cp_done:
    mov r0,#COPYH               ; back where it came from
    movx a,@r0
    mov r0,#HOST
    movx @r0,a
    call cp_end
    mov r4,#L_BOT
    mov a,#s_anykey&0FFh
    call say_c
    call c_key
    GOTO PG_FILES,E_RUN
cp_fail:
    call cp_end
    mov a,#s_cpfail&0FFh
    jmp c_fail

; cp_end: no longer copying. Clobbers A, r0.
cp_end:
    mov r0,#FLAGS
    movx a,@r0
    anl a,#~FL_COPY
    movx @r0,a
    ret

; cp_source: the source's entry in the reply (a block from COPYPL:COPYPH:
; its name at 4 + 13): MOUNT_HOST, OPEN_DIRECTORY ("", the filter it was
; marked under), SET_DIRECTORY_POSITION, READ_DIR_ENTRY. A = 0 if it's
; there. Clobbers A, r0-r7.
cp_source:
    mov r3,#FC_MOUNTHOST
    mov r4,#1
    call fuji_begin
    mov r0,#COPYH
    movx a,@r0
    call fn_par1
    call fn_go
    jnz cs_x
    mov r3,#FC_OPENDIR
    mov r4,#1
    call fuji_begin
    mov r0,#COPYH
    movx a,@r0
    call fn_par1
    clr a
    call fn_tx
    mov r5,#0
    mov r0,#FLAGS
    movx a,@r0
    jb1 cs_filt
    jmp cs_pat
cs_filt:
    mov r0,#FLEN
    movx a,@r0
    mov r5,a
cs_pat:
    mov r1,#EDBUF
    mov r7,#255
    call xr_tx
    call fn_go
    jnz cs_x
    mov r3,#FC_DIRPOS
    mov r4,#1
    call fuji_begin
    mov r0,#COPYPL
    movx a,@r0
    mov r6,a
    inc r0
    movx a,@r0
    mov r5,a
    call fn_par2
    call fn_go
    jnz cs_x
    mov r3,#FC_READDIR
    mov r4,#2
    call fuji_begin
    mov a,#1
    call fn_par1
    mov a,#DIRBLOCK
    call fn_par1
    jmp fn_go
cs_x:
    ret

    if $ > 900h
    error "the copy's page $8 has run out"
    endif

; ===========================================================================
    org 900h

; cp_file: COPY_FILE from the source's host to the browser's (1-based),
; "name|/": A = 0 the name as it is; else its .cfg sibling, "cfg" in A's
; case: the name up to its last '.', then ".cfg". A = 0 if it worked.
; Clobbers A, r0-r7.
cp_file:
    mov r0,#TMP2
    movx @r0,a
    mov r3,#FC_COPYFILE
    mov r4,#2
    call fuji_begin
    mov r0,#COPYH
    movx a,@r0
    inc a
    call fn_par1
    mov r0,#HOST
    movx a,@r0
    inc a
    call fn_par1
    mov r0,#TMP1                ; the name's length, and its last '.'
    mov a,#0FFh                 ; (TMP1: none)
    movx @r0,a
    mov r3,#0
    mov r2,#4+13
    mov r5,#0
cf_len:
    call st_next
    jz cf_have
    xrl a,#'.'
    jnz cf_nd
    mov r0,#TMP1
    mov a,r5
    movx @r0,a
cf_nd:
    inc r5
    jmp cf_len
cf_have:
    mov r0,#TMP2                ; the .cfg: up to the '.', if it has one
    movx a,@r0
    jz cf_send
    mov r0,#TMP1
    movx a,@r0
    mov r5,a
    inc a
    jnz cf_send
    mov a,#1                    ; no '.': no .cfg
    ret
cf_send:
    mov r3,#0
    mov r2,#4+13
    mov a,r5
    mov r7,a
    jz cf_ext
cf_name:
    call st_next
    call fn_tx
    djnz r7,cf_name
cf_ext:
    mov r0,#TMP2
    movx a,@r0
    jz cf_bar
    mov r7,a                    ; 'c' or 'C': cfg in that case
    mov a,#'.'
    call fn_tx
    mov a,r7
    call fn_tx
    mov a,r7
    add a,#'f'-'c'
    call fn_tx
    mov a,r7
    add a,#'g'-'c'
    call fn_tx
cf_bar:
    mov a,#'|'
    call fn_tx
    mov a,#'/'
    call fn_tx
    mov r5,#15                  ; a copy can take a while: a minute
    jmp fn_go_long

; c_key: until a key or the button. Clobbers A, r0-r7.
c_key:
    call in_poll
    mov r0,#KEY
    movx a,@r0
    cpl a
    jnz ck_x
    mov r0,#JOYEV
    movx a,@r0
    jb4 ck_x
    jmp c_key
ck_x:
    ret

    include "local.inc"
    if $ > 0A00h
    error "the copy's page $9 has run out"
    endif

; ===========================================================================
    org 0A00h
    STRPAGE c
s_copy:     db "   COPY FILE",0
s_samehost: db " SAME HOST",0
s_copying:  db " COPYING",0
s_copied:   db " COPIED",0
s_cfgtoo:   db " AND ITS .CFG",0
s_cpfail:   db " COPY FAILED",0
s_anykey:   db "   PRESS A KEY",0

; c_fail: the reason (the string at offset A of this page) on row 4, then a
; key: the hosts.
c_fail:
    mov r6,a
    mov a,#SND_ERROR
    call beep
    mov a,r6
    mov r4,#4
    call row_c
    mov r4,#L_BOT
    mov a,#s_anykey&0FFh
    call say_c
    call c_key
    GOTO PG_HOSTS,E_RUN

    if $ > 0B00h
    error "the copy's page $A has run out"
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
