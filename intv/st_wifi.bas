' st_wifi.bas -- ST_CHECK_WIFI / ST_CONNECT_WIFI / ST_SET_WIFI: scan for
' networks (or enter a custom SSID), enter a passphrase via the character
' grid, SET_SSID, then confirm the connection came up.
'
' Scan results are never cached (per the RAM budget rule) -- only the
' count. Picking a listed network re-fetches that one SSIDInfo again.
' KNOWN LIMITATION (v1): only the first 9 scan results are shown; there is
' no paging for networks the way there is for the file browser. Revisit if
' that proves too restrictive in practice.

    CONST WIFI_STATUS_CONNECTED       = 3
    CONST WIFI_STATUS_CONNECT_FAILED  = 4
    CONST WIFI_STATUS_CONNECTION_LOST = 5
    CONST WIFI_SCAN_SHOWN = 9          ' rows 1..9; row 10 is "OTHER"
    CONST WIFI_CONNECT_TRIES = 30      ' ~30 status polls, a few seconds

    DIM num_nets, ws_i, ws_row, ws_color, ws_tries, wifi_status
    DIM ws_delay

' ws_pause: block for ws_delay frames (no PAUSE statement in IntyBASIC).
ws_pause: PROCEDURE
    FOR ws_delay = 0 TO 119
        WAIT
    NEXT ws_delay
END

' ---------------------------------------------------------------------------
' do_check_wifi: query current status. Already connected -> straight to the
' host slots screen; otherwise go set one up.
' ---------------------------------------------------------------------------
do_check_wifi: PROCEDURE
    GOSUB scr_clear
    PRINT AT screenpos(0,0) COLOR COL_NORMAL,"FUJINET CONFIG  INTV"
    PRINT AT screenpos(2,5) COLOR COL_DIM,"CHECKING WIFI..."

    GOSUB fj_get_wifi_status
    wifi_status = 0
    IF fn_ok THEN wifi_status = PEEK(FN_RX) AND 255

    IF wifi_status = WIFI_STATUS_CONNECTED THEN
        state = ST_HOSTS
    ELSE
        ws_sub = WS_SCAN
        state = ST_SET_WIFI
    END IF
END

' ---------------------------------------------------------------------------
' do_set_wifi: dispatch on ws_sub.
' ---------------------------------------------------------------------------
do_set_wifi: PROCEDURE
    IF ws_sub = WS_SCAN THEN GOSUB ws_do_scan
    IF ws_sub = WS_SELECT THEN GOSUB ws_do_select
    IF ws_sub = WS_CUSTOM THEN GOSUB ws_do_custom
    IF ws_sub = WS_PASSWORD THEN GOSUB ws_do_password
    IF ws_sub = WS_DONE THEN GOSUB ws_do_done
END

ws_do_scan: PROCEDURE
    GOSUB scr_clear
    PRINT AT screenpos(0,0) COLOR COL_NORMAL,"SELECT NETWORK"
    PRINT AT screenpos(2,5) COLOR COL_DIM,"SCANNING..."

    GOSUB fj_scan_networks
    num_nets = 0
    IF fn_ok THEN num_nets = PEEK(FN_RX) AND 255
    IF num_nets > WIFI_SCAN_SHOWN THEN num_nets = WIFI_SCAN_SHOWN

    GOSUB scr_clear
    PRINT AT screenpos(0,0) COLOR COL_NORMAL,"SELECT NETWORK"
    GOSUB ws_draw_list
    PRINT AT screenpos(0,11) COLOR COL_DIM,"DISC=MOVE  BTN=PICK "

    sel_row = 0
    ws_sub = WS_SELECT
END

' ---------------------------------------------------------------------------
' ws_draw_list: rows 1..num_nets show scanned SSIDs (re-fetched one at a
' time -- never cached), row (num_nets+1) is the fixed "OTHER" entry.
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

ws_do_select: PROCEDURE
    GOSUB in_poll

    IF in_disc = DISC_UP AND sel_row > 0 THEN
        sel_row = sel_row - 1
        GOSUB ws_draw_list
    END IF
    IF in_disc = DISC_DOWN AND sel_row < num_nets THEN
        sel_row = sel_row + 1
        GOSUB ws_draw_list
    END IF

    IF in_btn <> 0 THEN
        IF sel_row = num_nets THEN
            ws_sub = WS_CUSTOM
        ELSE
            fc_i = sel_row : GOSUB fj_get_scan_result
            IF fn_ok THEN
                FOR ws_i = 0 TO 32
                    POKE (SC_SSID + ws_i), PEEK(FN_RX + ws_i) AND 255
                NEXT ws_i
                POKE (SC_PASS), 0   ' fresh password entry
                ws_sub = WS_PASSWORD
            END IF
        END IF
    END IF
END

ws_do_custom: PROCEDURE
    GOSUB scr_clear
    PRINT AT screenpos(0,0) COLOR COL_NORMAL,"ENTER NETWORK NAME"
    POKE (SC_SSID), 0
    #ge_dst = SC_SSID : g_max = 33
    GOSUB grid_entry

    IF fn_ok = 1 THEN
        POKE (SC_PASS), 0
        ws_sub = WS_PASSWORD
    ELSE
        ws_sub = WS_SCAN   ' cancelled -- rescan rather than track stale results
    END IF
END

ws_do_password: PROCEDURE
    GOSUB scr_clear
    PRINT AT screenpos(0,0) COLOR COL_NORMAL,"ENTER PASSWORD"
    #ge_dst = SC_PASS : g_max = 64
    GOSUB grid_entry

    IF fn_ok = 1 THEN
        ws_sub = WS_DONE
    ELSE
        ws_sub = WS_SCAN
    END IF
END

ws_do_done: PROCEDURE
    GOSUB scr_clear
    PRINT AT screenpos(0,0) COLOR COL_NORMAL,"SAVING..."
    GOSUB fj_set_ssid

    ws_sub = WS_SCAN   ' reset for next time this screen is entered
    IF fn_ok = 0 THEN
        PRINT AT screenpos(2,3) COLOR COL_ERROR,"SAVE FAILED"
        GOSUB ws_pause
    ELSE
        state = ST_CONNECT_WIFI
    END IF
END

' ---------------------------------------------------------------------------
' do_connect_wifi: poll GET_WIFISTATUS until connected, a definite failure,
' or WIFI_CONNECT_TRIES polls elapse. Each poll is its own mailbox
' transaction (so this already paces itself at roughly one per WAIT-bounded
' round trip; no extra delay needed between tries).
' ---------------------------------------------------------------------------
do_connect_wifi: PROCEDURE
    GOSUB scr_clear
    PRINT AT screenpos(0,0) COLOR COL_NORMAL,"CONNECTING..."
    s_row = 3 : s_col = 2 : s_max = 17 : s_col_color = COL_VALUE
    #s_src = SC_SSID : GOSUB scr_puts

    ws_tries = 0
    wifi_status = 0
    WHILE (ws_tries < WIFI_CONNECT_TRIES) AND (wifi_status <> WIFI_STATUS_CONNECTED) AND (wifi_status <> WIFI_STATUS_CONNECT_FAILED)
        GOSUB fj_get_wifi_status
        IF fn_ok THEN wifi_status = PEEK(FN_RX) AND 255
        ws_tries = ws_tries + 1
    WEND

    IF wifi_status = WIFI_STATUS_CONNECTED THEN
        state = ST_HOSTS
    ELSE
        PRINT AT screenpos(2,6) COLOR COL_ERROR,"CONNECT FAILED"
        GOSUB ws_pause
        ws_sub = WS_SCAN
        state = ST_SET_WIFI
    END IF
END
