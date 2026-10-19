; ui.inc -- screen scaffolding shared by every state (block 1).
;
; Every screen is the same frame: row 0 the title (or the path tail),
; rows 1-8 the list, row 9 context, row 10 messages, rows 11-12 the key
; legend. Titles, legends and messages are positioned strings (STRAT),
; drawn with PRS/PRINT; key names inside them are accent-coloured by $01
; toggles.

; UTITLE: clear the screen and print the positioned string R0:R1.
; Calls two down. Clobbers R0-R3.
UTITLE: STRZ    R2
        BSTA,UN DCLS
        LODZ    R2
        ZBRR    *ZPRS

; UMSG: the positioned string R0:R1 (a row-10 message) on a cleared
; row 10, then a ~2 s pause that any key cuts short. UFAIL: the same
; with the error code R2 in hex at the row's end and the error buzz.
; Calls three down. Clobbers R0-R3, V_T0, V_ERRC.
UFAIL:  STRA,R2 V_ERRC
        BSTR,UN UMSG0
        DAT     10,14
        LODA,R0 V_ERRC
        ZBSR    *ZDHEX
        ZBSR    *ZSNDERR
        BCTR,UN UMSG1
UMSG:   BSTR,UN UMSG0
UMSG1:  LODI,R3 120
        BCTR,UN UPAUSE
UMSG0:  STRA,R0 V_T0
        DAT     10,0
        LODI,R3 16
        ZBSR    *ZDROWS
        LODA,R0 V_T0
        ZBRR    *ZPRS

; UPAUSE: wait R3 frames or until a fresh key; R0 = the key or KNONE.
; Calls three down. Clobbers R0,R1,R3.
UPAUSE: ZBSR    *ZKEYSCN
        COMI,R0 KNONE
        BCFR,EQ UPS9
        BDRR,R3 UPAUSE
UPS9:   RETC,UN

; ROWCEL: R2 = the cell before list row V_CUR's first cell. Leaf.
ROWCEL: LODA,R0 V_CUR
        ADDI,R0 1
        RRL,R0
        RRL,R0
        RRL,R0
        RRL,R0
        SUBI,R0 1
        STRZ    R2
        RETC,UN

; ---- the list cursor --------------------------------------------------------
; LKEY: run the cursor over list rows 1..V_CNT (V_CUR 0-based, highlight
; V_HLW cells wide). KUP/KDOWN move it, wrapping -- keypad 8 has no
; "up" twin, so wrapping is how keypad-only users go back -- with a
; click. Any other key returns in R0. Calls four down.
; Clobbers R0-R3, V_T0.
LKEY:   BSTA,UN GETKEY
        COMI,R0 KDOWN
        BCTR,EQ LKDN
        COMI,R0 KUP
        BCTR,EQ LKUP
        RETC,UN
LKDN:   LODA,R1 V_CNT
        BCTR,EQ LKEY            ; nothing to move over
        LODA,R0 V_CUR
        ADDI,R0 1
        COMZ    R1
        BCTR,LT LKMV
        EORZ    R0
        BCTR,UN LKMV
LKUP:   LODA,R1 V_CNT
        BCTR,EQ LKEY
        LODA,R0 V_CUR
        SUBI,R0 1
        BCFR,LT LKMV            ; still >= 0
        LODZ    R1
        SUBI,R0 1
LKMV:   STRA,R0 V_T0            ; the new row
        LODA,R1 V_CUR
        ADDI,R1 1
        BSTA,UN DHILIT          ; old row off
        LODA,R0 V_T0
        STRA,R0 V_CUR
        STRZ    R1
        ADDI,R1 1
        BSTA,UN DHILIT          ; new row on
        BSTA,UN SNDMOV
        BCTA,UN LKEY

; LHI: highlight the cursor row (after a page draw). Leaf.
LHI:    LODA,R1 V_CUR
        ADDI,R1 1
        ZBRR    *ZDHILIT
