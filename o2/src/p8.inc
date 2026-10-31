; p8.inc -- page 8: the help. A screen's keys (HELPCTX), a page of eight
; rows, keys in tan; the editor's has a second page, the shifts' chart. A
; key or the button turns the page, after the last one goes back (HRET:
; entry << 4 | page); the editor's closed with ? types a ? there (FL_HELPQ).
;
; Each context's rows sit in one 256-byte page with their table and a copy
; of the drawer (HBANK): MOVP reads its own page only.

; HBANK: the drawer for this page's help, suffix SFX: hdraw_SFX draws the
; help page whose table (title, then 8 rows: string offsets, 0 blank) is at
; offset A of this page.
HBANK   macro SFX,{GLOBALSYMBOLS}
    STRPAGE SFX
hdraw_SFX:
    mov r0,#TMP0
    movx @r0,a
    movp a,@a
    mov r4,#L_TOP
    call sayk_SFX
    mov r4,#0
SFX_hrow:
    mov r0,#TMP1
    mov a,r4
    movx @r0,a
    mov r0,#TMP0
    movx a,@r0
    inc a
    movx @r0,a
    movp a,@a
    call rowk_SFX
    mov r0,#TMP1
    movx a,@r0
    inc a
    mov r4,a
    xrl a,#NROWS
    jnz SFX_hrow
    ret
; sayk_SFX, rowk_SFX: the string at offset A (0: blank) on line / row r4,
; white, its keys (bit 7) tan.
sayk_SFX:
    call SFX_hlb
    jmp say_line_c
rowk_SFX:
    call SFX_hlb
    mov r0,#RICON
    clr a
    movx @r0,a
    jmp row_line
SFX_hlb:
    mov r6,a
    call lb_clear
    mov a,r6
    jz SFX_hcol
    call lbs_SFX
SFX_hcol:
    mov a,#C_WHITE
    mov r0,#TXCOL
    movx @r0,a
    inc r0
    mov a,#C_TAN
    movx @r0,a
    ret
    endm

; KY: a key's name, tan (bit 7).
KY      function c,c|80h

    PAGE_HEAD
    db e_help&0FFh              ; E_RUN
e_help:
    jmp hp_run

; hp_run: the context's first page.
hp_run:
    mov r0,#HPAGE
    clr a
    movx @r0,a
hp_draw:
    call scr_text
    mov r0,#HPAGE               ; the page: context x 2 + its page
    movx a,@r0
    mov r6,a
    mov r0,#HELPCTX
    movx a,@r0
    rl a
    add a,r6
    add a,#hp_tab&0FFh
    jmpp @a
hp_tab:                         ; by context x 2 + page
    db hp_wifi&0FFh, hp_last&0FFh
    db hp_hosts&0FFh, hp_last&0FFh
    db hp_files&0FFh, hp_last&0FFh
    db hp_edit1&0FFh, hp_edit2&0FFh
    db hp_last&0FFh, hp_last&0FFh   ; (HC_BOOT: none)
    db hp_info&0FFh, hp_last&0FFh
hp_wifi:
    mov a,#ht_wifi&0FFh
    call hdraw_a
    jmp hp_wait
hp_hosts:
    mov a,#ht_hosts&0FFh
    call hdraw_b
    jmp hp_wait
hp_files:
    mov a,#ht_files&0FFh
    call hdraw_c
    jmp hp_wait
hp_info:
    mov a,#ht_info&0FFh
    call hdraw_a
    jmp hp_wait
hp_edit1:
    mov a,#ht_edit1&0FFh
    call hdraw_e
    jmp hp_wait
hp_edit2:
    mov a,#ht_edit2&0FFh
    call hdraw_d
    jmp hp_wait
hp_last:
    jmp hp_back

; hp_wait: a key or the button: the next page (the last: back). ? in the
; editor's: back, and a ? typed there.
hp_wait:
    call in_poll
    mov r0,#JOYEV
    movx a,@r0
    jb4 hp_next
    mov r0,#KEY
    movx a,@r0
    mov r5,a
    cpl a
    jz hp_wait
    mov a,r5
    xrl a,#K_QUEST
    jnz hp_next
    mov r0,#HELPCTX
    movx a,@r0
    xrl a,#HC_EDIT
    jnz hp_back
    mov r0,#FLAGS
    movx a,@r0
    orl a,#FL_HELPQ
    movx @r0,a
    jmp hp_back
hp_next:
    mov a,#SND_MOVE
    call beep
    mov r0,#HPAGE
    movx a,@r0
    jnz hp_back
    inc a
    movx @r0,a
    jmp hp_draw
hp_back:
    call spr_hide
    mov r0,#HRET
    movx a,@r0
    swap a
    anl a,#0Fh
    mov r7,a
    movx a,@r0
    anl a,#0Fh
    jmp far_go

    include "local.inc"
    include "beep.inc"
    if $ > 900h
    error "the help's page $8 has run out"
    endif

; ===========================================================================
    org 0900h
    HBANK a
ht_wifi:
    db hw_t&0FFh, hw_1&0FFh, hw_2&0FFh, hw_3&0FFh, hw_4&0FFh
    db hw_5&0FFh, 0, hw_7&0FFh, 0
hw_t:   db " HELP: WIFI",0
hw_1:   db KY('S'),KY('T'),KY('I'),KY('C'),KY('K'),"  MOVE",0
hw_2:   db KY('F'),KY('I'),KY('R'),KY('E'),"   JOIN",0
hw_3:   db KY('1'),"-",KY('8'),"    JOIN",0
hw_4:   db KY('R'),"      SCAN",0
hw_5:   db KY('C'),KY('L'),KY('R'),"    SKIP",0
hw_7:   db "OTHER: TYPE",0
ht_info:
    db hi_t&0FFh, hi_1&0FFh, hi_2&0FFh, hi_3&0FFh, 0, 0, 0, 0, 0
hi_t:   db " HELP: INFO",0
hi_1:   db KY('S'),KY('T'),KY('I'),KY('C'),KY('K'),"  PAGE",0
hi_2:   db KY('1'),"-",KY('3'),"    PAGE",0
hi_3:   db KY('C'),KY('L'),KY('R'),"    BACK",0
    if $ > 0A00h
    error "the help's page $9 has run out"
    endif

; ===========================================================================
    org 0A00h
    HBANK b
ht_hosts:
    db hh_t&0FFh, hh_1&0FFh, hh_2&0FFh, hh_3&0FFh, hh_4&0FFh
    db hh_5&0FFh, hh_6&0FFh, hh_7&0FFh, hh_8&0FFh
hh_t:   db "   HELP: HOSTS",0
hh_1:   db KY('F'),KY('I'),KY('R'),KY('E'),"   OPEN",0
hh_2:   db KY('1'),"-",KY('8'),"    OPEN",0
hh_3:   db KY('E'),"    RENAME",0
hh_4:   db KY('L'),"     LOBBY",0
hh_5:   db KY('I'),"      INFO",0
hh_6:   db KY('W'),"      WIFI",0
hh_7:   db KY('C'),KY('L'),KY('R'),"  NO COPY",0
hh_8:   db "EMPTY: NAME",0
    if $ > 0B00h
    error "the help's page $A has run out"
    endif

; ===========================================================================
    org 0B00h
    HBANK c
ht_files:
    db hf_t&0FFh, hf_1&0FFh, hf_2&0FFh, hf_3&0FFh, hf_4&0FFh
    db hf_5&0FFh, hf_6&0FFh, hf_7&0FFh, hf_8&0FFh
hf_t:   db "  HELP: FILES",0
hf_1:   db KY('F'),KY('I'),KY('R'),KY('E'),"  BOOT",0
hf_2:   db KY('E'),KY('N'),KY('T'),"   OR IN",0
hf_3:   db KY('C'),KY('L'),KY('R'),"    UP",0
hf_4:   db KY(CH_ARROW),"     PAGE",0
hf_5:   db KY('F'),"   FILTER",0
hf_6:   db "  *.BIN ETC",0
hf_7:   db KY('C'),"  COPY:THEN",0
hf_8:   db "HOST,C AGAIN",0
    if $ > 0C00h
    error "the help's page $B has run out"
    endif

; ===========================================================================
    org 0C00h
    HBANK d
ht_edit2:
    db hc_t&0FFh, hc_1&0FFh, hc_2&0FFh, hc_3&0FFh, hc_4&0FFh
    db hc_5&0FFh, hc_6&0FFh, hc_7&0FFh, hc_8&0FFh
hc_t:   db KY(CH_TIMES),"SHIFT ",KY(CH_DIV),"SYMBOL",0
hc_1:   db KY(CH_TIMES),": ",KY('1'),"! ",KY('2'),"@ ",KY('3'),"#",0
hc_2:   db KY('4'),"$ ",KY('5'),"% ",KY('6'),"^ ",KY('7'),"&",0
hc_3:   db KY('8'),"* ",KY('9'),"( ",KY('0'),") ",KY('?'),22h,0
hc_4:   db KY('+'),"; ",KY('.'),", ",KY('-'),"_ ",KY('='),":",0
hc_5:   db KY(CH_DIV),": ",KY('?'),"' ",KY('+'),"] ",KY('='),"[",0
hc_6:   db KY('.'),"> ",KY('-'),"< ",KY(CH_TIMES),"| ",KY(CH_DIV),"/",0
hc_7:   db KY('1'),"` ",KY('2'),"~ ",KY('9'),"{ ",KY('0'),"}",0
hc_8:   db "EXACT: ",KY(CH_ARROW),"TOP",0
    if $ > 0D00h
    error "the help's page $C has run out"
    endif

; ===========================================================================
    org 0D00h
    HBANK e
ht_edit1:
    db he_t&0FFh, he_1&0FFh, he_2&0FFh, he_3&0FFh, he_4&0FFh
    db he_5&0FFh, he_6&0FFh, he_7&0FFh, he_8&0FFh
he_t:   db "  HELP: TYPING",0
he_1:   db KY(CH_TIMES),"    SHIFT",0
he_2:   db KY(CH_TIMES),KY(CH_TIMES),"    LOCK",0
he_3:   db KY(CH_DIV),"  SYMBOLS",0
he_4:   db KY('C'),KY('L'),KY('R'),"  RUB OUT",0
he_5:   db KY(CH_TIMES),KY('C'),KY('L'),KY('R'),"  DELETE",0
he_6:   db KY(CH_DIV),KY('C'),KY('L'),KY('R'),"  CLEAR",0
he_7:   db KY('E'),KY('N'),KY('T'),"      OK",0
he_8:   db KY(CH_TIMES),KY('E'),KY('N'),KY('T'),"  CANCEL",0
    if $ > 0E00h
    error "the help's page $D has run out"
    endif

; ===========================================================================
    org 0E00h
    include "input.inc"
    include "snd.inc"

    PAGE_TAIL
