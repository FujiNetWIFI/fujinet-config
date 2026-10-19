; bartest.inc -- BARTEST=1 builds boot into widget test screens for
; snapshot review of the palette, the mosaic encodings and the glyphs
; (make shot). Screen A ~frame 60, screen B ~frame 300.

BTEST:  PRINT   SBTA
        LODI,R3 1               ; eight sample list rows
BTA1:   LODZ    R3
        STRA,R0 V_T2
        RRL,R0
        RRL,R0
        RRL,R0
        RRL,R0
        SUBI,R0 1
        STRZ    R2
        SETSTR  SBTROW
        DCOL    ANORM
        LODA,R0 V_T2
        ANDI,R0 1
        BCTR,EQ BTA2
        DCOL    AACC            ; odd rows: the directory colour
BTA2:   LODI,R1 14
        BSTA,UN DPRTN
        LODA,R3 V_T2
        ADDI,R3 1
        COMI,R3 9
        BCTR,LT BTA1
        LODI,R0 14
        STRA,R0 V_HLW
        LODI,R1 3
        BSTA,UN DHILIT
        LODI,R0 0               ; 48 entries, page at 16
        STRA,R0 V_TOT
        STRA,R0 V_DPOS
        LODI,R0 48
        STRA,R0 V_TOT+1
        LODI,R0 16
        STRA,R0 V_DPOS+1
        BSTA,UN SBAR
        LODI,R0 33              ; bars at 33 / 66 / 100 / 0 on rows 9-12
        BSTA,UN BTBAR
        LODI,R2 9*16
        BSTA,UN BTMOV
        LODI,R0 66
        BSTA,UN BTBAR
        LODI,R2 10*16
        BSTA,UN BTMOV
        LODI,R0 100
        BSTA,UN BTBAR
        LODI,R2 11*16
        BSTA,UN BTMOV
        LODI,R0 0
        BSTA,UN BTBAR
        LODI,R2 12*16
        BSTA,UN BTMOV
        LODI,R0 50              ; and 50% left on BARROW with its text
        BSTA,UN BARPCT
        LODI,R3 240
BTW:    BSTA,UN WAITVB
        BDRR,R3 BTW

        BSTA,UN DCLS            ; screen B: signal cells, sweep, charset
        PRINT   SBTB
        LODI,R1 1
        LODI,R0 $B0
        BSTA,UN SIGNAL
        LODI,R1 2
        LODI,R0 $C0
        BSTA,UN SIGNAL
        LODI,R1 3
        LODI,R0 $FF
        BSTA,UN SIGNAL
        LODI,R0 5
        BSTA,UN SWEEP
        LODI,R3 64              ; glyphs $00-$3F on rows 9-12
BTB1:   LODZ    R3
        SUBI,R0 1
        IORI,R0 ANORM
        STRA,R0 SCREEN+9*16-1,R3
        BDRR,R3 BTB1
BTHALT: BCTR,UN BTHALT

; BTBAR: the bar at R0 percent on BARROW (steps only).
BTBAR:  STRA,R0 V_T2
        ANDI,R0 $FE
        RRR,R0
        STRZ    R1
        LODA,R0 V_T2
BTB2:   SUBI,R0 20
        BCTR,LT BTB3
        SUBI,R1 1
        BCTR,UN BTB2
BTB3:   LODZ    R1
        BCTA,UN BARSTP

; BTMOV: copy BARROW to the row starting at cell R2.
BTMOV:  LODI,R3 16
BTM1:   LODA,R0 SCREEN+BARCEL-1,R3
        STRZ    R1
        LODZ    R2
        ADDZ    R3
        STRA,R0 V_T3
        LODA,R0 V_T3
        SUBI,R0 1
        STRZ    R1
        LODA,R0 SCREEN+BARCEL-1,R3
        STRA,R0 SCREEN,R1
        BDRR,R3 BTM1
        RETC,UN
