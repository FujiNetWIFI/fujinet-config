' input.bas -- edge-detected controller input, shared by every screen.
'
' Uses the unqualified CONT.* pseudo-variables, which OR together both
' controller ports -- this is a single-player utility, so there's no reason
' to force player 1's jack specifically.
'
' in_poll() populates, once per call:
'   in_disc  - DISC_UP/DOWN/LEFT/RIGHT on a fresh press or an auto-repeat
'              tick while held; 0 otherwise.
'   in_btn   - 1 on a fresh action-button press (any of B0/B1/B2); 0 otherwise.
'   in_key   - the decoded keypad digit/CLEAR/ENTER on a fresh press, or
'              KEYPAD_NONE (12) if nothing new. CONT.KEY costs 6 extra
'              8-bit vars per the IntyBASIC manual -- accounted for in the
'              constants.bas RAM budget comment.

    CONST IN_REPEAT_DELAY = 18   ' frames held before auto-repeat kicks in (~0.3s)
    CONST IN_REPEAT_RATE  = 6    ' frames between repeats once repeating (~0.1s)

    DIM in_disc, in_pdisc, in_rdelay
    DIM in_braw, in_btn,  in_pbtn
    DIM in_key,  in_pkey

' ---------------------------------------------------------------------------
' in_poll: call once per frame (after WAIT). Sets in_disc/in_btn/in_key.
' ---------------------------------------------------------------------------
in_poll: PROCEDURE
    ' --- disc, with auto-repeat while held in one direction ---
    in_disc = 0
    IF CONT.UP THEN in_disc = DISC_UP
    IF CONT.DOWN THEN in_disc = DISC_DOWN
    IF CONT.LEFT THEN in_disc = DISC_LEFT
    IF CONT.RIGHT THEN in_disc = DISC_RIGHT

    IF in_disc <> 0 THEN
        IF in_disc <> in_pdisc AND in_pdisc = 0 THEN
            in_rdelay = IN_REPEAT_DELAY     ' fresh press from neutral -- fire now
        ELSE
            ' Same direction held, OR slid/wobbled to a neighbour without
            ' releasing. Both go through the repeat gate, and that "OR" is the
            ' point: the disc has 16 positions and the diagonals set TWO
            ' cardinal bits, so ENE and NE read as both UP and RIGHT (jzintv
            ' src/pads/pads.c, "Pad bit 1 is set for SSE through NE / Pad bit 2
            ' is set for ENE through NW"). The tests above are last-match-wins,
            ' so a thumb parked near that boundary alternates in_disc between
            ' DISC_UP and DISC_RIGHT frame to frame. Treating a changed
            ' direction as a fresh press fired the handler every other frame
            ' instead of every seventh, which is what turned the list screens'
            ' redraw into a continuous flicker.
            '
            ' The cost is that changing direction without releasing the disc
            ' waits up to IN_REPEAT_RATE frames (~0.1s) instead of acting at
            ' once. Not perceptible, and it stops the cursor racing.
            IF in_rdelay > 0 THEN
                in_rdelay = in_rdelay - 1
                in_disc = 0
            ELSE
                in_rdelay = IN_REPEAT_RATE
            END IF
        END IF
    END IF
    in_pdisc = 0
    IF CONT.UP THEN in_pdisc = DISC_UP
    IF CONT.DOWN THEN in_pdisc = DISC_DOWN
    IF CONT.LEFT THEN in_pdisc = DISC_LEFT
    IF CONT.RIGHT THEN in_pdisc = DISC_RIGHT

    ' --- action buttons, edge-triggered (no repeat) ---
    in_braw = 0
    IF CONT.B0 OR CONT.B1 OR CONT.B2 THEN in_braw = 1
    IF in_braw <> 0 AND in_pbtn = 0 THEN
        in_btn = 1
    ELSE
        in_btn = 0
    END IF
    in_pbtn = in_braw

    ' --- keypad, edge-triggered ---
    in_key = KEYPAD_NONE
    IF CONT.KEY <> KEYPAD_NONE AND CONT.KEY <> in_pkey THEN
        in_key = CONT.KEY
    END IF
    in_pkey = CONT.KEY
END

' =============================================================================
' grid_entry: on-screen character-grid text entry (constants.bas GRID_*).
' All 95 printable characters fit in 6 rows x 16 columns, so the cursor
' position IS the character (ch = 32 + gy*16 + gx) -- no SHIFT/SYM paging.
' A 4th row of action buttons (SPC/DEL/OK/ESC) sits below it.
'
' Contract: caller sets #ge_dst (destination buffer) and g_max (its size,
' including the NUL) BEFORE calling, and is responsible for priming the
' buffer's content -- NUL-terminate offset 0 for a fresh empty field, or
' leave existing NUL-terminated text in place to edit it (grid_entry scans
' for the existing length rather than assuming empty, so pre-loaded text is
' preserved and editable). #ge_dst is mutated live as the user types,
' REGARDLESS of whether they eventually accept or cancel -- so a caller
' editing something that must survive a cancel unmutated (e.g. one slot of
' the 8-slot host table) must point #ge_dst at a scratch copy (SC_EDIT),
' never at the original, and only copy the result back on fn_ok=1.
'
' Returns fn_ok = 1 (ENTER or the OK button) or 0 (the ESC button only --
' there is no keypad shortcut for cancel, to avoid an accidental CLEAR
' during backspacing from discarding the whole edit). g_len holds the
' final length; #ge_dst is NUL-terminated at that offset.
' =============================================================================

    DIM g_x, g_y, g_px, g_py, g_len, g_max, g_ch, ga_idx
    DIM #ge_dst

' ---------------------------------------------------------------------------
' grid_video: program the grid's color stack. Called by each of the four
' callers BEFORE its scr_clear, so no frame of the outgoing screen is ever
' reinterpreted under the new palette.
'
' Three runs, three slots, one spare -- and unlike the file browser both
' boundaries are at fixed rows, so there are no selection-dependent edge
' cases and nothing has to be re-armed as the cursor moves.
'
' vid_now is left at a sentinel that can never equal vid_want (0-3), so
' config.bas's main loop always re-establishes the real screen's profile when
' grid_entry returns. Safe because every caller redraws on the way out.
' ---------------------------------------------------------------------------
grid_video: PROCEDURE
    MODE 0,CS_BLUE,CS_DARKGREEN,CS_PURPLE,CS_BLACK : BORDER CS_BLUE
    vid_now = 255
    WAIT
END

grid_entry: PROCEDURE
    GOSUB grid_draw_charset
    GOSUB grid_draw_actions
    ' Rows 0-2 blue (p0), 3-9 dark green (p1), 10-11 purple (p2). One pass is
    ' enough: the caller's scr_clear zeroed every word, and neither advance
    ' cell is ever written again -- the charset starts at GRID_COL0 and the
    ' action buttons at GRID_ACT_COL0, both column 2.
    #BACKTAB(GRID_PANEL_ROW * SCREEN_COLS) = CS_ADVANCE
    #BACKTAB(GRID_ACTION_ROW * SCREEN_COLS) = CS_ADVANCE

    g_len = 0
    WHILE (g_len < g_max - 1) AND ((PEEK(#ge_dst + g_len) AND 255) <> 0)
        g_len = g_len + 1
    WEND
    GOSUB grid_draw_value

    g_x = 0 : g_y = 0
    g_px = 255 : g_py = 255

    DO WHILE 1
        WAIT
        GOSUB grid_draw_cursor
        GOSUB in_poll

        IF in_disc = DISC_UP AND g_y > 0 THEN g_y = g_y - 1
        IF in_disc = DISC_DOWN AND g_y < GRID_ROWS THEN g_y = g_y + 1
        IF in_disc = DISC_LEFT AND g_x > 0 THEN g_x = g_x - 1
        IF in_disc = DISC_RIGHT THEN
            IF g_y = GRID_ROWS THEN
                IF g_x < 3 THEN g_x = g_x + 1
            ELSE
                IF g_x < GRID_COLS - 1 THEN g_x = g_x + 1
            END IF
        END IF
        IF g_y = GRID_ROWS AND g_x > 3 THEN g_x = 3

        IF in_key = KEYPAD_0 THEN g_ch = 32 : GOSUB grid_append
        IF in_key = KEYPAD_CLEAR THEN GOSUB grid_backspace
        IF in_key = KEYPAD_ENTER THEN
            fn_ok = 1
            EXIT DO
        END IF

        IF in_btn <> 0 THEN
            IF g_y < GRID_ROWS THEN
                g_ch = 32 + g_y * GRID_COLS + g_x
                IF g_ch <= 126 THEN GOSUB grid_append
            ELSE
                IF g_x = 0 THEN g_ch = 32 : GOSUB grid_append
                IF g_x = 1 THEN GOSUB grid_backspace
                IF g_x = 2 THEN
                    fn_ok = 1
                    EXIT DO
                END IF
                IF g_x = 3 THEN
                    fn_ok = 0
                    EXIT DO
                END IF
            END IF
        END IF
    LOOP

    SPRITE 0, 0, 0, 0      ' or the cursor block lingers over the next screen
END

' grid_draw_charset: paints all 96 cells (95 real chars + one always-blank).
grid_draw_charset: PROCEDURE
    FOR g_y = 0 TO GRID_ROWS - 1
        FOR g_x = 0 TO GRID_COLS - 1
            g_ch = 32 + g_y * GRID_COLS + g_x
            IF g_ch > 126 THEN g_ch = 32
            #BACKTAB((GRID_ROW0 + g_y) * SCREEN_COLS + GRID_COL0 + g_x) = (g_ch - 32) * 8 + COL_NORMAL
        NEXT g_x
    NEXT g_y
END

' White, not COL_DIM: COL_DIM is blue, which was invisible on the blue bar
' originally specified for this row and still poor on the purple one.
grid_draw_actions: PROCEDURE
    PRINT AT screenpos(GRID_ACT_COL0, GRID_ACTION_ROW) COLOR COL_NORMAL,"SPC"
    PRINT AT screenpos(GRID_ACT_COL1, GRID_ACTION_ROW) COLOR COL_NORMAL,"DEL"
    PRINT AT screenpos(GRID_ACT_COL2, GRID_ACTION_ROW) COLOR COL_NORMAL," OK"
    PRINT AT screenpos(GRID_ACT_COL3, GRID_ACTION_ROW) COLOR COL_NORMAL,"ESC"
END

' grid_draw_cursor: un-highlight the previous cell (g_px/g_py), highlight
' the current one (g_x/g_y). g_px=255 on the very first call skips the
' un-highlight (nothing has been drawn as "selected" yet).
'
' On the charset the highlight is real inverse video, done with MOB 0 rather
' than a colour change. A sprite with SPR_BEHIND set is drawn everywhere it is
' visible EXCEPT where the card underneath has a foreground pixel
' (jzintv src/stic/stic.c:1710 -- mb_msk = vs_msk & ~(pr_msk & bt_msk)). So a
' solid yellow block parked behind the cell fills it, and the glyph is punched
' out of it in whatever the card's own foreground colour is -- which is why the
' selected cell is recoloured to CS_DARKGREEN. Yellow rather than white: white
' leaves nothing to see if the block is even slightly misaligned, because the
' neighbouring cells' glyphs are white too, and it matches "yellow = selected"
' everywhere else in CONFIG. The action row has no MOB and recolours to yellow.
grid_draw_cursor: PROCEDURE
    ' grid_entry's loop calls this every frame, so the common case is "nothing
    ' moved" -- and the un-highlight/re-highlight pair would then write the SAME
    ' cell twice, straight into the live BACKTAB. Bail before touching it, and
    ' before re-issuing SPRITE: the MOB registers are shadowed in RAM and
    ' blitted by the ISR each frame, so one write persists. The g_px = 255
    ' first-call sentinel still works -- 255 can never equal g_x.
    IF g_x = g_px AND g_y = g_py THEN RETURN

    IF g_px <> 255 THEN
        IF g_py = GRID_ROWS THEN
            ga_idx = g_px : GOSUB grid_action_col
            s_row = GRID_ACTION_ROW : s_max = 3 : s_col_color = COL_NORMAL
            GOSUB scr_recolor
        ELSE
            g_ch = 32 + g_py * GRID_COLS + g_px
            IF g_ch > 126 THEN g_ch = 32
            #BACKTAB((GRID_ROW0 + g_py) * SCREEN_COLS + GRID_COL0 + g_px) = (g_ch - 32) * 8 + COL_NORMAL
        END IF
    END IF

    IF g_y = GRID_ROWS THEN
        SPRITE 0, 0, 0, 0
        ga_idx = g_x : GOSUB grid_action_col
        s_row = GRID_ACTION_ROW : s_max = 3 : s_col_color = COL_HILIGHT
        GOSUB scr_recolor
    ELSE
        g_ch = 32 + g_y * GRID_COLS + g_x
        IF g_ch > 126 THEN g_ch = 32
        #BACKTAB((GRID_ROW0 + g_y) * SCREEN_COLS + GRID_COL0 + g_x) = (g_ch - 32) * 8 + CS_DARKGREEN
        SPRITE 0, SPR_VISIBLE + GRID_MOB_X0 + (GRID_COL0 + g_x) * 8, \
                  SPR_ZOOMY2 + GRID_MOB_Y0 + (GRID_ROW0 + g_y) * 8, \
                  SPR_BEHIND + (256 + GLYPH_BLOCK) * 8 + CS_YELLOW
    END IF

    g_px = g_x : g_py = g_y
END

' grid_action_col: given ga_idx (0-3), sets s_col to that action button's
' starting column.
grid_action_col: PROCEDURE
    IF ga_idx = 0 THEN s_col = GRID_ACT_COL0
    IF ga_idx = 1 THEN s_col = GRID_ACT_COL1
    IF ga_idx = 2 THEN s_col = GRID_ACT_COL2
    IF ga_idx = 3 THEN s_col = GRID_ACT_COL3
END

' grid_draw_value: tail-anchored 20-column window onto #ge_dst on
' GRID_VALUE_ROW, with a trailing cursor block.
grid_draw_value: PROCEDURE
    s_i = 0
    IF g_len > SCREEN_COLS - 1 THEN s_i = g_len - (SCREEN_COLS - 1)
    FOR s_col = 0 TO SCREEN_COLS - 1
        s_c = 32
        IF s_i + s_col < g_len THEN s_c = PEEK(#ge_dst + s_i + s_col) AND 255
        IF s_c < 32 OR s_c > 126 THEN s_c = 32
        #BACKTAB(GRID_VALUE_ROW * SCREEN_COLS + s_col) = (s_c - 32) * 8 + COL_VALUE
    NEXT s_col
    IF g_len - s_i < SCREEN_COLS THEN
        #BACKTAB(GRID_VALUE_ROW * SCREEN_COLS + (g_len - s_i)) = (95 - 32) * 8 + COL_CURSOR
    END IF
END

grid_append: PROCEDURE
    IF g_len >= g_max - 1 THEN RETURN
    POKE (#ge_dst + g_len), g_ch
    g_len = g_len + 1
    POKE (#ge_dst + g_len), 0
    GOSUB grid_draw_value
END

grid_backspace: PROCEDURE
    IF g_len = 0 THEN RETURN
    g_len = g_len - 1
    POKE (#ge_dst + g_len), 0
    GOSUB grid_draw_value
END
