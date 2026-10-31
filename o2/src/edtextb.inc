; edtextb.inc -- the library's part of the editor (p5.inc), one page: the
; chart of what the keys type under the shift now, row 7 and T1: each key
; (tan) then what it types (white). With its strings.

    STRPAGE eb

; ed_chart: the chart for SHIFT: row 7 (r5 = 0) or T1 (r5 = 1). Clobbers
; A, r0-r7.
ed_chart:
    mov r0,#SHIFT
    movx a,@r0
    rl a
    add a,r5
    add a,#charts&0FFh
    movp a,@a
    call lbs0_eb
    call ec_cols
    mov a,r5
    jz ech_row
    mov r4,#L_BOT
    jmp say_line_c
ech_row:
    mov r0,#RICON
    clr a
    movx @r0,a
    mov r4,#7
    jmp row_line
ec_cols:
    mov a,#C_WHITE
    mov r0,#TXCOL
    movx @r0,a
    inc r0
    mov a,#C_TAN
    movx @r0,a
    ret

charts:                         ; by SHIFT: row 7, T1
    db s_c0r&0FFh, s_c0t&0FFh
    db s_c1r&0FFh, s_c1t&0FFh
    db s_c2r&0FFh, s_c2t&0FFh
    db s_c1r&0FFh, s_c1t&0FFh
KC      function c,c|80h        ; a key's name: tan
s_c0r:  db KC(CH_TIMES),"SHIFT ",KC(CH_DIV),"SYMB",0
s_c0t:  db KC('E'),KC('N'),KC('T'),"=OK ",KC(CH_TIMES),KC('E'),KC('N'),KC('T'),"=ESC",0
s_c1r:  db KC('1'),"!",KC('2'),"@",KC('3'),"#",KC('4'),"$",KC('5'),"%",KC('6'),"^",0
s_c1t:  db KC('7'),"&",KC('8'),"*",KC('9'),"(",KC('0'),")",KC('?'),22h,KC('+'),";",KC('-'),"_",KC('.'),",",0
s_c2r:  db KC('?'),27h,KC('+'),"]",KC('='),"[",KC('.'),">",KC('-'),"<",KC(CH_TIMES),"|",0
s_c2t:  db KC(CH_DIV),"/",KC('1'),"`",KC('2'),"~",KC('9'),"{",KC('0'),"}",0
