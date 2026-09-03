; edit.inc -- the on-screen character-grid text editor.
;
; Ports the INTV grid_entry: a 16x4 grid of 20H-5FH, a CASE toggle that
; turns the letter rows lowercase (drawn for real -- font.inc), a value
; line showing the tail of the buffer with an inverse-video cursor cell,
; and an action row (CASE SPC DEL OK ESC). Keypad shortcuts: digits type
; directly, . types a dot, CE backspaces, = accepts. Cancel is the ESC
; cell only, so a reflexive CE while backspacing can't discard the edit.
;
; Contract: caller preloads V_ENTRY (NUL-terminated) and passes the title
; in HL and the max length in A. The editor runs NO transactions, so a
; reply window the caller is holding stays valid throughout. Returns
; carry set on cancel. V_ENTRY is mutated either way; callers editing
; durable data copy out only on accept.

EVALY   EQU     12              ; value line y
EGRDX   EQU     2               ; grid origin, x byte pair
EGRDY   EQU     28              ; grid origin, y
EGPITCH EQU     10              ; grid row pitch

; EDIT: HL = title, A = max length (<= 127). Carry set on cancel.
EDIT:   LD      (V_GMAX),A
        PUSH    HL
        LD      HL,V_ENTRY
        CALL    USTRLN
        LD      A,C
        LD      (V_GLEN),A
        XOR     A
        LD      (V_GCASE),A
        LD      (V_GX),A
        LD      A,2             ; start on the A-O row
        LD      (V_GY),A
        POP     HL
        CALL    UTITLE
        CALL    EVDRAW
        CALL    EGDRAW
        CALL    EADRAW
ELOOP:  CALL    INWAIT
        CP      EVUP
        JP      Z,EMVU
        CP      EVDOWN
        JP      Z,EMVD
        CP      EVLEFT
        JP      Z,EMVL
        CP      EVRIGHT
        JP      Z,EMVR
        CP      EVTRIG
        JP      Z,EACT
        CP      EVCE
        JP      Z,EDELK
        CP      EVEQ
        JP      Z,EOK
        CP      EVDOT
        JR      NZ,ELP1
        LD      A,'.'
        JP      EAPP
ELP1:   CP      EVD0
        JR      C,ELOOP
        CP      EVD0+10
        JR      NC,ELOOP
        SUB     EVD0
        ADD     A,'0'
        JP      EAPP

; ---- movement ---------------------------------------------------------
EMVU:   LD      A,(V_GY)
        OR      A
        JP      Z,ELOOP
        CP      4
        JR      Z,EMVU4
        CALL    ECNRM
        LD      HL,V_GY
        DEC     (HL)
        CALL    ECINV
        JP      ELOOP
EMVU4:  LD      A,3             ; leave the action row: gx = gx*3+1
        LD      (V_GY),A
        LD      A,(V_GX)
        LD      B,A
        ADD     A,A
        ADD     A,B
        INC     A
        LD      (V_GX),A
        CALL    EADRAW
        CALL    ECINV
        JP      ELOOP

EMVD:   LD      A,(V_GY)
        CP      4
        JP      Z,ELOOP
        CP      3
        JR      Z,EMVD4
        CALL    ECNRM
        LD      HL,V_GY
        INC     (HL)
        CALL    ECINV
        JP      ELOOP
EMVD4:  CALL    ECNRM           ; enter the action row: gx = gx*5/16
        LD      A,4
        LD      (V_GY),A
        LD      A,(V_GX)
        LD      B,A
        ADD     A,A
        ADD     A,A
        ADD     A,B
        SRL     A
        SRL     A
        SRL     A
        SRL     A
        LD      (V_GX),A
        CALL    EADRAW
        JP      ELOOP

EMVL:   LD      A,(V_GX)
        OR      A
        JP      Z,ELOOP
        LD      A,(V_GY)
        CP      4
        JR      Z,EMVL4
        CALL    ECNRM
        LD      HL,V_GX
        DEC     (HL)
        CALL    ECINV
        JP      ELOOP
EMVL4:  LD      HL,V_GX
        DEC     (HL)
        CALL    EADRAW
        JP      ELOOP

EMVR:   LD      A,(V_GY)
        CP      4
        JR      Z,EMVR4
        LD      A,(V_GX)
        CP      15
        JP      NC,ELOOP
        CALL    ECNRM
        LD      HL,V_GX
        INC     (HL)
        CALL    ECINV
        JP      ELOOP
EMVR4:  LD      A,(V_GX)
        CP      4
        JP      NC,ELOOP
        LD      HL,V_GX
        INC     (HL)
        CALL    EADRAW
        JP      ELOOP

; ---- actions ----------------------------------------------------------
EACT:   LD      A,(V_GY)
        CP      4
        JR      Z,EACT4
        RLCA                    ; grid char: 20H + gy*16 + gx
        RLCA
        RLCA
        RLCA
        LD      B,A
        LD      A,(V_GX)
        ADD     A,B
        ADD     A,20H
        CALL    EMAPC
        JR      EAPP
EACT4:  LD      A,(V_GX)
        OR      A
        JR      Z,ECASE
        CP      1
        LD      A,' '
        JR      Z,EAPP
        LD      A,(V_GX)
        CP      2
        JR      Z,EDELK
        CP      3
        JR      Z,EOK
        SCF                     ; ESC
        RET

EOK:    OR      A
        RET

ECASE:  LD      A,(V_GCASE)
        XOR     1
        LD      (V_GCASE),A
        CALL    EGDRAW
        JP      ELOOP

EAPP:   LD      B,A
        LD      A,(V_GMAX)
        LD      C,A
        LD      A,(V_GLEN)
        CP      C
        JP      NC,ELOOP
        LD      E,A
        LD      D,0
        LD      HL,V_ENTRY
        ADD     HL,DE
        LD      (HL),B
        INC     HL
        LD      (HL),0
        INC     A
        LD      (V_GLEN),A
        CALL    EVDRAW
        JP      ELOOP

EDELK:  LD      A,(V_GLEN)
        OR      A
        JP      Z,ELOOP
        DEC     A
        LD      (V_GLEN),A
        LD      E,A
        LD      D,0
        LD      HL,V_ENTRY
        ADD     HL,DE
        LD      (HL),0
        CALL    EVDRAW
        JP      ELOOP

; EMAPC: apply the CASE toggle to grid character A.
EMAPC:  PUSH    BC
        LD      B,A
        LD      A,(V_GCASE)
        OR      A
        LD      A,B
        JR      Z,EMC9
        CP      'A'
        JR      C,EMC9
        CP      'Z'+1
        JR      NC,EMC9
        ADD     A,20H
EMC9:   POP     BC
        RET

; ---- drawing ----------------------------------------------------------
; ECINV / ECNRM: draw the cell under the grid cursor (no-op on the
; action row, which EADRAW owns).
ECINV:  LD      C,GCINV
        JR      ECDRW
ECNRM:  LD      C,GCNORM
ECDRW:  LD      A,(V_GY)
        CP      4
        RET     NC
        LD      B,A
        ADD     A,A
        ADD     A,A
        ADD     A,B
        ADD     A,A             ; gy*10
        ADD     A,EGRDY
        LD      D,A
        LD      A,B
        RLCA
        RLCA
        RLCA
        RLCA
        LD      B,A             ; gy*16
        LD      A,(V_GX)
        LD      E,A
        ADD     A,B
        ADD     A,20H
        CALL    EMAPC
        PUSH    AF
        LD      A,E
        ADD     A,EGRDX
        LD      E,A
        POP     AF
        JP      DRWCH

; EGDRAW: the whole 16x4 grid, cursor cell inverse.
EGDRAW: LD      B,0             ; gy
EGD1:   LD      C,0             ; gx
EGD2:   PUSH    BC
        LD      A,B
        ADD     A,A
        ADD     A,A
        ADD     A,B
        ADD     A,A
        ADD     A,EGRDY
        LD      D,A
        LD      A,C
        ADD     A,EGRDX
        LD      E,A
        LD      A,B
        RLCA
        RLCA
        RLCA
        RLCA
        ADD     A,C
        ADD     A,20H
        CALL    EMAPC
        PUSH    AF
        LD      A,(V_GY)
        CP      B
        JR      NZ,EGDN
        LD      A,(V_GX)
        CP      C
        JR      NZ,EGDN
        LD      C,GCINV
        JR      EGDC
EGDN:   LD      C,GCNORM
EGDC:   POP     AF
        CALL    DRWCH
        POP     BC
        INC     C
        LD      A,C
        CP      16
        JR      C,EGD2
        INC     B
        LD      A,B
        CP      4
        JR      C,EGD1
        RET

; EVDRAW: the value line -- tail window of V_ENTRY plus the cursor cell.
EVDRAW: LD      A,(V_GLEN)
        SUB     19
        JR      NC,EVD1
        XOR     A
EVD1:   LD      (V_TMP+1),A     ; window start
        LD      B,A
        LD      A,(V_GLEN)
        SUB     B
        LD      (V_TMP),A       ; cursor cell
        LD      B,0             ; cell
EVD2:   PUSH    BC
        LD      A,(V_TMP+1)
        ADD     A,B
        LD      HL,V_GLEN
        CP      (HL)
        JR      NC,EVDSP
        LD      E,A
        LD      D,0
        LD      HL,V_ENTRY
        ADD     HL,DE
        LD      A,(HL)
        JR      EVDC
EVDSP:  LD      A,' '
EVDC:   LD      C,GCNORM
        PUSH    AF
        LD      A,(V_TMP)
        CP      B
        JR      NZ,EVD3
        LD      C,GCINV
EVD3:   POP     AF
        LD      D,EVALY
        LD      E,B
        CALL    DRWCH
        POP     BC
        INC     B
        LD      A,B
        CP      20
        JR      C,EVD2
        RET

; EADRAW: the action row, selected word highlighted.
EADRAW: CALL    UCLRF
        LD      B,0
EAD1:   PUSH    BC
        LD      A,B
        ADD     A,A
        LD      E,A
        LD      D,0
        LD      HL,EAWTAB
        ADD     HL,DE
        LD      A,(HL)
        INC     HL
        LD      H,(HL)
        LD      L,A
        PUSH    HL
        LD      HL,EAXTAB
        LD      E,B
        LD      D,0
        ADD     HL,DE
        LD      E,(HL)
        POP     HL
        LD      D,FOOTY
        LD      A,(V_GY)
        CP      4
        JR      NZ,EADN
        LD      A,(V_GX)
        CP      B
        JR      NZ,EADN
        LD      C,OPTHI
        JR      EADC
EADN:   LD      C,OPTFB
EADC:   SYSTEM  STRDIS
        POP     BC
        INC     B
        LD      A,B
        CP      5
        JR      C,EAD1
        RET

EAWTAB: DW      TCASE,TSPC,TDEL,TOK,TESC
EAXTAB: DB      2,38,66,92,116
TCASE:  DB      "CASE",0
TSPC:   DB      "SPC",0
TDEL:   DB      "DEL",0
TOK:    DB      "OK",0
TESC:   DB      "ESC",0
