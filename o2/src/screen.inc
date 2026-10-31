; screen.inc -- the main program's side of the display: it writes the fixed
; half's tables (fixed.inc) through the mailbox's POKE register and leaves
; the VDC to the interrupts. The band list, the screen, the selection bar.
; One 256-byte page: the template with the code that copies it.
; Main program, P1 = P1_MAIN, register bank 0.

; poke_at: aim POKE at the fixed half's address r2:A (r2 = $4-$7). Leaves the
; register file selecting POKE, so each poke_b after it is one MOVX.
; Clobbers A, r0, r1.
poke_at:
    mov r1,a
    mov r0,#FN_W_REGSEL
    mov a,#FN_REG_POKE_LO
    movx @r0,a
    inc r0
    mov a,r1
    movx @r0,a
    dec r0
    mov a,#FN_REG_POKE_HI
    movx @r0,a
    inc r0
    mov a,r2
    add a,#0FCh                 ; - 4: offset from $400
    movx @r0,a
; poke_sel: select POKE again, the pointer where it was (a reply read
; selects RXWIN). Clobbers A, r0.
poke_sel:
    mov r0,#FN_W_REGSEL
    mov a,#FN_REG_POKE
    movx @r0,a
    ret

; poke_b: store A at the POKE pointer and advance it. Clobbers r0.
poke_b:
    mov r0,#FN_W_REGDATA
    movx @r0,a
    ret

; poke_n: r6 copies of A at the POKE pointer. Clobbers r0, r6.
poke_n:
    call poke_b
    djnz r6,poke_n
    ret

; copy_rom: r6 bytes from offset r4 of this page to the POKE pointer.
copy_rom:
    mov a,r4
    movp a,@a
    call poke_b
    inc r4
    djnz r6,copy_rom
    ret

; spr_hide: the four sprites below the screen. Clobbers A, r0-r2, r6.
spr_hide:
    mov r2,#SPRTAB>>8
    mov a,#SPRTAB&0FFh
    call poke_at
    mov r6,#4
sh_l:
    mov a,#SPR_OFF
    call poke_b
    clr a
    call poke_b
    clr a
    call poke_b
    djnz r6,sh_l
    ret

; spr_at: sprite r3's (Y r4, X r5, attribute r2) into SPRTAB.
; Clobbers A, r0-r2, r6.
spr_at:
    mov a,r2
    mov r6,a
    mov a,r3
    rl a
    add a,r3                    ; *3
    add a,#SPRTAB&0FFh
    mov r2,#SPRTAB>>8
    call poke_at
    mov a,r4
    call poke_b
    mov a,r5
    call poke_b
    mov a,r6
    jmp poke_b

; grid_rows: GRIDTAB's ten columns = A (bit r: row r filled). Clobbers A,
; r0-r2, r6.
grid_rows:
    mov r6,a
    mov r2,#GRIDTAB>>8
    mov a,#GRIDTAB&0FFh
    call poke_at
    mov a,r6
    mov r6,#10
    jmp poke_n

; grid_col: GRIDTAB's column r3 = r5 (the scrollbar is column 9, the boot's
; bar a row of them). Clobbers A, r0-r2.
grid_col:
    mov a,r3
    add a,#GRIDTAB&0FFh
    mov r2,#GRIDTAB>>8
    call poke_at
    mov a,r5
    jmp poke_b

; bar_row: the selection bar on row A (0FFh: none): columns 0-8, the
; scrollbar's column left alone. Clobbers A, r0-r2, r6.
bar_row:
    mov r6,a
    inc a
    jz br_set                   ; (A = 0: no bar)
    inc r6
    mov a,#80h
br_l:
    rl a                        ; 1 << row
    djnz r6,br_l
br_set:
    mov r6,a
    mov r2,#GRIDTAB>>8
    mov a,#GRIDTAB&0FFh
    call poke_at
    mov a,r6
    mov r6,#9
    jmp poke_n

; scr_text: a new screen: eight blank rows between T0 and T1, the band list,
; the colours, no bar, the sprites hidden. Rows first, then the band list:
; a frame between them draws the old screen. Clobbers A, r0-r7.
scr_text:
    call lb_clear
    mov r0,#RICON
    clr a
    movx @r0,a
    mov r4,#0
st_r:
    mov r0,#TMP0                ; (row_line keeps no register)
    mov a,r4
    movx @r0,a
    call col_white
    call row_line
    mov r0,#TMP0
    movx a,@r0
    inc a
    mov r4,a
    xrl a,#NROWS
    jnz st_r
    clr a
    call grid_rows
    call spr_hide
    mov r2,#SCRCOL>>8           ; the colour and the band list
    mov a,#SCRCOL&0FFh
    call poke_at
    mov r4,#scr_t&0FFh
    mov r6,#2+NBANDS*3
    call copy_rom
    mov r1,#MODE                ; PAL: the grid's band padded for its line
    mov a,@r1
    jb7 st_pal
    jmp st_mode
st_pal:
    mov r2,#BANDS>>8
    mov a,#(BANDS+3)&0FFh
    call poke_at
    mov a,#J_GRIDP
    call poke_b
st_mode:
    mov r1,#MODE                ; (the PAL bit stays)
    mov a,@r1
    orl a,#MODE_SETUP
    mov @r1,a
    ; fall into scr_lines

; scr_lines: T0 and T1 blank. Clobbers A, r0-r7.
scr_lines:
    call lb_clear
    mov r4,#L_TOP
    call say_line
    call lb_clear
    mov r4,#L_BOT
    jmp say_line

; The bands' lines. Row r's characters are at Y_ROW0 + 24r; the band before
; it starts when row r-1's have ended, LEAD lines early (the interrupt takes
; about a line to reach its first write).
BL_TOP  equ 1                   ; row 0 and the grid, before T0 shows
BL_GRID equ Y_T0+13-LEAD        ; T0 has ended: the grid on (h_grid pads its
                                ; write into the blank)
BL_ROW1 equ Y_ROW0+ROW_PITCH-10-LEAD
BL_T1   equ Y_ROW0+7*ROW_PITCH+14-LEAD
scr_t:
    db BG_BLACK|GRID_BLUE
    db 256-BL_TOP
    db J_TOP, (ROWS&0FFh)+ROW_TAB, 256-(BL_GRID-BL_TOP)
    db J_GRID, 0, 256-(BL_ROW1-BL_GRID)
K   set 2
    rept 6                      ; rows 1-6, each staging the next (its table)
    db J_ROW, (ROWS&0FFh)+K*ROW_TAB, 256-ROW_PITCH
K   set K+1
    endm
    db J_ROW, 0FFh, 256-(BL_T1-(BL_ROW1+6*ROW_PITCH))  ; row 7
    db J_TXT, T1&0FFh, 0        ; T1: the last
