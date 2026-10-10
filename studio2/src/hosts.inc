; hosts.inc -- st_hosts: the eight host slots, one per list row.
;
; READ_HOST_SLOTS puts all eight names (8 x 32) in the reply window at once,
; so the rows print straight from it and moving the bar costs no round trip.
; With V_MODE = 1 the same screen picks a copy's destination (copy.inc).

        RFIT    143
st_hosts:
        STATE
        SETV    V_MODE, 0
hs_go:  STATE
        LDV     V_HOST
        STV     V_CUR
        LDV     V_MODE
        BNZ     hsg1
        FCALL   pr_cls
        DB      "HOST SLOTS", 0
        FCALL   pr_leg
        DB      "5:OPEN 7:RENAME", 0
        BR      hsg2
hsg1:   FCALL   pr_cls
        DB      "COPY TO HOST", 0
        FCALL   pr_leg
        DB      "5:HERE 0:CANCEL", 0
hsg2:   FCALL   c_rhosts
        BZ      hsg3
        FCALL   fail
        DB      1
        DW      st_wifi
hsg3:   SETV    V_NROW, NROWS
        LDI     0
hsg4:   STV     V_I
        FCALL   hs_row
        LDV     V_I
        ADI     1
        XRI     NROWS
        BZ      hsg5
        XRI     NROWS
        BR      hsg4
hsg5:   FJMP    hs_in

        RFIT    31
hs_in:  FCALL   key_wait
        FCALL   kdisp
        KEYTO   2, hs_up
        KEYTO   8, hs_dn
        KEYTO   5, hs_sel
        KEYTO   7, hs_ren
        KEYTO   1, hs_wifi
        KEYTO   3, hs_info
        KEYTO   0, hs_back
        DB      0FFH
        FJMP    hs_in

        RFIT    125
hs_up:  FCALL   lst_up
        BR      hs_mv
hs_dn:  FCALL   lst_dn
hs_mv:  BNF     hs_mv1
        LDV     V_OLD
        FCALL   hs_row
        LDV     V_CUR
        FCALL   hs_row
hs_mv1: FJMP    hs_in
hs_bad: LDI     8
        PLO     RD
        FJMP    hs_in
hs_wifi:
        LDV     V_MODE
        BNZ     hs_bad
        FJMP    st_scan
hs_info:
        LDV     V_MODE
        BNZ     hs_bad
        SETW    V_IRET, st_hosts
        FJMP    st_info
hs_back:
        LDV     V_MODE
        BZ      hsb1
        FJMP    cp_end
hsb1:   FJMP    st_wifi

; hs_sel -- open the slot (MOUNT_HOST, then browse its root), or copy to it.
hs_sel: FCALL   hs_name
        LDN     R8
        BZ      hs_bad                  ; an empty slot
        LDV     V_MODE
        BZ      hss1
        FJMP    cp_go
hss1:   LDV     V_CUR
        STV     V_HOST
        FCALL   c_mhost
        BZ      hss2
        FCALL   fail
        DB      2
        DW      st_hosts
hss2:   LDR     R8, V_PATH
        LDI     '/'
        STR     R8
        INC     R8
        LDI     0
        STR     R8
        FJMP    fl_top

; hs_ren -- rename the slot through the keyboard, its name to start from.
        RFIT    48
hs_ren: LDV     V_MODE
        BZ      hsr1
        FJMP    hs_bad
hsr1:   LDV     V_CUR
        STV     V_HSEL
        FCALL   hs_name
        LDR     R9, V_EDIT
        LDI     31
        FCALL   s_copy
        SETV    V_EDWHAT, ED_HOST
        SETV    V_EDMAX, 31
        FJMP    st_edit

; hs_name -- R8 = slot V_CUR's name in the reply window.
        RFIT    14
hs_name:
        LDV     V_CUR
        SHL
        SHL
        SHL
        SHL
        SHL
        PLO     R8
        LDI     HI(FN_REPLY)
        PHI     R8
        FRET

; hs_row -- paint slot D: its number, then its name (a marquee on the bar).
        RFIT    93
hs_row: STV     V_ROW
        ADI     1
        SHL
        SHL
        SHL
        SHL
        FCALL   d_at
        FCALL   is_cur
        BNF     hr1
        LDI     TOP_MARQUEE
        FCALL   d_op
        LDI     TOP_INV_ON
        FCALL   d_op
hr1:    LDV     V_ROW
        ADI     '1'
        FCALL   d_putc
        LDV     V_ROW
        SHL
        SHL
        SHL
        SHL
        SHL
        PLO     R8
        LDI     HI(FN_REPLY)
        PHI     R8
        LDN     R8
        BZ      hr3                     ; empty
        LDI     0
        PHI     R8                      ; the reply offset
        FCALL   is_cur
        LDI     15
        BNF     hr2
        LDI     0
hr2:    FCALL   d_rplpad
        BR      hr4
hr3:    FCALL   pr_in
        DB      "(EMPTY)", 0
        LDI     TOP_CLREOL
        FCALL   d_op
hr4:    LDI     TOP_INV_OFF
        FJMP    d_op

; hs_save -- the keyboard's name into slot V_HSEL: read the table afresh,
; write it back with that one slot changed, and show it.
        RFIT    38
hs_save:
        STATE
        FCALL   c_rhosts
        BZ      hv1
        FCALL   fail
        DB      1
        DW      st_hosts
hv1:    FCALL   c_whosts
        BZ      hv2
        FCALL   fail
        DB      10
        DW      st_hosts
hv2:    LDV     V_HSEL
        STV     V_HOST
        FJMP    st_hosts
