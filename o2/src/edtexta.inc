; edtexta.inc -- the library's part of the editor (p5.inc), one page: T0's
; prompt and row 0's status line. With its strings: MOVP reads its own page.

    STRPAGE ea

; ed_prompt: T0 = the prompt for EDKIND, tan. Clobbers A, r0-r7.
ed_prompt:
    mov r0,#EDKIND
    movx a,@r0
    add a,#prompts&0FFh
    movp a,@a
    call lbs0_ea
    mov a,#C_TAN
    call col_set
    mov r4,#L_TOP
    jmp say_line_c

; ed_status: row 0: the shift's word in its colour, the length out of the
; most (nn/mm) in grey. Clobbers A, r0-r7.
ed_status:
    mov r0,#SHIFT
    movx a,@r0
    add a,#shwords&0FFh
    movp a,@a
    call lbs0_ea
    mov r0,#EDLEN
    movx a,@r0
    mov r1,#LINEBUF+7
    call es_num
    mov a,#'/'
    movx @r1,a
    inc r1
    mov r0,#EDMAX
    movx a,@r0
    call es_num
    mov a,#C_DGREY
    call col_set
    mov r0,#SHIFT
    movx a,@r0
    add a,#shcols&0FFh
    movp a,@a
    mov r0,#TXCOL2
    movx @r0,a
    mov r0,#RICON
    clr a
    movx @r0,a
    mov r4,#0
    jmp row_line

; es_num: A (0-99) as two digits into LINEBUF at r1, on. Clobbers A, r6.
es_num:
    mov r6,#'0'-1
en_10:
    inc r6
    add a,#256-10
    jc en_10
    add a,#10+'0'
    xch a,r6
    movx @r1,a
    inc r1
    mov a,r6
    movx @r1,a
    inc r1
    ret

prompts:                        ; by EDKIND
    db s_pssid&0FFh, s_ppass&0FFh, s_phost&0FFh, s_pfilt&0FFh
shwords:                        ; by SHIFT
    db s_none&0FFh, s_shift&0FFh, s_sym&0FFh, s_lock&0FFh
shcols:
    db C_DGREY, C_TAN, C_GREEN, C_RED
s_pssid:    db "WIFI NAME",0
s_ppass:    db "PASSWORD",0
s_phost:    db "HOST NAME",0
s_pfilt:    db "FILTER",0
s_none:     db 0
s_shift:    db 'S'|80h,'H'|80h,'I'|80h,'F'|80h,'T'|80h,0
s_sym:      db 'S'|80h,'Y'|80h,'M'|80h,'B'|80h,'O'|80h,'L'|80h,0
s_lock:     db 'L'|80h,'O'|80h,'C'|80h,'K'|80h,0
