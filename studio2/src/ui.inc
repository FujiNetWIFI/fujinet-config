; ui.inc -- what every screen shares: inline text, legends, keys, the key
; dispatcher, the error line, list movement and the string helpers.
;
; The screen is the cart's 16x10 text engine: row 0 a title, rows 1-8 a list
; (NROWS), row 9 a legend of keys ("5:GO 0:UP"). The selected row is an
; inverse bar, and a name too long for it scrolls there (MARQUEE). Text that
; lives in the reply window is printed by the cart itself (d_rpl/d_rplpad),
; so a long name never crosses the CPU.
;
; Routines here are FCALLed and may clobber R8, R9, RB.0 and RD.1; RA and RC
; belong to the states.

; pr_cls -- FCALL pr_cls; DB "title",0: clear the screen, title on row 0.
; pr_at  -- FCALL pr_at; DB rc, "text",0: text at row r, column c.
; pr_in  -- FCALL pr_in; DB "text",0: text at the cursor.
        RFIT    35
pr_cls: LDI     TOP_CLS
        HOT     TX_OP
        BR      pr_in
pr_at:  LDA     R6
        HOT     TX_CURSOR
pr_in:  LDI     TX_PUTC
        PHI     RF
pri1:   LDA     R6
        BZ      pri2
        PLO     RF
        LDN     RF
        BR      pri1
pri2:   LDI     FN_PARK
        PHI     RF
        FRET

; pr_leg -- FCALL pr_leg; DB "text",0: the legend on row 9, "5:GO 0:UP".
; pr_key -- FCALL pr_key; DB rc, "text",0: the same at row r, column c.
; Both blank the rest of the row. Digits stay plain: an inverse 3x5 digit is
; barely legible.
        RFIT    34
pr_leg: LDI     90H
        BR      plg0
pr_key: LDA     R6
plg0:   HOT     TX_CURSOR
        LDI     TX_PUTC
        PHI     RF
plg1:   LDA     R6
        BZ      plg2
        PLO     RF
        LDN     RF
        BR      plg1
plg2:   LDI     TOP_CLREOL
        HOT     TX_OP
        FRET

; key_wait -- D = the next key, 0-9, from either pad. A held key repeats
; only for 2/4/6/8: a held select must not fire twice.
; key_tmo  -- the same, but D = $FF once D seconds pass without one.
        RFIT    67
key_tmo: FCALL  fn_frames
        LDI     1
        BR      kw0
key_wait:
        LDI     0
kw0:    STV     V_KTO
kw1:    LDV     V_KLAST
        STV     V_KOLD
        FCALL   in_get
        BNZ     kw2
        LDV     V_KTO
        BZ      kw1
        FCALL   fn_tick
        BDF     kw1
        LDI     0FFH
        FRET
kw2:    STR     R2
        LDV     V_KOLD
        XOR
        BNZ     kw3                     ; a fresh press
        LDN     R2
        ANI     0FH
        BZ      kw1
        SHR
        BDF     kw1                     ; an odd key: no repeat
kw3:    LDI     2
        PLO     RD                      ; a click
        LDN     R2
        ANI     0FH
        FRET

; kdisp -- FCALL kdisp with D = a key, then KEYTO entries and DB $FF: go to
; the first entry's address whose key is D (with D kept), else return past
; the table. The jump is a return with R6 swapped, so the stack balances.
        RFIT    27
kdisp:  PLO     R9
kd1:    LDA     R6
        XRI     0FFH
        BZ      kd3                     ; the end of the table
        XRI     0FFH
        STR     R2
        GLO     R9
        XOR
        BZ      kd2
        INC     R6
        INC     R6
        BR      kd1
kd2:    LDA     R6
        PHI     R9
        LDN     R6
        PLO     R6
        GHI     R9
        PHI     R6
        GLO     R9
        FRET
kd3:    GLO     R9
        FRET

; go_ind -- go to the address held at RB (high byte first).
        RFIT    5
go_ind: LDA     RB
        PHI     R6
        LDN     RB
        PLO     R6
        FRET

; fail -- FCALL fail; DB step; DW resume, with D = the error: show both on
; the legend row ("ERROR ss:ee KEY"), beep, wait for a key, then go to
; `resume`.
        RFIT    69
fail:   PLO     RC
        LDA     R6
        PHI     RC
        LDA     R6
        PHI     RA
        LDA     R6
        PLO     RA
        LDI     TOP_INV_ON
        FCALL   d_op
        FCALL   pr_at
        DB      90H, "ERROR", 0
        LDI     TOP_INV_OFF
        FCALL   d_op
        LDI     ' '
        FCALL   d_putc
        GHI     RC
        FCALL   d_hex
        LDI     ':'
        FCALL   d_putc
        GLO     RC
        FCALL   d_hex
        FCALL   pr_in
        DB      " KEY", 0
        LDI     TOP_CLREOL
        FCALL   d_op
        LDI     30
        PLO     RD                      ; a long beep
        FCALL   key_wait
        GHI     RA
        PHI     R6
        GLO     RA
        PLO     R6
        FRET

; is_cur -- DF = 1 if V_ROW is the cursor's row.
; lst_up -- move the cursor up: DF = 1, V_OLD = the old row, if it moved.
; lst_dn -- move it down within V_NROW rows, the same way.
        RFIT    71
is_cur: LDV     V_CUR
        STR     R2
        LDV     V_ROW
        XOR
        BZ      lyes
lno:    ADI     0                       ; DF = 0
        FRET
lyes:   SMI     0                       ; DF = 1
        FRET
lst_up: LDV     V_CUR
        BZ      lno
        STV     V_OLD
        SMI     1
        STV     V_CUR
        BR      lyes
lst_dn: LDV     V_CUR
        STV     V_OLD
        ADI     1
        STR     R2
        LDV     V_NROW
        SM                              ; rows - (cursor + 1)
        BZ      lno
        BNF     lno
        LDN     R2
        STV     V_CUR
        BR      lyes

; wait_s -- wait D seconds.
        RFIT    9
wait_s: FCALL   fn_frames
ws1:    FCALL   fn_tick
        BDF     ws1
        FRET

; s_copy -- copy the string at R8 to R9: at most D characters, always
; NUL-terminated.
        RFIT    22
s_copy: STV     V_N
scp1:   LDA     R8
        STR     R9
        BZ      scp2
        INC     R9
        LDV     V_N
        SMI     1
        STR     RB
        BNZ     scp1
        STR     R9
scp2:   FRET

; tx_fix -- append the string at R8 to the TX stream, NUL-padded to exactly D
; bytes (0: 256). Every fixed-width field: paths, SSID, password, host name.
        RFIT    27
tx_fix: PLO     R9
        LDI     HI(FN_TXDATA)
        PHI     RF
txf1:   LDA     R8
        BZ      txf2
        PLO     RF
        LDN     RF
        DEC     R9
        GLO     R9
        BNZ     txf1
        BR      txf4
txf2:   LDI     0
        PLO     RF
        LDN     RF
        DEC     R9
        GLO     R9
        BNZ     txf2
txf4:   LDI     FN_PARK
        PHI     RF
        FRET

; tx_str -- append the string at R8 at its exact length: COPY_FILE's spec.
        RFIT    14
tx_str: LDI     HI(FN_TXDATA)
        PHI     RF
txs1:   LDA     R8
        BZ      txs2
        PLO     RF
        LDN     RF
        BR      txs1
txs2:   LDI     FN_PARK
        PHI     RF
        FRET

; path_add -- append the string at R8 to V_PATH; DF = 0, and V_PATH as it
; was, if it does not fit.
        RFIT    43
path_add:
        LDR     R9, V_PATH
pa1:    LDA     R9
        BNZ     pa1
        DEC     R9
        GLO     R9
        STV     V_PEND
pa2:    GLO     R9
        XRI     LO(V_PATH+PATHMAX-1)
        BZ      pa4
        LDA     R8
        STR     R9
        BZ      pa3
        INC     R9
        BR      pa2
pa3:    SMI     0                       ; DF = 1
        FRET
pa4:    LDV     V_PEND
        PLO     R9
        LDI     0
        STR     R9
        ADI     0                       ; DF = 0
        FRET

; path_up  -- drop V_PATH's last directory: "/a/b/" -> "/a/".
; path_cut -- cut V_PATH after its last '/': drops a file name.
        RFIT    41
path_up:
        LDR     R8, V_PATH
pu1:    LDA     R8
        BNZ     pu1
        DEC     R8
        DEC     R8
        LDI     0
        STR     R8                      ; the trailing '/'
path_cut:
        LDR     R8, V_PATH
        LDR     R9, V_PATH
pc1:    LDA     R8
        BZ      pc2
        XRI     '/'
        BNZ     pc1
        GLO     R8
        PLO     R9                      ; just past this '/'
        BR      pc1
pc2:    LDI     0
        STR     R9
        FRET

; def_glyphs -- the folder glyph: a tab over a box, in the font's 3x5 grid.
        RFIT    38
def_glyphs:
        LDR     R8, gl_dir
        LDI     13
        PLO     R9
dgl1:   LDA     R8
        FCALL   d_par
        DEC     R9
        GLO     R9
        BNZ     dgl1
        LDI     TOP_GLYPH
        FCALL   d_op
        FRET
gl_dir: DB      GL_DIR, 0, 0CH, 0CH, 0EH, 0EH, 0AH, 0AH, 0AH, 0AH, 0EH, 0EH, 0
s_slash: DB     "/", 0
