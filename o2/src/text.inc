; text.inc -- the screens' lines: say_line (T0, T1), row_line (rows 0-7).

; say_line: LINEBUF onto line r4, white. say_line_c: in TXCOL / TXCOL2 as
; they are. Clobbers A, r0-r7.
say_line:
    call col_white
say_line_c:
    mov a,r4
    jz sl_t0
    mov r3,#T1&0FFh
    mov r4,#Y_T1
    jmp sl_go
sl_t0:
    mov r3,#T0&0FFh
    mov r4,#Y_T0
sl_go:
    mov r2,#T0>>8
    jmp tx_build

; col_white: TXCOL and TXCOL2 white. Clobbers A, r0.
col_white:
    mov a,#C_WHITE
; col_set: TXCOL and TXCOL2 = A. Clobbers r0.
col_set:
    mov r0,#TXCOL
    movx @r0,a
    inc r0
    movx @r0,a
    ret

; row_str: LINEBUF onto row r4, white, no folder. Clobbers A, r0-r7.
row_str:
    call col_white
    mov r0,#RICON
    clr a
    movx @r0,a
    ; fall into row_line

; row_line: LINEBUF's first NCOLS characters onto row r4, in TXCOL (TXCOL2
; for a character with bit 7 set); sprite 1 (the folder) beside it if RICON
; is set. Clobbers A, r0-r7.
row_line:
    mov a,r4                    ; Y = Y_ROW0 + 24r
    rl a
    rl a
    rl a
    mov r5,a                    ; 8r
    rl a
    add a,r5                    ; 24r
    add a,#Y_ROW0
    mov r0,#LTMP
    movx @r0,a
    rr a                        ; the bias: -(Y/2)
    cpl a
    inc a
    mov r5,a
    mov a,r4                    ; the row's table: ROWS + r * ROW_TAB
    mov r6,a
    mov a,#ROWS&0FFh
    inc r6
    jmp rl_t
rl_m:
    add a,#ROW_TAB
rl_t:
    djnz r6,rl_m
    mov r2,#ROWS>>8
    call poke_at
    mov r6,#LINEBUF
rl_l:
    mov a,r6
    mov r0,a
    movx a,@r0
    mov r7,a
    anl a,#7Fh
    call gptr
    mov a,r3
    call poke_b                 ; pointer, low byte
    call gctl
    call poke_b                 ; colour and pointer bit 8
    inc r6
    mov a,r6
    xrl a,#LINEBUF+NCOLS
    jnz rl_l
    mov r0,#RICON               ; sprite 1's Y: the row's, or none
    movx a,@r0
    jz rl_noi
    mov r0,#LTMP
    movx a,@r0
    jmp poke_b
rl_noi:
    mov a,#SPR_OFF
    jmp poke_b

