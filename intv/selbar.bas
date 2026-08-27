' selbar.bas -- move the selection bar without redrawing the list, for the two
' list screens that are not the file browser: ST_HOSTS (st_hosts.bas, FGBG mode)
' and the WiFi scan list (st_wifi.bas, color stack mode). The file browser has
' its own equivalent in csbar.bas, which was already built this way.
'
' Both screens used to repaint their WHOLE list on every cursor move. #BACKTAB
' is the LIVE STIC display list at $0200 -- IntyBASIC keeps no shadow copy and
' nothing is double-buffered, so a repaint is scanned out as it happens. An NTSC
' frame with the display enabled leaves the CPU ~13518 cycles (jzintv
' doc/programming/interrupts.txt:127), and one IntyBASIC BACKTAB
' read-modify-write costs ~264 (every #BACKTAB(expr) re-runs a MULT-by-20 index
' computation, once per side of the assignment -- see the generated config.asm).
' That is a budget of about 50 cells per frame. hosts_draw_list is ~320 cells,
' so a single cursor move took the better part of seven frames and the list was
' visibly seen repainting in pieces. Moving a highlight only ever changes two
' rows, so that is all these procedures touch.
'
' Placed in the $D000 segment (INCLUDEd after fgbg.bas in config.bas) for the
' same reason st_lobby.bas, st_copy.bas and fgbg.bas are: the default
' $5000-$6FFF segment is essentially full, and overflowing it is silent (see
' config.bas). ws_draw_list moved here from st_wifi.bas as part of the same
' trade -- it now runs once per scan and nothing in an input loop calls it.

' ---------------------------------------------------------------------------
' hosts_recolor_row: recolour all 20 cells of row hd_row to foreground hd_color
' over background #s_bg, KEEPING each cell's glyph. The FGBG counterpart of
' screen.bas's scr_recolor, which masks AND $FFF8 and so cannot move the
' background bits.
'
' Unlike fgbg.bas's scr_fgbg_row this IS idempotent, and has to be -- it runs
' once per cursor move for as long as the screen is up. After scr_fgbg_row has
' painted a row once, every cell on it is card*8 + fg + bg with the card clamped
' to 0-63 (fgbg.bas:46), so:
'     bits 0-2         foreground
'     bits 3-8         card                <- $01F8
'     bit 11           FGBG GRAM select    <- always 0 here
'     bits 9,10,12,13  background          (the reversed order constants.bas
'                                           documents, not the manual's)
' $01F8 is therefore exactly the card field: masking with it drops the old
' foreground AND the old background and pins the cell to GROM, leaving the glyph
' and nothing else. Ground truth for the split is jzintv src/stic/stic.c:1473
' (gr_idx = card AND $9F8) and :1481 (bg_clr from word bits 12,13,10,9).
'
' *** Only valid on a row scr_fgbg_row has already painted. *** On a raw
' PRINT/scr_puts row the background bits are still part of the card number and
' the mask would truncate the glyph. Use $09F8 rather than $01F8 if a GRAM glyph
' is ever put on a host row -- today none is, and $01F8 keeps the GROM-only rule
' self-healing.
' ---------------------------------------------------------------------------
hosts_recolor_row: PROCEDURE
    FOR hd_i = 0 TO SCREEN_COLS - 1
        #BACKTAB(hd_row * SCREEN_COLS + hd_i) = (#BACKTAB(hd_row * SCREEN_COLS + hd_i) AND $01F8) + hd_color + #s_bg
    NEXT hd_i
END

' ---------------------------------------------------------------------------
' hosts_set_sel: move the host list's selection bar to slot hd_new (0-7).
' Recolours the row being left back to white-on-dark-green and the new one to
' black-on-tan: 40 cell operations instead of hosts_draw_list's ~320, with no
' PRINT, no scr_puts and no second scr_fgbg_row pass.
'
' hosts_draw_list is still what PAINTS the list (and still calls scr_fgbg_row
' exactly once per row, as fgbg.bas requires); this only ever runs afterwards.
' The equality guard also makes keypad 1-8 on the already-selected slot a true
' no-op instead of a full repaint. sel_row is owned here, not by the callers.
' ---------------------------------------------------------------------------
hosts_set_sel: PROCEDURE
    IF hd_new = sel_row THEN RETURN
    hd_row = 1 + sel_row
    hd_color = COL_NORMAL : #s_bg = BG_DARKGREEN
    GOSUB hosts_recolor_row
    sel_row = hd_new
    hd_row = 1 + sel_row
    hd_color = CS_BLACK : #s_bg = BG_TAN
    GOSUB hosts_recolor_row
END

' ---------------------------------------------------------------------------
' ws_draw_list: rows 1..num_nets show scanned SSIDs (re-fetched one at a time --
' never cached), row (num_nets+1) is the fixed "OTHER" entry.
'
' Moved here from st_wifi.bas, and now called from exactly one place:
' ws_do_scan. Every SSID it draws costs a BLOCKING fj_get_scan_result, and
' fujinet.bas's fn_transact spins on WAIT -- at least a frame per poll. Nine of
' those meant the list sat frozen half-drawn for the better part of a second on
' every single press of the disc. Cursor movement goes through ws_set_sel now,
' which touches two rows and does no mailbox I/O at all.
'
' Callers must set sel_row BEFORE calling: this decides the highlighted row from
' it, and it is the only pass that ever draws the unselected rows' text.
' ---------------------------------------------------------------------------
ws_draw_list: PROCEDURE
    FOR ws_i = 0 TO num_nets - 1
        ws_row = 1 + ws_i
        ws_color = COL_NORMAL
        IF ws_i = sel_row THEN ws_color = COL_HILIGHT

        fc_i = ws_i : GOSUB fj_get_scan_result
        IF fn_ok THEN
            s_row = ws_row : s_col = 1 : s_max = 18 : s_col_color = ws_color
            #s_src = FN_RX : GOSUB scr_puts
        END IF
    NEXT ws_i

    ws_row = 1 + num_nets
    ws_color = COL_DIM
    IF sel_row = num_nets THEN ws_color = COL_HILIGHT
    PRINT AT screenpos(1, ws_row) COLOR ws_color,"OTHER (ENTER SSID)"

    FOR ws_i = ws_row + 1 TO 10
        s_row = ws_i : GOSUB scr_row_clear
    NEXT ws_i
END

' ---------------------------------------------------------------------------
' ws_recolor_row / ws_set_sel: move the scan list's highlight to row ws_new
' (0..num_nets, the last of which is the OTHER entry). That screen runs in the
' color stack mode config.bas programs as MODE 0,0,0,0,0, so there are no
' advance bits anywhere on it and the highlight is purely a foreground colour --
' screen.bas's scr_recolor is enough, and its AND $FFF8 mask is complete here.
'
' Both kinds of row occupy exactly columns 1-18: the SSID rows are drawn by
' scr_puts with s_col = 1 / s_max = 18, and "OTHER (ENTER SSID)" is an
' 18-character literal PRINTed AT column 1, so one field geometry covers both.
' The UNSELECTED colour is not uniform though -- OTHER is COL_DIM and a real
' network COL_NORMAL -- hence the test against num_nets.
' ---------------------------------------------------------------------------
ws_recolor_row: PROCEDURE
    s_row = ws_row : s_col = 1 : s_max = 18 : s_col_color = ws_color
    GOSUB scr_recolor
END

ws_set_sel: PROCEDURE
    IF ws_new = sel_row THEN RETURN
    ws_row = 1 + sel_row
    ws_color = COL_NORMAL
    IF sel_row = num_nets THEN ws_color = COL_DIM
    GOSUB ws_recolor_row
    sel_row = ws_new
    ws_row = 1 + sel_row
    ws_color = COL_HILIGHT
    GOSUB ws_recolor_row
END
