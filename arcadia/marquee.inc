; marquee.inc -- the browser's long-name marquee (block 2).
;
; A selected row whose name is wider than its 13 cells rests for REST
; frames, then SFULL reads the whole name into the reply window, which
; holds still while the cursor does, and the name slides one character
; every STEP frames, holding HOLD frames at each end. GETKEY calls
; MQTICK on every idle frame and MQREST before it returns a control.
; V_MQL says where the row stands: $FF off, 0 fresh, 1 nothing to scroll,
; else the name's length. V_ROWM bit 7 marks the rows that overflow.

REST    EQU     50
STEP    EQU     6
HOLD    EQU     40

; MQARM: a fresh row. Leaf. Clobbers R0.
MQARM:  EORZ    R0
        STRA,R0 *PVMQL
        STRA,R0 *PVMQO
        LODI,R0 REST
        STRA,R0 *PVMQT
        RETC,UN

; MQREST: the row back at its start, and ready for the next one. Calls
; four down. Clobbers R0-R3, V_T2.
MQREST: LODA,R0 *PVMQL
        RETC,LT                 ; off
        COMI,R0 14
        BCTR,LT MQARM           ; never scrolled
        LODA,R0 *PVMQO
        BCTR,EQ MQARM
        EORZ    R0
        STRA,R0 *PVMQO
        BSTA,UN MQPAINT
        BCTR,UN MQARM

; MQTICK: one idle frame. Calls four down. Clobbers R0-R3, V_T2, V_NLEN.
MQTICK: LODA,R0 *PVMQL
        RETC,LT                 ; off
        COMI,R0 1
        RETC,EQ                 ; this name fits after all
        LODA,R1 *PVCUR
        LODA,R0 *PROWM,R1
        BCTR,LT MQT1            ; bit 7: a name wider than the row
        RETC,UN
MQT1:   LODA,R0 *PVMQT
        SUBI,R0 1
        STRA,R0 *PVMQT
        RETC,GT
        LODA,R0 *PVMQL
        BCFR,EQ MQSTEP          ; the name is read: slide it
        BSTA,UN SFULL
        BCFR,EQ MQDEAD
        BSTA,UN SNAMLN          ; R3 = its length
        LODA,R1 *PVCUR
        LODA,R0 *PROWM,R1
        ANDI,R0 $40
        BCTR,EQ MQT2
        SUBI,R3 1               ; a folder's '/' is not shown
MQT2:   LODZ    R3
        COMI,R0 14
        BCTR,LT MQDEAD
        STRA,R0 *PVMQL
        LODI,R0 HOLD
        STRA,R0 *PVMQT
        RETC,UN
MQDEAD: LODI,R0 1
        STRA,R0 *PVMQL
        RETC,UN

MQSTEP: LODI,R0 STEP
        STRA,R0 *PVMQT
        LODA,R0 *PVMQO
        ADDI,R0 13
        COMA,R0 *PVMQL
        BCTR,LT MQADV
        EORZ    R0              ; the end was on show: back to the start
        STRA,R0 *PVMQO
        LODI,R0 HOLD
        STRA,R0 *PVMQT
        BCTR,UN MQPAINT
MQADV:  LODA,R0 *PVMQO
        ADDI,R0 1
        STRA,R0 *PVMQO
        ADDI,R0 13
        COMA,R0 *PVMQL
        BCTR,LT MQPAINT
        LODI,R0 HOLD            ; the end is on show now
        STRA,R0 *PVMQT

; MQPAINT: the 13 cells of the cursor row's name from character V_MQO, in
; the selected-row colours, blank past the end. Calls two down.
; Clobbers R0-R3, V_T2.
MQPAINT: LODI,R0 13
        STRA,R0 *PVT2
        ZBSR    *ZROWCEL        ; R2 = the cell before column 0
        ADDI,R2 1
        LODA,R1 *PVCUR
        LODA,R0 *PROWM,R1
        ANDI,R0 $40             ; a folder: the accent ink (ASELA), else ASEL
        STRA,R0 *PDCOL
        LODA,R3 *PVMQO
MQP1:   LODZ    R3
        COMA,R0 *PVMQL
        BCTR,LT MQP2
        EORZ    R0              ; past the end of the name
        BCTR,UN MQP3
MQP2:   LODA,R0 FNRDATA,R3
        BSTA,UN DXLAT
MQP3:   ZBSR    *ZDPUTC
        ADDI,R3 1
        LODA,R0 *PVT2
        SUBI,R0 1
        STRA,R0 *PVT2
        BCFR,EQ MQP1
        RETC,UN
