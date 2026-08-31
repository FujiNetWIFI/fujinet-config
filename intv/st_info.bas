' st_info.bas -- ST_INFO: read-only display of GET_ADAPTERCONFIG_EXTENDED
' (SSID / local IP / firmware version), reusing the fujitest.bas idiom of
' PEEKing the reply fields straight out of FN_RX with no copy.
'
' Drawn in foreground/background mode, like st_hosts.bas -- the alternating
' yellow label rows sit between dark green ones, and that interleaving needs
' more color stack slots than exist (see fgbg.bas). The catch is that FGBG
' reaches only cards 0-63, so the SSID and version fold to uppercase on the
' way to the screen. FN_RX itself is never touched; note that unlike hostnames,
' 802.11 SSIDs really are case-sensitive, so this does lose fidelity.

    CONST INFO_OFF_SSID    = 0
    CONST INFO_OFF_FNVER   = 125
    CONST INFO_OFF_LOCALIP = 140

    DIM info_shown, info_ok, if_i

do_info: PROCEDURE
    IF info_shown = 0 THEN
        GOSUB scr_clear
        PRINT AT screenpos(0,0) COLOR COL_NORMAL,"FUJINET INFO"

        GOSUB fj_get_adapter_config_extended
        info_ok = fn_ok      ' info_paint keys the yellow label rows off this

        IF info_ok = 0 THEN
            PRINT AT screenpos(2,3) COLOR COL_ERROR,"MAILBOX ERROR"
            GOSUB info_paint
            info_shown = 1
            RETURN
        END IF

        ' Labels are black on yellow, values white on dark green.
        PRINT AT screenpos(0,2) COLOR CS_BLACK,"SSID:"
        s_row = 3 : s_col = 2 : s_max = 17 : s_col_color = COL_NORMAL
        #s_src = FN_RX + INFO_OFF_SSID : GOSUB scr_puts

        PRINT AT screenpos(0,4) COLOR CS_BLACK,"IP:"
        s_row = 5 : s_col = 2 : s_max = 17 : s_col_color = COL_NORMAL
        #s_src = FN_RX + INFO_OFF_LOCALIP : GOSUB scr_puts

        PRINT AT screenpos(0,6) COLOR CS_BLACK,"VERSION:"
        s_row = 7 : s_col = 2 : s_max = 17 : s_col_color = COL_NORMAL
        #s_src = FN_RX + INFO_OFF_FNVER : GOSUB scr_puts

        ' Footer reads white, with the "ANY BUTTON" key part picked out in blue
        ' -- same key-vs-label rule as st_hosts.bas's footer, and scr_recolor
        ' has to run before the background is stamped.
        PRINT AT screenpos(0,11) COLOR COL_NORMAL,"ANY BUTTON=BACK"
        s_row = 11 : s_col = 0 : s_max = 10 : s_col_color = COL_DIM
        GOSUB scr_recolor

        GOSUB info_paint
        info_shown = 1
    END IF

    GOSUB in_poll
    IF in_btn <> 0 THEN
        info_shown = 0
        state = ST_HOSTS
    END IF
END

' ---------------------------------------------------------------------------
' info_paint: stamp every row's background in one pass -- medium green title
' and footer bars, yellow on the three label rows, dark green everywhere else.
' Runs after all the text is drawn, and touches each row exactly once, which
' scr_fgbg_row requires (its header explains why a second pass garbles a row).
'
' On the mailbox-error path the labels were never printed, so the yellow bands
' are skipped and the body stays solid dark green rather than showing three
' empty yellow stripes.
' ---------------------------------------------------------------------------
info_paint: PROCEDURE
    s_row = 0 : #s_bg = BG_GREEN : GOSUB scr_fgbg_row
    FOR if_i = 1 TO 10
        s_row = if_i : #s_bg = BG_DARKGREEN
        IF info_ok = 1 THEN
            IF if_i = 2 OR if_i = 4 OR if_i = 6 THEN #s_bg = BG_YELLOW
        END IF
        GOSUB scr_fgbg_row
    NEXT if_i
    s_row = 11 : #s_bg = BG_GREEN : GOSUB scr_fgbg_row
END
