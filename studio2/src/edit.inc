; edit.inc -- st_edit: the on-screen keyboard.
;
; Rows 3-8 hold all of $20-$7F, sixteen to a row, so the cell number is the
; character less $20 and its screen position the character less $10: no
; table, and no CASE key (the font's lowercase is its own, shorter glyph).
; V_EDIT holds the text; V_EDWHAT says where it goes when 9 says done.

        RFIT    132
st_edit:
        STATE
        LDV     V_EDWHAT
        BNZ     ste1
        FCALL   pr_cls
        DB      "NETWORK NAME", 0
        BR      ste3
ste1:   XRI     ED_PASS
        BNZ     ste2
        FCALL   pr_cls
        DB      "PASSWORD", 0
        BR      ste3
ste2:   FCALL   pr_cls
        DB      "HOST NAME", 0
ste3:   FCALL   pr_key
        DB      20H, "5:ADD 0:DEL 9:OK", 0
        FCALL   pr_leg
        DB      "1:CANCEL", 0
        LDR     R8, V_EDIT
ste4:   LDA     R8
        BNZ     ste4
        DEC     R8
        GLO     R8
        SMI     LO(V_EDIT)
        STV     V_EDLEN
        SETV    V_OSK, 'a'-20H
        FCALL   ed_grid
        LDV     V_OSK
        FCALL   ed_sel
        FCALL   ed_text
        FJMP    ed_in

        RFIT    34
ed_in:  FCALL   key_wait
        FCALL   kdisp
        KEYTO   2, ed_up
        KEYTO   8, ed_dn
        KEYTO   4, ed_lt
        KEYTO   6, ed_rt
        KEYTO   5, ed_add
        KEYTO   0, ed_del
        KEYTO   9, ed_ok
        KEYTO   1, ed_esc
        DB      0FFH
        FJMP    ed_in

; Moves wrap at the edges. The old cell is repainted plain first.
        RFIT    117
ed_up:  LDV     V_OSK
        FCALL   ed_cell
        LDV     V_OSK
        SMI     16
        BDF     ed_mv
        ADI     96
        BR      ed_mv
ed_dn:  LDV     V_OSK
        FCALL   ed_cell
        LDV     V_OSK
        ADI     16
        STR     R2
        SMI     96
        BDF     ed_mv
        LDN     R2
        BR      ed_mv
ed_lt:  LDV     V_OSK
        FCALL   ed_cell
        LDV     V_OSK
        ANI     0FH
        BZ      edl1
        LDV     V_OSK
        SMI     1
        BR      ed_mv
edl1:   LDV     V_OSK
        ADI     15
        BR      ed_mv
ed_rt:  LDV     V_OSK
        FCALL   ed_cell
        LDV     V_OSK
        ANI     0FH
        XRI     0FH
        BZ      edr1
        LDV     V_OSK
        ADI     1
        BR      ed_mv
edr1:   LDV     V_OSK
        SMI     15
ed_mv:  STV     V_OSK
        FCALL   ed_sel
        FCALL   ed_text
        FJMP    ed_in

; ed_add -- type the selected character, if there is room ($7F types nothing).
        RFIT    77
ed_add: LDV     V_OSK
        XRI     7FH-20H
        BZ      ed_bad
        LDV     V_EDLEN
        STR     R2
        LDV     V_EDMAX
        SM
        BZ      ed_bad                  ; full
        LDN     R2
        ADI     LO(V_EDIT)
        PLO     R8
        LDI     HI(V_EDIT)
        PHI     R8
        LDV     V_OSK
        ADI     20H
        STR     R8
        INC     R8
        LDI     0
        STR     R8
        LDV     V_EDLEN
        ADI     1
        STR     RB
        BR      ed_shw
ed_del: LDV     V_EDLEN
        BZ      ed_bad
        SMI     1
        STR     RB
        ADI     LO(V_EDIT)
        PLO     R8
        LDI     HI(V_EDIT)
        PHI     R8
        LDI     0
        STR     R8
ed_shw: FCALL   ed_text
        FJMP    ed_in
ed_bad: LDI     8
        PLO     RD
        FJMP    ed_in

; ed_ok -- the text goes where V_EDWHAT says.
        RFIT    71
ed_ok:  LDV     V_EDWHAT
        BNZ     eok1
        LDR     R8, V_EDIT              ; a network typed by name: now its password
        LDR     R9, V_SSID
        LDI     32
        FCALL   s_copy
        LDR     R8, V_EDIT
        LDI     0
        STR     R8
        SETV    V_EDWHAT, ED_PASS
        SETV    V_EDMAX, 63
        FJMP    st_edit
eok1:   XRI     ED_PASS
        BNZ     eok2
        FJMP    st_conn
eok2:   FJMP    hs_save
ed_esc: LDV     V_EDWHAT
        XRI     ED_HOST
        BNZ     ees1
        FJMP    st_hosts
ees1:   FJMP    st_wifi

; ed_grid -- characters $20-$7F from row 3: the engine wraps each row.
        RFIT    24
ed_grid:
        LDI     30H
        FCALL   d_at
        LDI     20H
        PLO     R8
        LDI     TX_PUTC
        PHI     RF
edg1:   GLO     R8
        PLO     RF
        LDN     RF
        INC     R8
        GLO     R8
        XRI     80H
        BNZ     edg1
        LDI     FN_PARK
        PHI     RF
        FRET

; ed_sel  -- paint cell D inverse: the keyboard's cursor.
; ed_cell -- paint cell D plain.
        RFIT    27
ed_sel: PLO     R9
        LDI     TOP_INV_ON
        FCALL   d_op
        GLO     R9
        FCALL   ed_cell
        LDI     TOP_INV_OFF
        FJMP    d_op
ed_cell:
        PLO     R9
        ADI     30H
        FCALL   d_at
        GLO     R9
        ADI     20H
        FJMP    d_putc

; ed_text -- row 1: the text's last 14 characters, then the selected key as
; an inverse block cursor (what 5 would type), short of the row's end so
; CLREOL never wraps onto row 2.
        RFIT    54
ed_text:
        LDI     10H
        FCALL   d_at
        LDV     V_EDLEN
        SMI     15
        BNF     edt1
        LDV     V_EDLEN
        ADI     LO(V_EDIT)-14
        BR      edt2
edt1:   LDI     LO(V_EDIT)
edt2:   PLO     R8
        LDI     HI(V_EDIT)
        PHI     R8
        FCALL   d_puts
        LDI     TOP_INV_ON
        FCALL   d_op
        LDV     V_OSK
        ADI     20H
        FCALL   d_putc
        LDI     TOP_INV_OFF
        FCALL   d_op
        LDI     TOP_CLREOL
        FJMP    d_op
