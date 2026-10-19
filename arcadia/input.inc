; input.inc -- keypad + two-axis disc, folded into one code space
; (block 1). The two-axis scheme is fujinet-battleship/arcadia's.
;
; Hardware facts that shape this:
;   - FIRE is wired to the SAME line as keypad '2' ($1901 b3): they are
;     indistinguishable, so '2' only ever means select.
;   - The P1 disc is an analog pot valid ONLY in vertical blank; read
;     elsewhere it looks like "down". Every scan syncs to a fresh vblank
;     edge, which also gives CONFIG its frame clock: one KEYSCN = one
;     frame.
;   - Both disc axes arrive on $19FF; $19F9 b6 (the pot mux) selects
;     which -- clear = vertical, set = horizontal. The mux flips once per
;     frame and only the selected axis is sampled, but BOTH values are
;     cached and the direction decodes from the cache, so a held
;     direction reads the same every frame (what edge detection needs).
;
; Keypad 4/6/8 report as KLEFT/KRIGHT/KDOWN (tables.inc): with '2' gone
; there is no keypad "up", so lists wrap at the bottom instead, and the
; arrow keys get the disc's auto-repeat for free.

KNONE   EQU     $FF
KFIRE   EQU     2               ; FIRE and keypad 2
; The disc directions are contiguous so the auto-repeat gate is two
; compares; Enter/Clear sit above them and never repeat.
KUP     EQU     $80
KDOWN   EQU     $81
KLEFT   EQU     $82
KRIGHT  EQU     $83
KENT    EQU     $84
KCLR    EQU     $85

JOYP1   EQU     $19FF           ; P1 pot; the axis depends on BGCOL b6
JOYLO   EQU     $40             ; < this = up / left
JOYHI   EQU     $C0             ; > this = down / right
JOYCTR  EQU     $6F             ; the resting value

RPTDLY  EQU     18              ; frames before a held direction repeats
RPTINT  EQU     4               ; frames between repeats

; WAITVB: wait for the next vertical-blank EDGE (SENSE low, then high).
; Leaf. Preserves everything but CC.
WAITVB: TPSU    $80
        BCTR,EQ WAITVB          ; still in the previous vblank
WVB1:   TPSU    $80
        BCFR,EQ WVB1
        RETC,UN

; KEYRAW: R0 = the first control found this frame (KNONE if none).
; Costs exactly one frame. Calls one down. Clobbers R0,R1.
KEYRAW: LODI,R0 BGCB            ; arm the mux for this frame's axis
        LODA,R1 AXSEL
        BCTR,EQ KRARM
        IORI,R0 $40
KRARM:  STRA,R0 BGCOL
        BSTR,UN WAITVB          ; a whole frame with the mux settled
        LODA,R0 JOYP1
        STRZ    R1
        LODA,R0 AXSEL
        BCTR,EQ KRSTV
        LODZ    R1
        STRA,R0 AXH
        EORZ    R0
        BCTR,UN KRSTD
KRSTV:  LODZ    R1
        STRA,R0 AXV
        LODI,R0 1
KRSTD:  STRA,R0 AXSEL
; Decode from the cache. Vertical wins, so a diagonal is one axis.
        LODA,R0 AXV
        COMI,R0 JOYLO
        BCTR,LT KRUP
        COMI,R0 JOYHI
        BCTR,GT KRDN
        LODA,R0 AXH
        COMI,R0 JOYLO
        BCTR,LT KRLF
        COMI,R0 JOYHI
        BCTR,GT KRRT
        LODA,R0 $1900           ; b3 '1' b2 '4' b1 '7' b0 Clear
        ANDI,R0 $0F
        BCFR,EQ KRC0
        LODA,R0 $1901           ; b3 '2'/FIRE b2 '5' b1 '8' b0 '0'
        ANDI,R0 $0F
        BCFR,EQ KRC1
        LODA,R0 $1902           ; b3 '3' b2 '6' b1 '9' b0 Enter
        ANDI,R0 $0F
        BCFR,EQ KRC2
        LODI,R0 KNONE
        RETC,UN
KRUP:   LODI,R0 KUP
        RETC,UN
KRDN:   LODI,R0 KDOWN
        RETC,UN
KRLF:   LODI,R0 KLEFT
        RETC,UN
KRRT:   LODI,R0 KRIGHT
        RETC,UN
KRC2:   ADDI,R0 16              ; three 16-byte tables, one pointer
KRC1:   ADDI,R0 16
KRC0:   STRZ    R1
        LODA,R0 *PKTAB,R1
        RETC,UN

PKTAB:  DWBE    KTAB            ; tables.inc, block 2

; KEYSCN: edge-detected KEYRAW -- a control reports once per press; a
; held direction auto-repeats (RPTDLY, then every RPTINT frames).
; Calls two down. Clobbers R0,R1.
KEYSCN: BSTA,UN KEYRAW
        COMA,R0 V_KEY
        BCTR,EQ KSHELD          ; unchanged: held, or still idle
        STRA,R0 V_KEY           ; a fresh control: latch, arm the repeat
        STRZ    R1
        LODI,R0 RPTDLY
        STRA,R0 V_RPT
        LODZ    R1
        COMI,R0 KNONE
        BCTR,EQ KSNONE          ; the release edge reports nothing
        RETC,UN
KSHELD: COMI,R0 KUP             ; digits are < $80: never repeat
        BCTR,LT KSNONE
        COMI,R0 KRIGHT          ; KENT/KCLR/KNONE: never repeat
        BCTR,GT KSNONE
        LODA,R0 V_RPT
        SUBI,R0 1
        STRA,R0 V_RPT
        BCFR,EQ KSNONE
        LODI,R0 RPTINT
        STRA,R0 V_RPT
        LODA,R0 V_KEY
        RETC,UN
KSNONE: LODI,R0 KNONE
        RETC,UN

; GETKEY: block for a fresh control; R0 = its code. Calls three down.
GETKEY: BSTR,UN KEYSCN
        COMI,R0 KNONE
        BCTR,EQ GETKEY
        RETC,UN
