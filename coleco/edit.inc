; edit.inc -- the on-screen keyboard, laid out like Phantasy Star's
; name-entry screen: characters on every other column of every other row,
; a cyan underline under the one the cursor is on, a white one where the
; next character goes. The 63 characters from '!' to '_' fill 13 columns
; by 5 rows; space is an action.
;
;   - CASE redraws the grid in lowercase (and @[\]^ as `{|}~), so what
;     will be typed is what shows.
;   - ESC is the only cancel. The right button rubs out, and must never
;     discard an edit: that is the key pressed again and again.
;   - Keypad digits type themselves; # and * end the edit.
;
; It runs NO mailbox transactions: callers hold data in the reply window
; across it (rename reads the slots, edits one, streams all 8 back).

ED_VALUE_ROW    EQU     2
ED_VALUE_W      EQU     29              ; shown; the marker takes column 30
ED_GRID_COL     EQU     4
ED_GRID_ROW     EQU     6
ED_GRID_COLS    EQU     13
ED_GRID_ROWS    EQU     5
ED_ACT_ROW      EQU     16
ED_ACT_CELLS    EQU     5
ED_SPR_MARK     EQU     4               ; sprites 0-3: the cursor's line
SPR_CYAN        EQU     07H
SPR_WHITE       EQU     0FH

; letters first; 0 is an empty cell the cursor steps around
kb_cells:
        DB      "ABCDEFGHIJKLM"
        DB      "NOPQRSTUVWXYZ"
        DB      "0123456789.-_"
        DB      21H,22H,23H,24H,25H,26H,27H,28H,29H,2AH,2BH,2CH,2FH
        DB      3AH,3BH,3CH,3DH,3EH,3FH,40H,5BH,5CH,5DH,5EH,0,0

act_text:
        DW      str_a_case,str_a_spc,str_a_rub,str_a_end,str_a_esc
act_col:
        DB      4,10,15,20,25
act_len:
        DB      4,3,3,3,3
grid_to_act:                            ; the action nearest a column
        DB      0,0,0,1,1,1,2,2,3,3,3,4,4
act_to_grid:                            ; and back
        DB      1,3,6,8,11

; kb_index: HL onto kb_cells for column B, row C. Preserves BC.
kb_index:
        LD      A,C                     ; x 13 = (x 3) x 4 + x 1
        ADD     A,A
        ADD     A,C
        ADD     A,A
        ADD     A,A
        ADD     A,C
        ADD     A,B
        LD      E,A
        LD      D,0
        LD      HL,kb_cells
        ADD     HL,DE
        RET

; cell_char: A = the character at column B, row C, cased; 0 if empty.
; Preserves BC.
cell_char:
        CALL    kb_index
        LD      A,(HL)
        OR      A
        RET     Z
        LD      E,A
        LD      A,(ed_case)
        OR      A
        LD      A,E
        RET     Z
        CP      40H
        RET     C
        CP      5FH
        RET     NC
        ADD     A,20H
        RET

draw_grid:
        LD      C,0
dg_row: LD      B,0
dg_col: CALL    cell_char
        OR      A
        JR      NZ,dg_put
        LD      A,' '
dg_put: PUSH    AF
        LD      A,C
        ADD     A,A
        ADD     A,ED_GRID_ROW
        LD      D,A
        LD      A,B
        ADD     A,A
        ADD     A,ED_GRID_COL
        LD      E,A
        POP     AF
        CALL    disp_char
        INC     B
        LD      A,B
        CP      ED_GRID_COLS
        JR      C,dg_col
        INC     C
        LD      A,C
        CP      ED_GRID_ROWS
        JR      C,dg_row
        RET

draw_actions:
        LD      B,0
da_loop:
        PUSH    BC
        LD      HL,act_col
        LD      E,B
        LD      D,0
        ADD     HL,DE
        LD      A,(HL)
        LD      HL,act_text
        ADD     HL,DE
        ADD     HL,DE
        LD      E,(HL)
        INC     HL
        LD      H,(HL)
        LD      L,E
        LD      E,A
        LD      D,ED_ACT_ROW
        CALL    disp_str
        POP     BC
        INC     B
        LD      A,B
        CP      ED_ACT_CELLS
        JR      C,da_loop
        RET

; place_cursor: the underline under the grid character, or one per letter
; under an action word.
place_cursor:
        LD      A,(ed_y)
        CP      ED_GRID_ROWS
        JR      Z,pc_act
        ADD     A,A
        ADD     A,ED_GRID_ROW
        LD      D,A
        LD      A,(ed_x)
        ADD     A,A
        ADD     A,ED_GRID_COL
        LD      E,A
        LD      B,1
        JR      pc_place
pc_act: LD      D,ED_ACT_ROW
        LD      A,(ed_x)
        LD      C,A
        LD      B,0
        LD      HL,act_col
        ADD     HL,BC
        LD      E,(HL)
        LD      HL,act_len
        ADD     HL,BC
        LD      B,(HL)
pc_place:
        LD      A,0                     ; A = sprite
pc_spr: PUSH    AF
        PUSH    BC
        PUSH    DE
        CP      B
        JR      C,pc_on
        LD      DE,24*256+0             ; parked below the screen
pc_on:  LD      C,SPR_CYAN
        CALL    spr_cell
        POP     DE
        POP     BC
        POP     AF
        INC     E
        INC     A
        CP      4
        JR      C,pc_spr
        RET

; draw_value: the tail of the buffer, the white marker where the next
; character lands. Editing is append and rub only.
draw_value:
        LD      D,ED_VALUE_ROW
        CALL    disp_row_blank
        LD      A,(ed_len)
        LD      C,0                     ; C = the first shown
        SUB     ED_VALUE_W
        JR      C,dv_draw
        LD      C,A
dv_draw:
        LD      HL,fn_entry
        LD      B,0
        ADD     HL,BC
        LD      A,(ed_len)
        SUB     C
        LD      B,A                     ; B = how many show
        PUSH    BC
        LD      DE,ED_VALUE_ROW*256+TEXT_L
        CALL    disp_strn
        POP     BC
        LD      A,B
        ADD     A,TEXT_L
        LD      E,A
        LD      D,ED_VALUE_ROW
        LD      C,SPR_WHITE
        LD      A,ED_SPR_MARK
        JP      spr_cell

; move_to: the cursor to column B, row C.
move_to:
        LD      A,B
        LD      (ed_x),A
        LD      A,C
        LD      (ed_y),A
        CALL    place_cursor
        LD      A,SND_MOVE
        JP      snd_play

; type_char: A onto the end, unless the buffer is full.
type_char:
        LD      B,A
        LD      A,(ed_max)
        LD      C,A
        LD      A,(ed_len)
        CP      C
        RET     NC
        LD      E,A
        LD      D,0
        LD      HL,fn_entry
        ADD     HL,DE
        LD      (HL),B
        INC     HL
        LD      (HL),0
        INC     A
        LD      (ed_len),A
        CALL    draw_value
        LD      A,SND_TYPE
        JP      snd_play

rub_out:
        LD      A,(ed_len)
        OR      A
        RET     Z
        DEC     A
        LD      (ed_len),A
        LD      E,A
        LD      D,0
        LD      HL,fn_entry
        ADD     HL,DE
        LD      (HL),0
        CALL    draw_value
        LD      A,SND_BACK
        JP      snd_play

; fn_edit: the keyboard over fn_entry, titled HL, prompting with DE, at
; most A characters. Carry clear when ended, set when escaped. fn_entry
; changes either way.
fn_edit:
        CP      ENTRY_MAX
        JR      C,fe_max
        LD      A,ENTRY_MAX-1
fe_max: LD      (ed_max),A
        PUSH    DE
        PUSH    HL
        LD      E,A
        LD      D,0
        LD      HL,fn_entry
        ADD     HL,DE
        LD      (HL),0
        LD      HL,fn_entry
        CALL    str_len
        LD      A,B
        LD      (ed_len),A
        XOR     A
        LD      (ed_case),A
        LD      (ed_x),A
        LD      (ed_y),A
        CALL    layout_show
        CALL    win_clear
        POP     HL
        CALL    win_title
        LD      D,MSG_LINE1
        CALL    disp_row_blank
        LD      HL,str_leg_edit
        CALL    legend_line
        CALL    draw_grid
        CALL    draw_actions
        LD      A,ED_SPR_MARK+1
        CALL    spr_end
        CALL    draw_value
        CALL    place_cursor
        POP     HL
        CALL    status_line
fe_loop:
        CALL    in_read
        LD      HL,ed_x
        LD      B,(HL)
        INC     HL
        LD      C,(HL)                  ; B = column, C = row
        CP      EV_UP
        JR      Z,fe_up
        CP      EV_DOWN
        JR      Z,fe_down
        CP      EV_LEFT
        JR      Z,fe_left
        CP      EV_RIGHT
        JR      Z,fe_right
        CP      EV_FIRE
        JP      Z,fe_fire
        CP      EV_BACK
        JR      Z,fe_rub
        CP      EV_MENU
        JP      Z,fe_end
        CP      EV_HASH
        JP      Z,fe_end
        CP      EV_KEY0
        JR      C,fe_loop
        CP      EV_KEY0+10
        JR      NC,fe_loop
        ADD     A,'0'-EV_KEY0           ; a digit types itself
        CALL    type_char
        JR      fe_loop
fe_rub: CALL    rub_out
        JR      fe_loop
fe_up:  LD      A,C
        CP      ED_GRID_ROWS
        JR      NZ,fe_up_grid
        LD      HL,act_to_grid          ; back into the grid near the word
        LD      E,B
        LD      D,0
        ADD     HL,DE
        LD      B,(HL)
        LD      C,ED_GRID_ROWS-1
fe_up_cell:
        CALL    kb_index
        LD      A,(HL)
        OR      A
        JR      NZ,fe_go
        DEC     B
        JR      fe_up_cell
fe_up_grid:
        OR      A
        JR      Z,fe_loop
        DEC     C
fe_go:  CALL    move_to
        JR      fe_loop
fe_down:
        LD      A,C
        CP      ED_GRID_ROWS
        JP      Z,fe_loop
        CP      ED_GRID_ROWS-1
        JR      Z,fe_to_act
        INC     C
        CALL    kb_index
        LD      A,(HL)
        OR      A
        JR      NZ,fe_go
        DEC     C
fe_to_act:
        LD      HL,grid_to_act          ; the action nearest the column
        LD      E,B
        LD      D,0
        ADD     HL,DE
        LD      B,(HL)
        LD      C,ED_GRID_ROWS
        JR      fe_go
fe_left:
        LD      A,B
        OR      A
        JP      Z,fe_loop
        DEC     B
        JR      fe_go
fe_right:
        INC     B
        LD      A,C
        CP      ED_GRID_ROWS
        JR      NZ,fe_right_grid
        LD      A,B
        CP      ED_ACT_CELLS
        JP      NC,fe_loop
        JR      fe_go
fe_right_grid:
        LD      A,B
        CP      ED_GRID_COLS
        JP      NC,fe_loop
        CALL    kb_index
        LD      A,(HL)
        OR      A
        JR      NZ,fe_go
        JP      fe_loop
fe_fire:
        LD      A,C
        CP      ED_GRID_ROWS
        JR      Z,fe_action
        CALL    cell_char
        CALL    type_char
        JP      fe_loop
fe_action:
        LD      A,B
        OR      A
        JR      NZ,fe_spc
        LD      A,(ed_case)             ; CASE
        XOR     1
        LD      (ed_case),A
        CALL    draw_grid
        LD      A,SND_OK
        CALL    snd_play
        JP      fe_loop
fe_spc: DEC     A
        JR      NZ,fe_rub2
        LD      A,' '
        CALL    type_char
        JP      fe_loop
fe_rub2:
        DEC     A
        JP      Z,fe_rub
        DEC     A
        JR      NZ,fe_esc
fe_end: CALL    fe_finish
        LD      A,SND_OK
        CALL    snd_play
        OR      A
        RET
fe_esc: CALL    fe_finish
        LD      A,SND_BACK
        CALL    snd_play
        SCF
        RET
fe_finish:
        XOR     A
        JP      spr_end
