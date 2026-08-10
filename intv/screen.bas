' screen.bas -- low-level text drawing helpers shared by every screen.
'
' Character encoding follows the standard IntyBASIC card formula used
' throughout the FujiNet Intellivision tree (fujitest.bas, 5cardstud):
'     card = ascii - 32   (clamped to a printable placeholder if out of range)
'     screen word = card*8 + color
' Nothing here ever buffers a whole string in an IntyBASIC variable -- text
' is always drawn straight from a source address (ROM DATA or scratch RAM)
' via PEEK, one character at a time, per the RAM budget rule in constants.bas.

    CONST COL_NORMAL   = CS_WHITE
    CONST COL_HILIGHT  = CS_YELLOW
    CONST COL_DIM      = CS_BLUE
    CONST COL_ERROR    = CS_RED
    CONST COL_VALUE    = CS_TAN
    CONST COL_CURSOR   = CS_GREEN

    DIM s_row, s_col, s_i, s_c, s_len, s_max, s_col_color
    DIM #s_src, #s_val

' ---------------------------------------------------------------------------
' scr_clear: blank the whole 20x12 screen.
' ---------------------------------------------------------------------------
scr_clear: PROCEDURE
    CLS
END

' ---------------------------------------------------------------------------
' scr_row_clear: blank row s_row (all 20 columns).
' ---------------------------------------------------------------------------
scr_row_clear: PROCEDURE
    FOR s_i = 0 TO SCREEN_COLS - 1
        #BACKTAB(s_row * SCREEN_COLS + s_i) = CS_BLACK
    NEXT s_i
END

' ---------------------------------------------------------------------------
' scr_puts: draw bytes from #s_src onto row s_row starting at column s_col,
' in color s_col_color, stopping at the first NUL or after s_max characters,
' then space-padding the remainder of the s_max-wide field. Sets s_len to
' the number of real (non-pad) characters drawn. Caller ensures
' s_col + s_max <= SCREEN_COLS.
' ---------------------------------------------------------------------------
scr_puts: PROCEDURE
    s_len = 0
    WHILE (s_len < s_max) AND ((PEEK(#s_src + s_len) AND 255) <> 0)
        s_len = s_len + 1
    WEND
    FOR s_i = 0 TO s_max - 1
        IF s_i < s_len THEN
            s_c = PEEK(#s_src + s_i) AND 255
        ELSE
            s_c = 32
        END IF
        IF s_c < 32 OR s_c > 126 THEN s_c = 32
        #BACKTAB(s_row * SCREEN_COLS + s_col + s_i) = (s_c - 32) * 8 + s_col_color
    NEXT s_i
END

' ---------------------------------------------------------------------------
' scr_dec: print unsigned #s_val at (s_col,s_row) as decimal, in s_col_color.
' ---------------------------------------------------------------------------
scr_dec: PROCEDURE
    PRINT AT screenpos(s_col, s_row) COLOR s_col_color, <>#s_val
END

' ---------------------------------------------------------------------------
' scr_recolor: change only the COLOR of s_max already-drawn characters on
' row s_row starting at column s_col, to s_col_color -- the card (glyph)
' underneath is left untouched. Used to re-highlight a cursor row without
' redrawing its text, which for a directory listing would mean re-fetching
' the filename from the mailbox (never cached, per the RAM budget rule).
' ---------------------------------------------------------------------------
scr_recolor: PROCEDURE
    FOR s_i = 0 TO s_max - 1
        #s_val = (#BACKTAB(s_row * SCREEN_COLS + s_col + s_i) AND $FFF8) + s_col_color
        #BACKTAB(s_row * SCREEN_COLS + s_col + s_i) = #s_val
    NEXT s_i
END
