; p1.inc -- page 1: the display's routines -- the character generator, the
; lines and rows, the screen, the sprites, the bar -- which every other page
; reaches by farcall (local.inc's doors). They take their arguments in r2-r5
; and LINEBUF (farcall spends A, r0, r1, r6 and r7 getting here).

    PAGE_HEAD
    db l_sayline&0FFh           ; L_SAYLINE
    db l_saylinec&0FFh          ; L_SAYLINEC
    db l_rowstr&0FFh            ; L_ROWSTR
    db l_rowline&0FFh           ; L_ROWLINE
    db l_scrtext&0FFh           ; L_SCRTEXT
    db l_bar&0FFh               ; L_BAR
    db l_grid&0FFh              ; L_GRID
    db l_sprat&0FFh             ; L_SPRAT
    db l_sprhide&0FFh           ; L_SPRHIDE
    db l_gcol&0FFh              ; L_GCOL
    db l_folder&0FFh            ; L_FOLDER
    db l_color&0FFh             ; L_COLOR
l_sayline:
    jmp say_line
l_saylinec:
    jmp say_line_c
l_rowstr:
    jmp row_str
l_rowline:
    jmp row_line
l_scrtext:
    jmp scr_text
l_bar:
    mov a,r5
    jmp bar_row
l_grid:
    mov a,r5
    jmp grid_rows
l_sprat:
    jmp spr_at
l_sprhide:
    jmp spr_hide
l_gcol:
    jmp grid_col
l_folder:
    jmp folder
l_color:
    jmp color

; color: the screen's colour register ($A3: background, grid) = r5.
; Clobbers A, r0-r2.
color:
    mov r2,#SCRCOL>>8
    mov a,#SCRCOL&0FFh
    call poke_at
    mov a,r5
    jmp poke_b

; folder: sprite 1 is the folder, tan, left of the rows (its Y is each
; row's: row_line). The VDC takes a shape with the display on. Clobbers A,
; r0-r6.
folder:
    mov a,#P1_VDC
    outl p1,a
    mov r0,#vdc_spr1_shape
    mov r1,#fold&0FFh
    mov r2,#8
fo_l:
    mov a,r1
    movp a,@a
    movx @r0,a
    inc r0
    inc r1
    djnz r2,fo_l
    mov a,#P1_MAIN
    outl p1,a
    mov r3,#1
    mov r4,#SPR_OFF
    mov r5,#X_ICON
    mov r2,#C_TAN<<3
    jmp spr_at
fold:                           ; bit 0 is the leftmost pixel
    db 07h, 7Fh, 7Fh, 7Fh, 7Fh, 7Fh, 7Fh, 00h

; ===========================================================================
    org 900h
    include "disp.inc"

; ===========================================================================
    org 0A00h
    include "text.inc"

; ===========================================================================
    org 0B00h
    include "screen.inc"

    PAGE_TAIL
