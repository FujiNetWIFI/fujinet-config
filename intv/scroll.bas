' scroll.bas -- bounce-scroll a long filename in place on its highlighted
' row, ported from fujinet-config's src/coco/scroll.c. Text lives in
' SC_ENTRY and is PEEKed in place -- nothing is ever held in an IntyBASIC
' string.
'
' Driven by st_file.bas's sf_choose loop: set sc_row/sc_col/sc_max/sc_color
' and GOSUB scroll_step once per frame while a row is highlighted; GOSUB
' scroll_reset (with sc_row/sc_col/sc_max/sc_color describing the row being
' LEFT) whenever the cursor moves off it. Assumes st_file.bas provides
' `sf_get_long_filename`, which re-fetches the full name (up to 128 bytes,
' bypassing the 36-byte listing truncation) into SC_ENTRY and sets sc_len.
' This is the only module using scroll.bas so far, hence the direct
' coupling rather than a generalized callback.

    CONST SCROLL_TICKS = 6      ' frames per character step (~10 chars/sec)
    CONST SCROLL_HOLD  = 30     ' frames paused at each end
    CONST IDLE_FRAMES  = 45     ' frames of no input before scrolling starts

    DIM sc_row, sc_col, sc_max, sc_color
    DIM sc_off, sc_len, sc_dir, sc_tick, sc_hold, sc_idle, sc_active

' ---------------------------------------------------------------------------
' scroll_draw: paint sc_max cells of SC_ENTRY, starting at byte sc_off,
' onto row sc_row/column sc_col, in color sc_color, space-padded.
' ---------------------------------------------------------------------------
scroll_draw: PROCEDURE
    FOR s_i = 0 TO sc_max - 1
        s_c = 32
        IF sc_off + s_i < sc_len THEN s_c = PEEK(SC_ENTRY + sc_off + s_i) AND 255
        IF s_c < 32 OR s_c > 126 THEN s_c = 32
        #BACKTAB(sc_row * SCREEN_COLS + sc_col + s_i) = (s_c - 32) * 8 + sc_color
    NEXT s_i
END

' ---------------------------------------------------------------------------
' scroll_step: call once per frame while sc_row is the highlighted row.
' After IDLE_FRAMES of no scroll_reset, fetches the full name and starts
' bouncing a sc_max-wide window back and forth across it. A no-op if the
' name turns out to fit within sc_max already.
' ---------------------------------------------------------------------------
scroll_step: PROCEDURE
    IF sc_active = 0 THEN
        sc_idle = sc_idle + 1
        IF sc_idle < IDLE_FRAMES THEN RETURN
        GOSUB sf_get_long_filename
        sc_active = 1 : sc_off = 0 : sc_dir = 0 : sc_tick = 0 : sc_hold = SCROLL_HOLD
        RETURN
    END IF
    IF sc_len <= sc_max THEN RETURN

    IF sc_hold > 0 THEN
        sc_hold = sc_hold - 1
        RETURN
    END IF
    sc_tick = sc_tick + 1
    IF sc_tick < SCROLL_TICKS THEN RETURN
    sc_tick = 0

    IF sc_dir = 0 THEN
        IF sc_off < sc_len - sc_max THEN
            sc_off = sc_off + 1
        ELSE
            sc_dir = 1 : sc_hold = SCROLL_HOLD
        END IF
    ELSE
        IF sc_off > 0 THEN
            sc_off = sc_off - 1
        ELSE
            sc_dir = 0 : sc_hold = SCROLL_HOLD
        END IF
    END IF
    GOSUB scroll_draw
END

' ---------------------------------------------------------------------------
' scroll_reset: call when the cursor is about to leave row sc_row (with
' sc_color set to whatever color that row should end up in, normally
' COL_NORMAL). If a scroll was in progress, SC_ENTRY still holds the full
' name that was fetched for it, so the truncated (offset-0) view can be
' repainted from it directly -- no mailbox round trip needed. Always resets
' the idle countdown so the module is primed to start counting for whatever
' row the caller points sc_row/sc_col/sc_max at next.
' ---------------------------------------------------------------------------
scroll_reset: PROCEDURE
    IF sc_active = 1 THEN
        sc_off = 0
        GOSUB scroll_draw
    END IF
    sc_active = 0 : sc_idle = 0 : sc_dir = 0 : sc_tick = 0 : sc_hold = 0
END
