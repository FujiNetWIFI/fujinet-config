; wifi.inc -- the status screen, the network picker and connecting.
;
; At power-on, a FujiNet already on a network shows where for three seconds
; and goes on to the host slots by itself: the common case is a cart set up
; once and never touched again. Back here later, it waits for a key.

; st_wifi -- the link, and the way into WiFi setup.
        RFIT    152
st_wifi:
        STATE
        FCALL   pr_cls
        DB      "FUJINET CONFIG", 0
        FCALL   pr_at
        DB      20H, "CHECKING WIFI", 0
        FCALL   c_wst
        BZ      sw1
        FCALL   fail
        DB      14
        DW      st_wifi
sw1:    PEEK    FN_REPLY
        XRI     3
        BZ      sw_on
        FCALL   pr_at
        DB      20H, "NO WIFI LINK ", 0
        FCALL   pr_key
        DB      60H, "5:WIFI SETUP", 0
        FCALL   pr_key
        DB      70H, "0:SKIP", 0
        FCALL   pr_key
        DB      80H, "3:ADAPTER INFO", 0
swk:    FCALL   key_wait
        FCALL   kdisp
        KEYTO   5, st_scan
        KEYTO   0, st_hosts
        KEYTO   3, sw_info
        DB      0FFH
        BR      swk
sw_on:  FJMP    sw_on1

; sw_on1 -- connected: the network and address, then where to.
        RFIT    165
sw_on1: FCALL   c_adpx
        BZ      sw2
        FCALL   fail
        DB      8
        DW      st_hosts
sw2:    FCALL   pr_at
        DB      20H, "WIFI CONNECTED", 0
        LDI     30H
        FCALL   d_at
        LDI     TOP_MARQUEE
        FCALL   d_op
        LDR     R8, AC_SSID
        LDI     0
        FCALL   d_rpl
        LDI     40H
        FCALL   d_at
        LDR     R8, AC_SIP
        LDI     16
        FCALL   d_rpl
        FCALL   pr_key
        DB      60H, "5:HOST SLOTS", 0
        FCALL   pr_key
        DB      70H, "1:WIFI SETUP", 0
        FCALL   pr_key
        DB      80H, "3:ADAPTER INFO", 0
        LDV     V_SEEN                  ; at power-on, on by itself
        BNZ     swo1
        LDI     1
        STR     RB
        LDI     3
        FCALL   key_tmo
        BR      swo2
swo1:   FCALL   key_wait
swo2:   FCALL   kdisp
        KEYTO   1, st_scan
        KEYTO   3, sw_info
        DB      0FFH
        FJMP    st_hosts                ; 5, any other key, or the time

sw_info:
        SETW    V_IRET, st_wifi
        FJMP    st_info

; st_scan -- pick a network. The list pages by NROWS; its last row is
; "<OTHER>", for a network that does not broadcast its name.
        RFIT    177
st_scan:
        STATE
        FCALL   pr_cls
        DB      "WIFI NETWORKS", 0
        FCALL   pr_at
        DB      20H, "SCANNING...", 0
        FCALL   c_scan
        BZ      sc1
        FCALL   fail
        DB      11
        DW      st_wifi
sc1:    PEEK    FN_REPLY
        XRI     0FFH
        BNZ     sc1a
        LDI     1                       ; 255 found: keep 254, so count + 1 fits
sc1a:   XRI     0FFH
        STV     V_SCANN
        SETV    V_SCANB, 0
        SETV    V_CUR, 0
sc_pg:  FCALL   pr_cls
        DB      "WIFI NETWORKS", 0
        FCALL   pr_leg
        DB      "5:PICK 0:BACK", 0
        LDV     V_SCANB                 ; rows = min(NROWS, count + 1 - base)
        STR     R2
        LDV     V_SCANN
        ADI     1
        SM
        STR     R2
        SMI     NROWS+1
        BNF     sc2
        LDI     NROWS
        STR     R2
sc2:    LDN     R2
        STV     V_NROW
        LDI     0
sc3:    STV     V_I
        FCALL   sc_row
        LDV     V_I
        ADI     1
        STR     R2
        LDV     V_NROW
        XOR
        BZ      sc4
        LDN     R2
        BR      sc3
sc4:    FJMP    sc_in

        RFIT    28
sc_in:  FCALL   key_wait
        FCALL   kdisp
        KEYTO   2, sc_up
        KEYTO   8, sc_dn
        KEYTO   4, sc_pu
        KEYTO   6, sc_pd
        KEYTO   5, sc_sel
        KEYTO   0, st_wifi
        DB      0FFH
        FJMP    sc_in

; Moving repaints the old row and the new: each is a GET_SCAN_RESULT, since
; the reply window holds one network at a time.
        RFIT    99
sc_up:  FCALL   lst_up
        BDF     sc_mv
        LDI     NROWS-1
        BR      sc_pu2
sc_dn:  FCALL   lst_dn
        BDF     sc_mv
        BR      sc_pd
sc_mv:  LDV     V_OLD
        FCALL   sc_row
        LDV     V_CUR
        FCALL   sc_row
        FJMP    sc_in
sc_bad: LDI     8
        PLO     RD
        FJMP    sc_in
sc_pu:  LDI     0
sc_pu2: STV     V_ARG                   ; the cursor on the page above
        LDV     V_SCANB
        BZ      sc_bad
        SMI     NROWS
        STR     RB
        LDV     V_ARG
        STV     V_CUR
        FJMP    sc_pg
sc_pd:  LDV     V_SCANB                 ; a next page if base + NROWS <= count
        ADI     NROWS
        STR     R2
        LDV     V_SCANN
        SM
        BNF     sc_bad
        LDN     R2
        STV     V_SCANB
        SETV    V_CUR, 0
        FJMP    sc_pg

; sc_row -- paint row D of this page.
        RFIT    100
sc_row: STV     V_ROW
        ADI     1
        SHL
        SHL
        SHL
        SHL
        FCALL   d_at
        FCALL   is_cur
        BNF     sr1
        LDI     TOP_MARQUEE
        FCALL   d_op
        LDI     TOP_INV_ON
        FCALL   d_op
sr1:    LDI     ' '
        FCALL   d_putc
        FCALL   sc_idx
        STR     R2
        LDV     V_SCANN
        XOR
        BZ      sr3                     ; the <OTHER> row
        LDN     R2
        FCALL   c_sres
        BNZ     sr4
        LDR     R8, 0
        FCALL   is_cur
        LDI     15
        BNF     sr2
        LDI     0
sr2:    FCALL   d_rplpad
        BR      sr5
sr3:    FCALL   pr_in
        DB      "<OTHER>", 0
        BR      sr6
sr4:    FCALL   pr_in
        DB      "?", 0
sr6:    LDI     TOP_CLREOL
        FCALL   d_op
sr5:    LDI     TOP_INV_OFF
        FJMP    d_op

; sc_idx -- D = V_SCANB + V_ROW, the network's index (V_SCANN: <OTHER>).
        RFIT    11
sc_idx: LDV     V_SCANB
        STR     R2
        LDV     V_ROW
        ADD
        FRET

; sc_sel -- a network: its password next; <OTHER>: its name first.
        RFIT    86
sc_sel: LDV     V_CUR
        STV     V_ROW
        FCALL   sc_idx
        STR     R2
        LDV     V_SCANN
        XOR
        BZ      ss2
        LDN     R2
        FCALL   c_sres
        BZ      ss1
        FCALL   fail
        DB      12
        DW      st_wifi
ss1:    LDR     R8, FN_REPLY
        LDR     R9, V_SSID
        LDI     32
        FCALL   s_copy
        SETV    V_EDWHAT, ED_PASS
        LDI     63
        BR      ss4
ss2:    SETV    V_EDWHAT, ED_SSID
        LDI     32
ss4:    STV     V_EDMAX
        LDR     R8, V_EDIT
        LDI     0
        STR     R8
        FJMP    st_edit

; st_conn -- send the credentials, then poll the link for up to 20 s.
        RFIT    124
st_conn:
        STATE
        FCALL   pr_cls
        DB      "CONNECTING TO", 0
        LDI     20H
        FCALL   d_at
        LDR     R8, V_SSID
        FCALL   d_puts
        FCALL   c_ssid
        BZ      cn1
        FCALL   fail
        DB      13
        DW      st_wifi
cn1:    SETV    V_I, 20
cn2:    LDI     1
        FCALL   wait_s
        FCALL   c_wst
        BNZ     cn3
        PEEK    FN_REPLY
        XRI     3
        BNZ     cn3
        FJMP    st_hosts
cn3:    LDV     V_I
        SMI     1
        STR     RB
        BNZ     cn2
        FCALL   pr_at
        DB      50H, "NOT CONNECTED", 0
        FCALL   pr_leg
        DB      "ANY KEY", 0
        FCALL   key_wait
        FJMP    st_wifi
