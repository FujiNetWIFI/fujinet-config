; info.inc -- st_info: what the adapter says about itself, all from one
; GET_ADAPTERCONFIG_EXTENDED. Any key returns to V_IRET.

        RFIT    195
st_info:
        STATE
        FCALL   pr_cls
        DB      "ADAPTER INFO", 0
        FCALL   c_adpx
        BZ      si1
        FCALL   fail
        DB      8
        DW      st_hosts
si1:    FCALL   pr_at
        DB      10H, "NETWORK", 0
        LDI     20H
        FCALL   d_at
        LDI     TOP_MARQUEE
        FCALL   d_op
        LDR     R8, AC_SSID
        LDI     0
        FCALL   d_rpl
        FCALL   pr_at
        DB      30H, "IP ADDRESS", 0
        LDI     40H
        FCALL   d_at
        LDR     R8, AC_SIP
        LDI     16
        FCALL   d_rpl
        FCALL   pr_at
        DB      50H, "FIRMWARE", 0
        LDI     60H
        FCALL   d_at
        LDR     R8, AC_VER
        LDI     15
        FCALL   d_rpl
        FCALL   pr_at
        DB      70H, "MAC ADDRESS", 0
        LDI     80H
        FCALL   d_at
        LDR     R8, FN_REPLY+AC_MAC
        SETV    V_I, 6
si2:    LDA     R8
        FCALL   d_hex
        LDV     V_I
        SMI     1
        STR     RB
        BNZ     si2
        FCALL   pr_leg
        DB      "ANY KEY: BACK", 0
        FCALL   key_wait
        PTV     V_IRET
        FCALL   go_ind
