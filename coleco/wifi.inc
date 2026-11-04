; wifi.inc -- CHECK_WIFI, CONNECT_WIFI and SET_WIFI.

WIFI_POLL       EQU     120             ; frames between status polls
WIFI_TRIES      EQU     20

; st_check_wifi: off in the adapter's config, or connected: the hosts.
; An SSID on file: join it. None: pick one.
st_check_wifi:
        IF      WIFIFAKE
        JR      cw_set
        ENDIF
        CALL    cmd_wifi_enabled
        JR      C,cw_hosts
        LD      A,(FN_REPLY)
        OR      A
        JR      Z,cw_hosts
        CALL    cmd_wifi_status
        JR      C,cw_hosts
        LD      A,(FN_REPLY)
        CP      3
        JR      Z,cw_hosts
        CALL    cmd_get_ssid
        JR      C,cw_set
        LD      A,(FN_REPLY)
        OR      A
        JR      Z,cw_set
        LD      A,STATE_CONNECT
        JR      cw_state
cw_set: LD      A,STATE_SET_WIFI
        JR      cw_state
cw_hosts:
        LD      A,STATE_HOSTS
cw_state:
        LD      (state),A
        RET

; st_connect_wifi: poll the status every two seconds, 20 times; either
; button gives up.
st_connect_wifi:
        LD      HL,str_joining
        CALL    draw_frame
        LD      HL,str_leg_giveup
        CALL    legend_line
        CALL    cmd_get_ssid
        JR      C,cn_wait
        LD      HL,FN_REPLY
        LD      DE,LIST_TOP*256+TEXT_L
        LD      B,SSID_LEN-1
        CALL    disp_strn
cn_wait:
        LD      HL,str_please_wait
        CALL    status_line
        LD      A,WIFI_TRIES
        LD      (tmp_a),A
cn_try: CALL    in_frames
        LD      (tmp_b),A
cn_tick:
        CALL    in_read
        CP      EV_BACK
        JR      Z,cn_quit
        CP      EV_FIRE
        JR      Z,cn_quit
        CALL    in_frames
        LD      HL,tmp_b
        SUB     (HL)
        CP      WIFI_POLL
        JR      C,cn_tick
        CALL    cmd_wifi_status
        JR      C,cn_next
        LD      A,(FN_REPLY)
        LD      HL,str_no_ssid
        CP      1
        JR      Z,cn_fail
        LD      HL,str_conn_failed
        CP      4
        JR      Z,cn_fail
        LD      HL,str_conn_lost
        CP      5
        JR      Z,cn_fail
        CP      3
        JR      Z,cn_joined
cn_next:
        LD      HL,tmp_a
        DEC     (HL)
        JR      NZ,cn_try
        LD      HL,str_unable
cn_fail:
        PUSH    HL
        LD      A,SND_ERROR
        CALL    snd_play
        POP     HL
        CALL    status_line
        LD      A,120
        CALL    in_wait
        JR      cn_to_set
cn_joined:
        LD      A,SND_CONNECT
        CALL    snd_play
        LD      HL,str_connected
        CALL    status_line
        LD      A,60
        CALL    in_wait
        LD      A,STATE_HOSTS
        LD      (state),A
        RET
cn_quit:
        LD      A,SND_BACK
        CALL    snd_play
cn_to_set:
        LD      A,STATE_SET_WIFI
        LD      (state),A
        RET

; draw_rssi: the signed dBm A at row D, columns 26 on.
draw_rssi:
        LD      E,26
        LD      C,' '
        OR      A
        JP      P,dr_pos
        NEG
        LD      C,'-'
dr_pos: PUSH    AF
        LD      A,C
        CALL    disp_char
        INC     E
        POP     AF
        JP      disp_u8

; wifi_page: the 8 networks from wtop, then OTHER after the last.
wifi_page:
        CALL    win_clear
        XOR     A
        LD      (nrows),A
        LD      (tmp_a),A               ; tmp_a = row on the page
wp_row: LD      A,(tmp_a)
        CP      LIST_ROWS
        JR      NC,wp_done
        LD      HL,wtop
        ADD     A,(HL)
        LD      (tmp_b),A               ; tmp_b = network
        LD      HL,nnets
        CP      (HL)
        JR      Z,wp_other
        JR      NC,wp_done
        CALL    cmd_scan_result
        JR      NC,wp_net
        LD      A,(tmp_b)               ; the rest are gone: OTHER moves up
        LD      (nnets),A
wp_other:
        LD      A,(tmp_a)
        CALL    list_row
        LD      E,TEXT_L
        LD      HL,str_other
        CALL    disp_str
        CALL    wp_count
        JR      wp_done
wp_net: LD      A,(tmp_a)
        CALL    list_row
        PUSH    DE
        LD      E,TEXT_L
        LD      HL,FN_REPLY
        LD      B,24
        CALL    disp_strn
        POP     DE
        LD      A,(FN_REPLY+SSID_LEN)
        CALL    draw_rssi
        CALL    wp_count
        LD      HL,tmp_a
        INC     (HL)
        JR      wp_row
wp_done:
        LD      A,(nrows)
        LD      B,A
        LD      A,(cur)
        CP      B
        JR      C,wp_sel
        LD      A,B
        DEC     A
        LD      (cur),A
wp_sel: JP      select_cur
wp_count:
        LD      A,(tmp_a)
        INC     A
        LD      (nrows),A
        RET

wifi_legend:
        LD      HL,str_leg_wifi
        CALL    legend_line
        LD      HL,str_select_net
        JP      status_line

wifi_draw:
        LD      HL,str_networks
        CALL    draw_frame
        LD      HL,str_scanning
        CALL    status_now
        XOR     A
        LD      (nnets),A
        LD      (wtop),A
        CALL    cmd_scan
        JR      C,wd_page
        LD      A,(FN_REPLY)
        CP      WIFI_MAX+1
        JR      C,wd_n
        LD      A,WIFI_MAX
wd_n:   LD      (nnets),A
wd_page:
        LD      A,(cur)
        CP      LIST_ROWS
        JR      C,wd_cur
        XOR     A
        LD      (cur),A
wd_cur: CALL    wifi_page
        JR      wifi_legend

wifi_redraw:
        LD      HL,str_networks
        CALL    draw_frame
        CALL    wifi_page
        JR      wifi_legend

; wifi_more: Z clear if a second page exists and is not showing.
wifi_more:
        LD      A,(wtop)
        OR      A
        JR      NZ,wm_no
        LD      A,(nnets)
        CP      LIST_ROWS               ; nnets + 1 > 8
        JR      C,wm_no
        OR      1
        RET
wm_no:  XOR     A
        RET

wifi_up:
        LD      A,(cur)
        OR      A
        JR      Z,wu_page
        CALL    cur_hide
        LD      HL,cur
        DEC     (HL)
        CALL    select_cur
        JR      wifi_moved
wu_page:
        LD      A,(wtop)
        OR      A
        RET     Z
        XOR     A
        LD      (wtop),A
        LD      A,LIST_ROWS-1
        LD      (cur),A
        CALL    wifi_page
wifi_moved:
        LD      A,SND_MOVE
        JP      snd_play

wifi_down:
        LD      A,(nrows)
        LD      B,A
        LD      A,(cur)
        INC     A
        CP      B
        JR      NC,wdn_page
        CALL    cur_hide
        LD      HL,cur
        INC     (HL)
        CALL    select_cur
        JR      wifi_moved
wdn_page:
        CALL    wifi_more
        RET     Z
        XOR     A
        LD      (cur),A
wifi_page2:
        LD      A,LIST_ROWS
        LD      (wtop),A
        CALL    wifi_page
        JR      wifi_moved

; wifi_pick: the network's SSID (or one typed for OTHER) into src_spec,
; then the password into fn_entry, then SET_SSID.
wifi_pick:
        LD      A,SND_OK
        CALL    snd_play
        LD      A,(wtop)
        LD      HL,cur
        ADD     A,(HL)
        LD      HL,nnets
        CP      (HL)
        JR      NC,wk_other
        CALL    cmd_scan_result         ; fresh, then held in src_spec
        JR      NC,wk_ssid
        LD      HL,str_escan
        JP      fail_code
wk_ssid:
        LD      HL,FN_REPLY
        LD      DE,src_spec
        LD      B,SSID_LEN-1
wk_copy:
        LD      A,(HL)
        LD      (DE),A
        OR      A
        JR      Z,wk_pass
        INC     HL
        INC     DE
        DJNZ    wk_copy
        XOR     A
        LD      (DE),A
        JR      wk_pass
wk_other:
        XOR     A
        LD      (fn_entry),A
        LD      HL,str_network
        LD      DE,str_input_net
        LD      A,SSID_LEN-1
        CALL    fn_edit
        JP      C,wifi_redraw
        LD      A,(fn_entry)
        OR      A
        JP      Z,wifi_redraw
        LD      HL,fn_entry
        LD      DE,src_spec
        CALL    str_copy
wk_pass:
        XOR     A
        LD      (fn_entry),A
        LD      HL,str_password
        LD      DE,str_input_pass
        LD      A,PASS_MAX
        CALL    fn_edit
        JP      C,wifi_redraw
        LD      HL,str_setting_ssid
        CALL    status_now
        CALL    cmd_set_ssid
        JR      NC,wk_join
        LD      HL,str_essid
        CALL    fail_code
        LD      A,120
        CALL    in_wait
        JP      wifi_redraw
wk_join:
        LD      A,STATE_CONNECT
        LD      (state),A
        RET

; st_set_wifi: the network list. * rescans; the right button goes to
; the hosts.
st_set_wifi:
        XOR     A
        LD      (cur),A
        CALL    wifi_draw
sw_loop:
        LD      A,(state)
        CP      STATE_SET_WIFI
        RET     NZ
        CALL    in_read
        CP      EV_MENU
        JR      NZ,sw_ev
        LD      A,EV_HASH
sw_ev:  CP      EV_UP
        JR      NZ,sw_dn
        CALL    wifi_up
        JR      sw_loop
sw_dn:  CP      EV_DOWN
        JR      NZ,sw_lf
        CALL    wifi_down
        JR      sw_loop
sw_lf:  CP      EV_LEFT
        JR      NZ,sw_rt
        LD      A,(wtop)
        OR      A
        JR      Z,sw_loop
        XOR     A
        LD      (wtop),A
        LD      (cur),A
        CALL    wifi_page
        CALL    wifi_moved
        JR      sw_loop
sw_rt:  CP      EV_RIGHT
        JR      NZ,sw_fire
        CALL    wifi_more
        JR      Z,sw_loop
        XOR     A
        LD      (cur),A
        CALL    wifi_page2
        JR      sw_loop
sw_fire:
        CP      EV_FIRE
        JR      NZ,sw_hash
        CALL    wifi_pick
        JR      sw_loop
sw_hash:
        CP      EV_HASH
        JR      NZ,sw_back
        LD      A,SND_OK
        CALL    snd_play
        XOR     A
        LD      (cur),A
        CALL    wifi_draw
        JR      sw_loop
sw_back:
        CP      EV_BACK
        JR      NZ,sw_loop
        LD      A,SND_BACK
        CALL    snd_play
        LD      A,STATE_HOSTS
        LD      (state),A
        RET
