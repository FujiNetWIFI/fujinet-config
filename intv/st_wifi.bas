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
    CONST WIFI_CONNECT_TRIES = 20      ' status polls, ~2s apart -> ~40s
    CONST LINK_WAIT_FRAMES = 900       ' ~15s for the ESP32 to boot+enumerate

    DIM num_nets, ws_i, ws_row, ws_color, ws_tries, wifi_status, ws_new
    DIM ws_delay, ws_abort

' ws_pause: block for ws_delay frames (no PAUSE statement in IntyBASIC).
ws_pause: PROCEDURE
    ws_i = ws_delay
    WHILE ws_i > 0
        WAIT
        ws_i = ws_i - 1
    WEND
END

' ---------------------------------------------------------------------------
' do_check_wifi: wait for the ESP32 link, then: disabled/connected -> hosts;
' stored SSID -> ST_CONNECT_WIFI; anything else -> scan. Per src/check_wifi.c.
' ---------------------------------------------------------------------------
do_check_wifi: PROCEDURE
    GOSUB scr_clear
    PRINT AT screenpos(0,0) COLOR COL_NORMAL,"FUJINET CONFIG  INTV"

    ' fallback for every exit that isn't usable WiFi
    ws_sub = WS_SCAN
    state = ST_SET_WIFI

    ' FN_LINK is a free poll -- the ESP32 may still be booting at power-on
    IF (PEEK(FN_LINK) AND 255) = 0 THEN
        PRINT AT screenpos(0,5) COLOR COL_DIM,"WAITING FOR FUJINET "
        PRINT AT screenpos(0,11) COLOR COL_DIM,"PRESS KEY TO SKIP   "
        #fn_t = 0
        WHILE ((PEEK(FN_LINK) AND 255) = 0) AND (#fn_t < LINK_WAIT_FRAMES)
            WAIT
            GOSUB in_poll
            IF (in_btn <> 0) OR (in_key <> KEYPAD_NONE) THEN
                #fn_t = LINK_WAIT_FRAMES
            ELSE
                #fn_t = #fn_t + 1
            END IF
        WEND
        IF (PEEK(FN_LINK) AND 255) = 0 THEN RETURN
        s_row = 11 : GOSUB scr_row_clear
    END IF

    s_row = 5 : GOSUB scr_row_clear
    PRINT AT screenpos(2,5) COLOR COL_DIM,"CHECKING WIFI..."

    GOSUB fj_get_wifi_enabled
    IF fn_ok THEN
        IF (PEEK(FN_RX) AND 255) = 0 THEN
            state = ST_HOSTS
            RETURN
        END IF
    END IF

    GOSUB fj_get_wifi_status
    wifi_status = 0
    IF fn_ok THEN wifi_status = PEEK(FN_RX) AND 255

    IF wifi_status = WIFI_STATUS_CONNECTED THEN
        state = ST_HOSTS
        RETURN
    END IF

    ' stored SSID -> wait for it rather than rescanning
    GOSUB fj_get_ssid
    IF fn_ok THEN
        IF (PEEK(FN_RX) AND 255) <> 0 THEN
            FOR ws_i = 0 TO 32
                POKE (SC_SSID + ws_i), PEEK(FN_RX + ws_i) AND 255
            NEXT ws_i
            state = ST_CONNECT_WIFI
        END IF
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
    ' sel_row BEFORE the draw, not after: ws_draw_list picks the highlighted row
    ' from it, and it survives from whatever screen ran last (a cancelled
    ' grid_entry re-enters here with it still on the OTHER row, and
    ' do_connect_wifi's failure path can arrive with a file-browser row in it).
    ' Latent while every cursor move redrew the whole list and hid it;
    ' load-bearing now that ws_set_sel only ever touches two rows again.
    sel_row = 0
    GOSUB ws_draw_list
    PRINT AT screenpos(0,11) COLOR COL_DIM,"DISC=MOVE  BTN=PICK "

    ws_sub = WS_SELECT
END

' ws_draw_list lives in selbar.bas (the $D000 segment) alongside ws_set_sel --
' the default segment has no room for it, and it is now called from here only.

ws_do_select: PROCEDURE
    GOSUB in_poll

    ' ws_set_sel (selbar.bas) owns sel_row and recolours only the two rows that
    ' change -- do NOT call ws_draw_list here, which re-fetches every SSID over
    ' the mailbox and leaves the list frozen half-drawn for the better part of
    ' a second.
    IF in_disc = DISC_UP AND sel_row > 0 THEN
        ws_new = sel_row - 1
        GOSUB ws_set_sel
    END IF
    IF in_disc = DISC_DOWN AND sel_row < num_nets THEN
        ws_new = sel_row + 1
        GOSUB ws_set_sel
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
    GOSUB grid_video
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
    GOSUB grid_video
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
        ws_delay = 120 : GOSUB ws_pause
    ELSE
        state = ST_CONNECT_WIFI
    END IF
END

' ---------------------------------------------------------------------------
' do_connect_wifi: poll GET_WIFISTATUS ~2s apart until connected or the try
' budget runs out (firmware only returns 3/6); any keypress skips to scan.
' ---------------------------------------------------------------------------
do_connect_wifi: PROCEDURE
    GOSUB scr_clear
    PRINT AT screenpos(0,0) COLOR COL_NORMAL,"CONNECTING..."
    s_row = 3 : s_col = 2 : s_max = 17 : s_col_color = COL_VALUE
    #s_src = SC_SSID : GOSUB scr_puts
    PRINT AT screenpos(0,11) COLOR COL_DIM,"PRESS KEY TO SKIP   "

    ws_abort = 0
    ws_tries = 0
    wifi_status = 0
    WHILE (ws_abort = 0) AND (ws_tries < WIFI_CONNECT_TRIES) AND (wifi_status <> WIFI_STATUS_CONNECTED) AND (wifi_status <> WIFI_STATUS_CONNECT_FAILED)
        GOSUB fj_get_wifi_status
        IF fn_ok THEN wifi_status = PEEK(FN_RX) AND 255
        ws_tries = ws_tries + 1
        IF wifi_status <> WIFI_STATUS_CONNECTED THEN
            FOR ws_i = 0 TO 119
                WAIT
                GOSUB in_poll
                IF (in_btn <> 0) OR (in_key <> KEYPAD_NONE) THEN
                    ws_abort = 1
                    ws_i = 119
                END IF
            NEXT ws_i
        END IF
    WEND

    IF wifi_status = WIFI_STATUS_CONNECTED THEN
        state = ST_HOSTS
    ELSE
        IF ws_abort = 0 THEN
            PRINT AT screenpos(2,6) COLOR COL_ERROR,"CONNECT FAILED"
            ws_delay = 120 : GOSUB ws_pause
        END IF
        ws_sub = WS_SCAN
        state = ST_SET_WIFI
    END IF
END
