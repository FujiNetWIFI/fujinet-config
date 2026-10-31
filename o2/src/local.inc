; local.inc -- what every screen's page carries for the display: the doors
; into the library (page 1, by farcall: arguments in r2-r5 and LINEBUF;
; A, r0, r1, r6 and r7 are spent), LINEBUF's clear and its colours.
; Main program, P1 = P1_MAIN.

LDOOR   macro entry
    mov r7,#PG_LIB
    mov a,#entry
    jmp lib_go
    endm
EDOOR   macro entry             ; (the editor's library: p10.inc)
    mov r7,#PG_EDLIB
    mov a,#entry
    jmp lib_go
    endm

; say_line: LINEBUF onto line r4 (L_TOP, L_BOT), white; say_line_c: in
; TXCOL / TXCOL2 as they are.
say_line:
    LDOOR L_SAYLINE
say_line_c:
    LDOOR L_SAYLINEC
; row_str: LINEBUF onto row r4, white, no folder; row_line: in TXCOL /
; TXCOL2, a folder if RICON.
row_str:
    LDOOR L_ROWSTR
row_line:
    LDOOR L_ROWLINE
; scr_text: a new screen, blank, the bar off, the sprites hidden.
scr_text:
    LDOOR L_SCRTEXT
; bar_row: the bar on row A (0FFh: none); grid_rows: the grid's columns = A.
bar_row:
    mov r5,a
    LDOOR L_BAR
grid_rows:
    mov r5,a
    LDOOR L_GRID
; grid_col: the grid's column r3 = r5.
grid_col:
    LDOOR L_GCOL
; color: the screen's colour register = r5.
color:
    LDOOR L_COLOR
; folder: sprite 1 the folder beside a row (row_line's RICON).
folder:
    LDOOR L_FOLDER
; spr_at: sprite r3: Y r4, X r5, attribute r2. spr_hide: all four gone.
spr_at:
    LDOOR L_SPRAT
spr_hide:
    LDOOR L_SPRHIDE
    ifdef EDPAGE
; the editor's (edtext.inc): T0's prompt, row 0's status, the chart on row
; 7 and T1; the magnifier on character r5 (0FFh: none).
ed_prompt:
    EDOOR L_EDPROMPT
ed_status:
    EDOOR L_EDSTAT
ed_chart:
    EDOOR L_EDCHART
mag:
    EDOOR L_MAG
    endif
lib_go:
    FCALL farcall
    ret

; lb_clear: LINEBUF to spaces. Clobbers A, r0, r2.
lb_clear:
    mov r0,#LINEBUF
    mov r2,#TXT_LEN
    mov a,#' '
lbc_l:
    movx @r0,a
    inc r0
    djnz r2,lbc_l
    ret

; col_white: TXCOL and TXCOL2 white; col_set: = A. Clobbers r0.
col_white:
    mov a,#C_WHITE
col_set:
    mov r0,#TXCOL
    movx @r0,a
    inc r0
    movx @r0,a
    ret
