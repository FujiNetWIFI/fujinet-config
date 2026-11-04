; disp.inc -- Phantasy Star-style windows in Graphics I.
;
; Every screen is a main window (rows 0-17, its title in the top border)
; over a message window (rows 18-23): a two-pixel white line with rounded
; corners around a black field. Text is font.txt with tile = ASCII. The
; selection is a pointer notched into a window's left edge; it blinks,
; ticked once a frame from input.inc's frame poll.
;
; INVARIANT: never poll the frame flag (in_read, in_frames, in_wait)
; between setting a VRAM address and writing its data. Reading the status
; register resets the VDP's address latch, and the blink writes VRAM.

; tiles (tools/mkfont.py makes them)
T_TL            EQU     01H             ; window corners
T_TR            EQU     02H
T_BL            EQU     03H
T_BR            EQU     04H
T_TOP           EQU     05H             ; edges
T_BOT           EQU     06H
T_LEFT          EQU     07H
T_RIGHT         EQU     08H
T_CURSOR        EQU     09H             ; the left edge with the pointer
T_CAPL          EQU     0AH             ; progress bar ends
T_CAPR          EQU     0BH
T_BAR0          EQU     0CH             ; bar cell, 0-8 px filled: +n
T_TRI           EQU     18H             ; the splash pointer, yellow

DISP_COLS       EQU     32
MAIN_TOP        EQU     0
MAIN_BOT        EQU     17
MSG_TOP         EQU     18
MSG_BOT         EQU     23
MSG_LINE1       EQU     20              ; prompts, results, errors: typed
MSG_LINE2       EQU     22              ; the legend: at once
TEXT_L          EQU     1
TEXT_R          EQU     30

FADE_STEP       EQU     6               ; frames per fade level
FADE_FULL       EQU     3

; ---- cells and text -----------------------------------------------------

; cell_addr: HL = the name-table address of row D, column E. Preserves
; BC and DE.
cell_addr:
        LD      A,D
        AND     7
        RRCA
        RRCA
        RRCA
        OR      E
        LD      L,A
        LD      A,D
        RRCA
        RRCA
        RRCA
        AND     1FH
        ADD     A,NAME_TAB/256
        LD      H,A
        RET

; text_tile: A as a font tile; anything outside 20H-7FH is '?'.
text_tile:
        CP      20H
        JR      C,tt_bad
        CP      80H
        RET     C
tt_bad: LD      A,'?'
        RET

; disp_tile: tile A at row D, column E. Preserves BC, DE and HL.
disp_tile:
        PUSH    HL
        PUSH    AF
        CALL    cell_addr
        CALL    vdp_wr_addr
        POP     AF
        OUT     (VDP_DATA),A
        POP     HL
        RET

; disp_char: character A at row D, column E, if E <= TEXT_R. Preserves BC,
; DE and HL.
disp_char:
        PUSH    AF
        LD      A,E
        CP      TEXT_R+1
        JR      NC,dc_skip
        POP     AF
        CALL    text_tile
        JR      disp_tile
dc_skip:
        POP     AF
        RET

; disp_str: the string at HL on row D from column E, clipped at TEXT_R.
; Leaves E past what was drawn and HL past what was read. Preserves BC
; and D.
disp_str:
        LD      A,E
        CP      TEXT_R+1
        RET     NC
        PUSH    HL
        CALL    cell_addr
        CALL    vdp_wr_addr
        POP     HL
ds_loop:
        LD      A,(HL)
        OR      A
        RET     Z
        LD      A,E
        CP      TEXT_R+1
        RET     NC
        LD      A,(HL)
        CALL    text_tile
        OUT     (VDP_DATA),A
        INC     HL
        INC     E
        JR      ds_loop

; disp_strn: as disp_str, but at most B characters: for names in the
; reply window. Preserves C and D.
disp_strn:
        LD      A,B
        OR      A
        RET     Z
        LD      A,E
        CP      TEXT_R+1
        RET     NC
        PUSH    HL
        CALL    cell_addr
        CALL    vdp_wr_addr
        POP     HL
dsn_loop:
        LD      A,(HL)
        OR      A
        RET     Z
        LD      A,E
        CP      TEXT_R+1
        RET     NC
        LD      A,(HL)
        CALL    text_tile
        OUT     (VDP_DATA),A
        INC     HL
        INC     E
        DJNZ    dsn_loop
        RET

; disp_row30: columns 1-30 of row D straight from the 30 bytes at HL,
; for the name scroll. Preserves D.
disp_row30:
        LD      E,TEXT_L
        PUSH    HL
        CALL    cell_addr
        CALL    vdp_wr_addr
        POP     HL
        LD      B,TEXT_R-TEXT_L+1
dr_loop:
        LD      A,(HL)
        CALL    text_tile
        OUT     (VDP_DATA),A
        INC     HL
        DJNZ    dr_loop
        RET

; disp_row_blank: columns 1-30 of row D. Preserves C and D.
disp_row_blank:
        LD      E,TEXT_L
        CALL    cell_addr
        CALL    vdp_wr_addr
        LD      B,TEXT_R-TEXT_L+1
drb_loop:
        LD      A,' '
        OUT     (VDP_DATA),A
        NOP
        DJNZ    drb_loop
        RET

; disp_hex8: A as two hex digits at row D, column E. Leaves E past them.
disp_hex8:
        PUSH    AF
        RRCA
        RRCA
        RRCA
        RRCA
        CALL    dh_digit
        POP     AF
dh_digit:
        AND     0FH
        ADD     A,'0'
        CP      '9'+1
        JR      C,dh_put
        ADD     A,'A'-'9'-1
dh_put: CALL    disp_char
        INC     E
        RET

; disp_u8: A in decimal, no leading zeros, at row D, column E. Leaves E
; past it.
disp_u8:
        LD      HL,numbuf
        LD      B,100
        CALL    du_digit
        LD      B,10
        CALL    du_digit
        ADD     A,'0'
        LD      (HL),A
        INC     HL
        LD      (HL),0
        LD      HL,numbuf               ; skip leading zeros, keep the last
du_skip:
        LD      A,(HL)
        CP      '0'
        JR      NZ,du_draw
        INC     HL
        LD      A,(HL)
        OR      A
        JR      NZ,du_skip
        DEC     HL
du_draw:
        JP      disp_str
du_digit:
        LD      C,'0'
du_sub: CP      B
        JR      C,du_put
        SUB     B
        INC     C
        JR      du_sub
du_put: LD      (HL),C
        INC     HL
        RET

; ---- windows ------------------------------------------------------------

box_tiles:
        DB      T_TL,T_TOP,T_TR
        DB      T_LEFT,' ',T_RIGHT
        DB      T_BL,T_BOT,T_BR

; box_row: one row of a box on row D from column E to column C; A = 0 for
; the top, 1 between, 2 the bottom.
box_row:
        LD      B,A
        ADD     A,A
        ADD     A,B
        PUSH    DE
        LD      E,A
        LD      D,0
        LD      HL,box_tiles
        ADD     HL,DE
        POP     DE
        PUSH    HL
        CALL    cell_addr
        CALL    vdp_wr_addr
        POP     HL
        LD      A,(HL)
        OUT     (VDP_DATA),A
        INC     HL
        LD      A,C                     ; C-E-1 middle cells
        SUB     E
        DEC     A
        JR      Z,br_end
        LD      B,A
br_mid: LD      A,(HL)
        OUT     (VDP_DATA),A
        NOP
        DJNZ    br_mid
br_end: INC     HL
        LD      A,(HL)
        OUT     (VDP_DATA),A
        RET

; win_box: a box from row D column E to row B column C. A nonzero wipes
; it open a row a frame.
win_box:
        LD      (wb_wipe),A
        LD      A,D
        LD      (wb_y0),A
        LD      (wb_y),A
        LD      A,E
        LD      (wb_x0),A
        LD      A,B
        LD      (wb_y1),A
        LD      A,C
        LD      (wb_x1),A
wbx_row:
        LD      A,(wb_y)
        LD      D,A
        LD      B,0
        LD      A,(wb_y0)
        CP      D
        JR      Z,wbx_kind
        INC     B
        LD      A,(wb_y1)
        CP      D
        JR      NZ,wbx_kind
        INC     B
wbx_kind:
        LD      A,(wb_x0)
        LD      E,A
        LD      A,(wb_x1)
        LD      C,A
        LD      A,B
        CALL    box_row
        LD      A,(wb_wipe)
        OR      A
        LD      A,1
        CALL    NZ,in_wait
        LD      HL,wb_y
        LD      A,(wb_y1)
        CP      (HL)
        RET     Z
        INC     (HL)
        JR      wbx_row

; layout_show: the two windows, wiped in unless already up.
layout_show:
        LD      A,(layout_up)
        OR      A
        RET     NZ
        LD      DE,MAIN_TOP*256+0
        LD      BC,MAIN_BOT*256+DISP_COLS-1
        LD      A,1
        CALL    win_box
        LD      DE,MSG_TOP*256+0
        LD      BC,MSG_BOT*256+DISP_COLS-1
        LD      A,1
        CALL    win_box
        LD      A,1
        LD      (layout_up),A
        RET

; layout_drop: something else took the screen.
layout_drop:
        XOR     A
        LD      (layout_up),A
        RET

; win_title: HL in the main window's top border, a blank either side; at
; most TITLE_MAX characters, columns 3-28.
win_title:
        PUSH    HL
        LD      DE,MAIN_TOP*256+0
        LD      C,DISP_COLS-1
        XOR     A
        CALL    box_row
        POP     HL
        CALL    str_len
        LD      A,B
        OR      A
        RET     Z
        CP      TITLE_MAX+1
        JR      C,wt_fit
        LD      B,TITLE_MAX
wt_fit: PUSH    BC
        PUSH    HL
        LD      DE,MAIN_TOP*256+2
        LD      A,' '
        CALL    disp_tile
        INC     E
        CALL    cell_addr
        CALL    vdp_wr_addr
        POP     HL
wt_char:
        LD      A,(HL)
        CALL    text_tile
        OUT     (VDP_DATA),A
        INC     HL
        DJNZ    wt_char
        POP     BC
        LD      A,B
        ADD     A,3
        LD      E,A
        LD      A,' '
        JP      disp_tile

; win_clear: the main window's interior, and the cursor off.
win_clear:
        CALL    cur_hide
        LD      D,MAIN_TOP+1
wc_row: CALL    disp_row_blank
        INC     D
        LD      A,D
        CP      MAIN_BOT
        JR      C,wc_row
        RET

; msg_put: row D blanked, then HL at column 1, at once.
msg_put:
        PUSH    HL
        CALL    disp_row_blank
        POP     HL
        LD      E,TEXT_L
        JP      disp_str

; msg_type: row D blanked, then HL typed a character a frame, the way
; Phantasy Star's dialogue prints.
msg_type:
        PUSH    HL
        CALL    disp_row_blank
        POP     HL
        LD      E,TEXT_L
mt_loop:
        LD      A,(HL)
        OR      A
        RET     Z
        LD      A,E
        CP      TEXT_R+1
        RET     NC
        LD      A,(HL)
        CALL    disp_char
        INC     HL
        INC     E
        LD      A,1
        CALL    in_wait
        JR      mt_loop

; ---- pop-ups --------------------------------------------------------------
; What a pop-up covers is copied to the same place in SAVE_TAB, a mirror
; of the name table, and copied back when it closes. Nothing may draw
; under an open pop-up.

; save_cells: B cells of row D from column E into SAVE_TAB. Preserves DE.
save_cells:
        PUSH    DE
        PUSH    BC
        CALL    cell_addr
        LD      D,H
        LD      E,L
        LD      A,D
        ADD     A,(SAVE_TAB-NAME_TAB)/256
        LD      D,A
        POP     BC
        CALL    vram_copy
        POP     DE
        RET

; restore_cells: the reverse. Preserves DE.
restore_cells:
        PUSH    DE
        PUSH    BC
        CALL    cell_addr
        LD      D,H
        LD      E,L
        LD      A,H
        ADD     A,(SAVE_TAB-NAME_TAB)/256
        LD      H,A
        POP     BC
        CALL    vram_copy
        POP     DE
        RET

; ovl_open: a pop-up from row D column E to row B column C, wiped open.
ovl_open:
        LD      A,E
        LD      (ovl_x0),A
        LD      A,D
        LD      (ovl_y0),A
        LD      A,C
        LD      (ovl_x1),A
        LD      A,B
        LD      (ovl_y1),A
        PUSH    BC
        PUSH    DE
oo_save:
        LD      A,(ovl_x1)
        SUB     E
        INC     A
        LD      B,A
        CALL    save_cells
        LD      A,(ovl_y1)
        CP      D
        JR      Z,oo_saved
        INC     D
        JR      oo_save
oo_saved:
        POP     DE
        POP     BC
        LD      A,1
        JP      win_box

; ovl_close: what the pop-up covered back, a row a frame.
ovl_close:
        LD      A,(cb_active)           ; its own pointer is about to go
        OR      A
        JR      Z,oc_rows
        LD      A,(cb_row)
        LD      HL,ovl_y0
        CP      (HL)
        JR      C,oc_rows
        LD      HL,ovl_y1
        CP      (HL)
        JR      Z,oc_off
        JR      NC,oc_rows
oc_off: LD      A,(cb_col)
        LD      HL,ovl_x0
        CP      (HL)
        JR      C,oc_rows
        LD      HL,ovl_x1
        CP      (HL)
        JR      Z,oc_kill
        JR      NC,oc_rows
oc_kill:
        XOR     A
        LD      (cb_active),A
oc_rows:
        LD      A,(ovl_y0)
        LD      D,A
oc_loop:
        LD      A,(ovl_x0)
        LD      E,A
        LD      A,(ovl_x1)
        SUB     E
        INC     A
        LD      B,A
        CALL    restore_cells
        LD      A,1
        CALL    in_wait
        LD      A,(ovl_y1)
        CP      D
        RET     Z
        INC     D
        JR      oc_loop

; ---- the blinking cursor ----------------------------------------------------
; One cell alternating between two tiles, 8 frames each. Showing it
; somewhere new leaves the old place steadily on: a list's pointer stays
; lit under its menu.

; cb_put: tile A at the cursor's cell. Preserves BC and DE.
cb_put: PUSH    DE
        PUSH    AF
        LD      A,(cb_row)
        LD      D,A
        LD      A,(cb_col)
        LD      E,A
        POP     AF
        CALL    disp_tile
        POP     DE
        RET

; cur_show: the cursor at row D, column E, B its on tile, C its off tile.
cur_show:
        LD      A,(cb_active)
        OR      A
        JR      Z,cs_new
        LD      A,(cb_col)
        CP      E
        JR      NZ,cs_old
        LD      A,(cb_row)
        CP      D
        JR      Z,cs_new
cs_old: LD      A,(cb_on)
        CALL    cb_put
cs_new: LD      A,E
        LD      (cb_col),A
        LD      A,D
        LD      (cb_row),A
        LD      A,B
        LD      (cb_on),A
        LD      A,C
        LD      (cb_off),A
        XOR     A
        LD      (cb_phase),A
        INC     A
        LD      (cb_active),A
        LD      A,B
        JR      cb_put

; cur_hide: back to its off tile, and stop.
cur_hide:
        LD      A,(cb_active)
        OR      A
        RET     Z
        XOR     A
        LD      (cb_active),A
        LD      A,(cb_off)
        JR      cb_put

; cur_steady: on, until the next tick.
cur_steady:
        LD      A,(cb_active)
        OR      A
        RET     Z
        XOR     A
        LD      (cb_phase),A
        LD      A,(cb_on)
        JR      cb_put

; cur_tick: once a frame. Preserves BC and DE.
cur_tick:
        LD      A,(cb_active)
        OR      A
        RET     Z
        LD      HL,cb_phase
        INC     (HL)
        LD      A,(HL)
        AND     7
        RET     NZ
        BIT     3,(HL)
        LD      A,(cb_on)
        JR      Z,cb_put
        LD      A,(cb_off)
        JR      cb_put

; ---- sprites: the keyboard's underlines ----------------------------------

; spr_cell: sprite A under text row D, column E, in colour C. A sprite
; shows from the line after its Y, so 8 x row + 7 is the glyph's next row.
spr_cell:
        ADD     A,A
        ADD     A,A
        LD      L,A
        LD      H,SPAT_TAB/256
        CALL    vdp_wr_addr
        LD      A,D
        ADD     A,A
        ADD     A,A
        ADD     A,A
        ADD     A,7
        CALL    vdp_put
        LD      A,E
        ADD     A,A
        ADD     A,A
        ADD     A,A
        CALL    vdp_put
        XOR     A
        CALL    vdp_put
        LD      A,C
vdp_put:
        OUT     (VDP_DATA),A
        RET

; spr_end: sprites A and up are off.
spr_end:
        ADD     A,A
        ADD     A,A
        LD      L,A
        LD      H,SPAT_TAB/256
        CALL    vdp_wr_addr
        LD      A,0D0H
        OUT     (VDP_DATA),A
        RET

; ---- colours and fades -------------------------------------------------------
; Graphics I gives each group of 8 patterns one colour. The palette is
; fixed, so a fade steps each group's colour through darker ones: level 0
; is black, FADE_FULL is the colour itself.

; The colour of each group: frame and font white, the splash pointer
; yellow, the logo's twelve groups a sweep from white down to dark red.
COLOUR_TARGETS:
        DB      0FH,0FH,0FH,0BH,0FH,0FH,0FH,0FH
        DB      0FH,0FH,0FH,0FH,0FH,0FH,0FH,0FH
        DB      0FH,0FH,0BH,0BH,0AH,0AH,09H,09H
        DB      08H,08H,06H,06H,0FH,0FH,0FH,0FH

; Darker versions of each TMS colour, at levels 1 and 2.
FADE_RAMP:
        DB      1,1,0CH,0CH,1,4,1,4,1,6,1,6,1,4,4,4
        DB      1,1,2,2,4,4,6,5,6,8,6,0AH,0CH,0DH,0EH,0EH

; col_apply: the colour table at fade level A.
col_apply:
        LD      (fade_level),A
        LD      C,A
        LD      HL,COLR_TAB
        CALL    vdp_wr_addr
        LD      HL,COLOUR_TARGETS
        LD      B,32
ca_loop:
        LD      A,(HL)
        INC     HL
        PUSH    HL
        CALL    fade_colour
        RLCA                            ; foreground; background transparent
        RLCA
        RLCA
        RLCA
        OUT     (VDP_DATA),A
        POP     HL
        DJNZ    ca_loop
        RET

; fade_colour: A = colour A at fade level C. Preserves BC.
fade_colour:
        LD      E,A
        LD      A,C
        CP      FADE_FULL
        LD      A,E
        RET     NC
        LD      A,C
        OR      A
        LD      A,1                     ; level 0: black
        RET     Z
        LD      D,0
        LD      HL,FADE_RAMP
        ADD     HL,DE
        LD      A,C
        CP      1
        JR      Z,fc_get
        LD      DE,16
        ADD     HL,DE
fc_get: LD      A,(HL)
        RET

; fade_out / fade_in: a level every FADE_STEP frames.
fade_out:
        LD      A,(fade_level)
        OR      A
        RET     Z
        LD      A,FADE_STEP
        CALL    in_wait
        LD      A,(fade_level)
        DEC     A
        CALL    col_apply
        JR      fade_out

fade_in:
        LD      A,(fade_level)
        CP      FADE_FULL
        RET     NC
        LD      A,FADE_STEP
        CALL    in_wait
        LD      A,(fade_level)
        INC     A
        CALL    col_apply
        JR      fade_in

; ---- setup -------------------------------------------------------------------

vdp_regs:
        DB      00H                     ; R0: Graphics I
        DB      80H                     ; R1: 16K, display off
        DB      NAME_TAB/400H           ; R2
        DB      COLR_TAB/40H            ; R3
        DB      PATT_TAB/800H           ; R4
        DB      SPAT_TAB/80H            ; R5
        DB      SPPT_TAB/800H           ; R6
        DB      01H                     ; R7: black backdrop

; disp_init: registers, tiles, colours, a blank screen, no sprites.
disp_init:
        LD      HL,vdp_regs
        LD      B,0
di_reg: LD      A,(HL)
        CALL    vdp_reg
        INC     HL
        INC     B
        LD      A,B
        CP      8
        JR      C,di_reg
        LD      HL,FONT_PATTERNS
        LD      DE,PATT_TAB
        LD      BC,128*8
        CALL    vdp_write
        LD      HL,SPRITE_PATTERNS
        LD      DE,SPPT_TAB
        LD      BC,8
        CALL    vdp_write
        LD      A,0
        CALL    spr_end
        LD      HL,NAME_TAB
        LD      BC,DISP_COLS*24
        LD      A,' '
        CALL    vdp_fill
        XOR     A
        LD      (cb_active),A
        LD      (layout_up),A
        LD      A,FADE_FULL
        CALL    col_apply
        LD      A,0C0H                  ; display on, interrupt off
        LD      B,1
        JP      vdp_reg
