; bar.inc -- block-graphics widgets (block 1): the progress bar, the
; busy sweep, the browser scrollbar, the WiFi signal cell.
;
; The 2637 has a mosaic mode no other FujiNet client uses: a cell of
; EXACTLY $C0 switches the rest of its row to block graphics (the switch
; cell draws blank), and each row starts over in text mode. A mosaic
; cell's low 6 bits light a 3x2 grid -- top half bit2 left 3 px, bit1
; middle 3 px, bit0 right 2 px; bottom half bits 5/4/3 the same columns
; -- and its attribute bits still pick ink and background (MAME
; arcadia_v.cpp vh_draw_line / video_start). Rules kept here:
;   - a mosaic run always goes to the end of its row: an exact $40 would
;     switch back to text and draw a stray bg-A cell;
;   - no data cell is ever an exact $C0 or $40 (every value below has
;     a nonzero pattern, or is $00 / $80).
;
;   bar     $00 empty (black trough), $64 1/3, $76 2/3, $7F full (yellow)
;   track   $89 thin white line on the page
;   thumb   $DB full cell, $CB top half, $D9 bottom half (yellow)
;   signal  $E0 / $F2 / $FB one, two, three rising bars

BARROW  EQU     6               ; the progress bar's screen row
BARCEL  EQU     BARROW*16

; BARSTP: the progress bar at R0 steps (0-45: 15 cells x 3) on BARROW.
; Leaf. Clobbers R0-R3, V_T0.
BARSTP: STRA,R0 V_T0
        LODI,R0 $C0
        STRA,R0 SCREEN+BARCEL
        LODI,R2 BARCEL
        LODI,R3 15
BS1:    LODA,R0 V_T0
        COMI,R0 3
        BCTR,LT BS2
        SUBI,R0 3
        STRA,R0 V_T0
        LODI,R1 3
        BCTR,UN BS3
BS2:    STRZ    R1              ; the partial cell, then nothing left
        EORZ    R0
        STRA,R0 V_T0
BS3:    LODA,R0 BARCLS,R1
        STRA,R0 SCREEN,R2,+
        BDRR,R3 BS1
        RETC,UN
BARCLS: DB      $00,$64,$76,$7F

; BARPCT: the bar at R0 percent (0-100) plus "NN%" on the row below.
; steps = pct/2 - pct/20 = pct*45/100 within half a step.
; Calls two down. Clobbers R0-R3, V_T0, V_T2, V_FLD.
BARPCT: STRA,R0 V_T2
        ANDI,R0 $FE
        RRR,R0
        STRZ    R1              ; pct/2
        LODA,R0 V_T2
BP1:    SUBI,R0 20
        BCTR,LT BP2
        SUBI,R1 1
        BCTR,UN BP1
BP2:    LODZ    R1
        BSTA,UN BARSTP
        DAT     BARROW+2,5
        LODI,R3 5
        ZBSR    *ZDROWS
        DAT     BARROW+2,6
        DCOL    ANORM
        LODA,R0 V_T2
        BSTA,UN DDEC
        LODI,R0 GPCT
        ZBRR    *ZDPUTC

; SWEEP: the busy indicator for waits with no percentage: a two-cell
; block at phase R0 (0-15) on BARROW. Leaf. Clobbers R0-R3, V_T0.
SWEEP:  STRA,R0 V_T0
        LODI,R0 $C0
        STRA,R0 SCREEN+BARCEL
        LODI,R2 BARCEL
        LODI,R3 0
SW1:    LODZ    R3
        COMA,R0 V_T0
        BCTR,EQ SWON
        ADDI,R0 1
        COMA,R0 V_T0
        BCTR,EQ SWON
        EORZ    R0
        BCTR,UN SWST
SWON:   LODI,R0 $7F
SWST:   STRA,R0 SCREEN,R2,+
        ADDI,R3 1
        COMI,R3 15
        BCTR,LT SW1
        RETC,UN

; SWTICK: advance the sweep one frame (it steps every fourth). Calls one
; down. Clobbers R0-R3, V_T0.
SWTICK: LODA,R0 V_FRM
        ADDI,R0 1
        STRA,R0 V_FRM
        ANDI,R0 3
        BCFR,EQ SWT9
        LODA,R0 V_SWP
        ADDI,R0 1
        COMI,R0 16
        BCTR,LT SWT1
        EORZ    R0
SWT1:   STRA,R0 V_SWP
        BCTA,UN SWEEP
SWT9:   RETC,UN

; SBAR: the browser scrollbar in cols 14-15 of rows 1-8, from V_DPOS
; and V_TOT. The 16 half-cells split the directory into equal bins; a
; half is lit when its bin overlaps the visible page [DPOS, DPOS+8), so
; the thumb is proportional and never less than half a cell.
; Scratch: V_T3:4 TOT, V_T5:6 DPOS then LO = DPOS*16, V_T7:8 HI = LO+128,
; V_T0:1 the running bin edge. Calls one down. Clobbers R0-R3, V_T*.
SBAR:   LODA,R0 V_TOT
        STRA,R0 V_T3
        LODA,R0 V_TOT+1
        STRA,R0 V_T4
        LODA,R0 V_DPOS
        STRA,R0 V_T5
        LODA,R0 V_DPOS+1
        STRA,R0 V_T6
SBSC:   LODA,R0 V_T3            ; halve both until TOT fits a byte
        BCTR,EQ SBS2
        PPSL    $08             ; WC: the rotates go through carry
        CPSL    $01
        LODA,R0 V_T3
        RRR,R0
        STRA,R0 V_T3
        LODA,R0 V_T4
        RRR,R0
        STRA,R0 V_T4
        CPSL    $01
        LODA,R0 V_T5
        RRR,R0
        STRA,R0 V_T5
        LODA,R0 V_T6
        RRR,R0
        STRA,R0 V_T6
        CPSL    $09
        BCTR,UN SBSC
SBS2:   LODI,R2 15              ; col 15 of row 0; +16 per row
        LODI,R3 8
        LODA,R0 V_T4
        COMI,R0 9
        BCTA,LT SBNONE          ; one page holds it all
        LODA,R0 V_T6            ; LO = DPOS*16, HI = LO + 128
        RRL,R0
        RRL,R0
        RRL,R0
        RRL,R0
        STRZ    R1
        ANDI,R0 $0F
        STRA,R0 V_T5
        LODZ    R1
        ANDI,R0 $F0
        STRA,R0 V_T6
        ADDI,R0 $80
        STRA,R0 V_T8
        LODA,R0 V_T5
        TPSL    $01
        BCFR,EQ SBH1
        ADDI,R0 1
SBH1:   STRA,R0 V_T7
        EORZ    R0
        STRA,R0 V_T0
        STRA,R0 V_T1
SBRW:   BSTR,UN SBLIT           ; top half
        RRL,R0
        STRA,R0 V_T2
        BSTR,UN SBLIT           ; bottom half
        IORA,R0 V_T2
        STRZ    R1
        LODA,R0 SBCELL,R1
        ADDI,R2 16
        STRA,R0 SCREEN,R2
        LODI,R0 $C0
        STRA,R0 SCREEN-1,R2
        BDRR,R3 SBRW
        RETC,UN
SBNONE: LODI,R0 ANORM
        ADDI,R2 16
        STRA,R0 SCREEN,R2
        LODI,R0 $C0
        STRA,R0 SCREEN-1,R2
        BDRR,R3 SBNONE
        RETC,UN
SBCELL: DB      $89,$D9,$CB,$DB ; none, bottom, top, both

; SBLIT: R0 = 1 when the bin [EDGE, EDGE+TOT) overlaps [LO, HI), else 0;
; then EDGE += TOT. Leaf. Clobbers R0,R1.
SBLIT:  LODI,R1 0
        LODA,R0 V_T0            ; EDGE < HI ?
        COMA,R0 V_T7
        BCTR,LT SBA1
        BCFR,EQ SBA0
        LODA,R0 V_T1
        COMA,R0 V_T8
        BCFR,LT SBA0
SBA1:   LODI,R1 1
SBA0:   LODA,R0 V_T1            ; EDGE += TOT
        ADDA,R0 V_T4
        STRA,R0 V_T1
        LODA,R0 V_T0
        TPSL    $01
        BCFR,EQ SBA2
        ADDI,R0 1
SBA2:   STRA,R0 V_T0
        COMA,R0 V_T5            ; new EDGE > LO ?
        BCTR,GT SBB1
        BCFR,EQ SBB0
        LODA,R0 V_T1
        COMA,R0 V_T6
        BCTR,GT SBB1
SBB0:   EORZ    R0
        RETC,UN
SBB1:   LODZ    R1
        RETC,UN

; SIGNAL: the RSSI cell for screen row R1 (cols 14-15). R0 = RSSI, a
; negative dBm byte (fujinet-pc's dummy networks report 255).
; Leaf. Clobbers R0-R2.
SIGNAL: STRZ    R2
        LODZ    R1
        RRL,R0
        RRL,R0
        RRL,R0
        RRL,R0
        ADDI,R0 14
        STRZ    R1
        LODI,R0 $C0
        STRA,R0 SCREEN,R1
        LODI,R0 $E0             ; worse than -70 dBm
        COMI,R2 $BB
        BCTR,LT SIG1
        LODI,R0 $F2             ; worse than -55
        COMI,R2 $CA
        BCTR,LT SIG1
        LODI,R0 $FB
SIG1:   STRA,R0 SCREEN+1,R1
        RETC,UN
