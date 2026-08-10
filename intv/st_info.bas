' st_info.bas -- ST_INFO: read-only display of GET_ADAPTERCONFIG_EXTENDED
' (SSID / local IP / firmware version), reusing the fujitest.bas idiom of
' PEEKing the reply fields straight out of FN_RX with no copy.

    CONST INFO_OFF_SSID    = 0
    CONST INFO_OFF_FNVER   = 125
    CONST INFO_OFF_LOCALIP = 140

    DIM info_shown

do_info: PROCEDURE
    IF info_shown = 0 THEN
        GOSUB scr_clear
        PRINT AT screenpos(0,0) COLOR COL_NORMAL,"FUJINET INFO"

        GOSUB fj_get_adapter_config_extended

        IF fn_ok = 0 THEN
            PRINT AT screenpos(2,3) COLOR COL_ERROR,"MAILBOX ERROR"
            info_shown = 1
            RETURN
        END IF

        PRINT AT screenpos(0,2) COLOR COL_DIM,"SSID:"
        s_row = 3 : s_col = 2 : s_max = 17 : s_col_color = COL_NORMAL
        #s_src = FN_RX + INFO_OFF_SSID : GOSUB scr_puts

        PRINT AT screenpos(0,4) COLOR COL_DIM,"IP:"
        s_row = 5 : s_col = 2 : s_max = 17 : s_col_color = COL_NORMAL
        #s_src = FN_RX + INFO_OFF_LOCALIP : GOSUB scr_puts

        PRINT AT screenpos(0,6) COLOR COL_DIM,"VERSION:"
        s_row = 7 : s_col = 2 : s_max = 17 : s_col_color = COL_NORMAL
        #s_src = FN_RX + INFO_OFF_FNVER : GOSUB scr_puts

        PRINT AT screenpos(0,11) COLOR COL_DIM,"ANY BUTTON=BACK"
        info_shown = 1
    END IF

    GOSUB in_poll
    IF in_btn <> 0 THEN
        info_shown = 0
        state = ST_HOSTS
    END IF
END
