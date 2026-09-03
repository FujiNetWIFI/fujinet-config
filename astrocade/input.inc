; input.inc -- player 1 hand controller + the 24-key keypad, as events.
;
; The clients run DI/polled and the vertical-address feedback register is
; lightpen-latched (useless as a frame source), so pacing is a calibrated
; ~10 ms delay loop and all state is edge-detected against previous-state
; bytes in RAM.
;
; Directions merge the stick with the keypad's physical arrow keys and
; auto-repeat (~300 ms delay, ~80 ms rate). A diagonal keeps only the
; lowest set bit, and -- the INTV port's lesson -- a direction CHANGE
; without release goes through the repeat gate instead of firing at once,
; so a thumb parked on a boundary cannot machine-gun the list.
; Trigger and keypad keys are edge-only.

EVNONE  EQU     0
EVUP    EQU     1
EVDOWN  EQU     2
EVLEFT  EQU     3
EVRIGHT EQU     4
EVTRIG  EQU     5
EVCE    EQU     6               ; CE key (clear / cancel / backspace)
EVEQ    EQU     7               ; = key (accept / edit)
EVDOT   EQU     8               ; . key
EVD0    EQU     10H             ; digits: EVD0+n

BUP     EQU     01H             ; SW0 bits, active high
BDOWN   EQU     02H
BLEFT   EQU     04H
BRIGHT  EQU     08H
BTRIG   EQU     10H

REPDEL  EQU     30              ; auto-repeat: initial delay, ticks
REPRATE EQU     8               ; auto-repeat: rate, ticks

; DELAY10: ~10 ms at 1.789 MHz. Clobbers A,D,E.
DELAY10:
        LD      DE,745
DLY1:   DEC     DE
        LD      A,D
        OR      E
        JR      NZ,DLY1
        RET

; INPOLL: one input tick. Returns A = event (EVNONE if nothing).
; Clobbers everything.
INPOLL: CALL    DELAY10
        IN      A,(SW0)
        LD      B,A             ; keep for the trigger bit
        AND     0FH
        LD      C,A
        IN      A,(KEY2)
        AND     1               ; keypad ^ key
        JR      Z,IPK1
        SET     0,C
IPK1:   IN      A,(KEY1)
        AND     1               ; keypad v key
        JR      Z,IPK2
        SET     1,C
IPK2:   LD      A,C
        OR      A
        JR      Z,IPNOD
        LD      E,A             ; lowest set bit only
        NEG
        AND     E
        LD      E,A
        LD      A,(V_PDIR)
        CP      E
        JR      Z,IPHELD
        OR      A               ; anything held before?
        LD      A,E
        LD      (V_PDIR),A
        LD      A,REPDEL
        LD      (V_RPT),A
        JR      NZ,IPTRG        ; changed without release: gate, no event
        LD      A,E             ; fresh press: emit
        JR      IPDIR
IPHELD: LD      A,(V_RPT)
        DEC     A
        LD      (V_RPT),A
        JR      NZ,IPTRG
        LD      A,REPRATE
        LD      (V_RPT),A
        LD      A,E
IPDIR:  LD      D,EVUP          ; bit 0..3 -> EVUP..EVRIGHT
IPDB:   RRA
        JR      C,IPDG
        INC     D
        JR      IPDB
IPDG:   LD      A,D
        RET

IPNOD:  XOR     A
        LD      (V_PDIR),A

; ---- trigger, edge only ----------------------------------------------
IPTRG:  LD      A,B
        AND     BTRIG
        LD      E,A
        LD      A,(V_PTRG)
        LD      D,A
        LD      A,E
        LD      (V_PTRG),A
        OR      A
        JR      Z,IPKEYS
        LD      A,D
        OR      A
        JR      NZ,IPKEYS
        LD      A,EVTRIG
        RET

; ---- keypad matrix, edge only ----------------------------------------
IPKEYS: LD      HL,V_PKP
        LD      D,0             ; column index: KEY0..KEY3
IPKL:   LD      A,D
        ADD     A,KEY0
        LD      C,A
        LD      B,0
        IN      A,(C)
        AND     3FH
        LD      E,A             ; current
        LD      A,(HL)
        CPL
        AND     E               ; rising edges
        LD      (HL),E
        LD      C,A
        LD      A,D             ; the arrow keys (KEY1/KEY2 bit 0) are
        CP      1               ; direction inputs, not keypad edges
        JR      C,IPKM
        CP      3
        JR      NC,IPKM
        RES     0,C
IPKM:   LD      A,C
        OR      A
        JR      NZ,IPKDEC
        INC     HL
        INC     D
        LD      A,D
        CP      4
        JR      C,IPKL
        XOR     A               ; EVNONE
        RET

IPKDEC: LD      E,A             ; lowest edge bit
        NEG
        AND     E
        LD      B,0             ; bit index
IPKB:   RRA
        JR      C,IPKG
        INC     B
        JR      IPKB
IPKG:   LD      A,D             ; table index = column*6 + bit
        ADD     A,A
        ADD     A,D
        ADD     A,A
        ADD     A,B
        LD      E,A
        LD      D,0
        LD      HL,KPTAB
        ADD     HL,DE
        LD      A,(HL)
        RET

; Keypad decode: MAME astrohome.cpp's matrix, active high.
; Column KEY0 (rightmost) .. KEY3 (leftmost), bit 0 (bottom) .. 5 (top).
KPTAB:  DB      0,0,0,0,0,EVEQ                          ; KEY0: % / x - + =
        DB      0,0,EVD0+9,EVD0+6,EVD0+3,EVDOT          ; KEY1: v CH 9 6 3 .
        DB      0,0,EVD0+8,EVD0+5,EVD0+2,EVD0           ; KEY2: ^ MS 8 5 2 0
        DB      0,0,EVD0+7,EVD0+4,EVD0+1,EVCE           ; KEY3: C MR 7 4 1 CE

; INWAIT: block until an event arrives. Returns it in A.
INWAIT: CALL    INPOLL
        OR      A
        JR      Z,INWAIT
        RET

; UPAUSE: wait DE ticks (~10 ms each); any input ends the wait early.
; Returns A = the event, or EVNONE if the wait ran out.
UPAUSE: PUSH    DE
        CALL    INPOLL
        POP     DE
        OR      A
        RET     NZ
        DEC     DE
        LD      A,D
        OR      E
        JR      NZ,UPAUSE
        XOR     A
        RET
