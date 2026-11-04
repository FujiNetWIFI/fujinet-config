; info.inc -- adapter info: GET_ADAPTERCONFIG_EXTENDED, drawn field by
; field straight out of the reply window, before anything else runs.

; Each field: its row, its label, its offset in the reply, its width.
info_fields:
        DB      LIST_TOP
        DW      str_i_ssid,ACX_SSID
        DB      21
        DB      LIST_TOP+2
        DW      str_i_host,ACX_HOSTNAME
        DB      21
        DB      LIST_TOP+4
        DW      str_i_ip,ACX_LOCALIP
        DB      15
        DB      LIST_TOP+6
        DW      str_i_gateway,ACX_GATEWAY
        DB      15
        DB      LIST_TOP+8
        DW      str_i_netmask,ACX_NETMASK
        DB      15
        DB      LIST_TOP+10
        DW      str_i_dns,ACX_DNSIP
        DB      15
        DB      LIST_TOP+12
        DW      str_i_mac,ACX_MAC
        DB      17
        DB      LIST_TOP+14
        DW      str_i_bssid,ACX_BSSID
        DB      17
        DB      0

INFO_VALUE_COL  EQU     10

; info_value: B characters from FN_REPLY+HL at row D, column 10.
info_value:
        PUSH    DE
        LD      DE,FN_REPLY
        ADD     HL,DE
        POP     DE
        LD      E,INFO_VALUE_COL
        JP      disp_strn

st_info:
        LD      HL,str_adapter_info
        CALL    draw_frame
        LD      HL,str_reading
        CALL    status_now
        CALL    cmd_adapter_config
        JR      NC,in_fields
        LD      HL,str_einfo
        CALL    fail_code
        JR      in_loop
in_fields:
        LD      HL,info_fields
in_field:
        LD      A,(HL)
        OR      A
        JR      Z,in_version
        LD      D,A
        INC     HL
        LD      E,(HL)                  ; the label
        INC     HL
        LD      C,(HL)
        INC     HL
        PUSH    HL
        LD      H,C
        LD      L,E
        LD      E,TEXT_L
        CALL    disp_str
        POP     HL
        LD      C,(HL)                  ; the offset
        INC     HL
        LD      B,(HL)
        INC     HL
        LD      A,(HL)                  ; the width
        INC     HL
        PUSH    HL
        LD      H,B
        LD      L,C
        LD      B,A
        CALL    info_value
        POP     HL
        JR      in_field
in_version:
        LD      D,MSG_LINE1
        LD      HL,str_i_version
        CALL    msg_put
        LD      D,MSG_LINE1
        LD      HL,ACX_VERSION
        LD      B,14
        CALL    info_value
        LD      HL,str_leg_back
        CALL    legend_line
in_loop:
        LD      A,(state)
        CP      STATE_INFO
        RET     NZ
        CALL    in_read
        CP      EV_FIRE
        JR      Z,in_done
        CP      EV_BACK
        JR      NZ,in_loop
in_done:
        LD      A,SND_BACK
        CALL    snd_play
        LD      A,STATE_HOSTS
        LD      (state),A
        RET
