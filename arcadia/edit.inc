; edit.inc -- the on-screen character-grid text editor (block 1).
;
; A 16 x 6 grid of ASCII $20-$7F fills rows 3-8 exactly; row 1 shows
; the tail of the value with a black cursor cell, row 2 its length.
; The charset has no lowercase and only a few symbols, so:
;   - lowercase draws as the uppercase glyph in the accent ink;
;   - the 21 symbols with no glyph draw as the placeholder box, except
;     under the grid cursor, where UDC slot $3F (':' everywhere else) is
;     loaded with the real glyph -- the cursor sits on the character a
;     FIRE would type, so that one preview is all the grid needs.
;
; Keys: the disc (and keypad 4/6/8) move the grid cursor, wrapping;
; FIRE types; Clear deletes; Enter accepts; 0 escapes.
;
; Contract: the caller preloads the buffer at V_EBUF (NUL-terminated),
; sets V_EMAX, and passes the title (a row-0 string) in R0:R1. The
; editor runs NO transactions, so a reply window the caller is holding
; stays valid. R0 = 0 / CC EQ on Enter, R0 = 1 on ESC; the buffer is
; changed either way, so callers editing durable data edit a copy.
;
; Scratch: V_ECUR (V_T2) cursor flag, V_ECH (V_T3) the character,
; V_ECEL (V_T4) the cell, V_T5/V_T6 loop state.

V_ECUR  EQU     V_T2
V_ECH   EQU     V_T3
V_ECEL  EQU     V_T4

EDIT:   ZBSR    *ZUTITLE
        LODI,R3 $FF
ED1:    LODA,R0 *V_EBUF,R3,+
        BCFR,EQ ED1
        STRA,R3 V_ELEN
        LODI,R0 1               ; start on 'A'
        STRA,R0 V_GX
        LODI,R0 2
        STRA,R0 V_GY
        PRINT   LEDIT
        BSTA,UN EGRID
        BSTA,UN EFIELD
ELOOP:  BSTA,UN GETKEY
        COMI,R0 KUP
        BCTA,LT ENOTMV          ; codes below the arrows
        COMI,R0 KRIGHT
        BCTA,GT ENOTMV
        STRA,R0 V_T6            ; an arrow: un-cursor the old cell first
        LODI,R1 0
        BSTA,UN ECELL
        LODA,R0 V_T6
        COMI,R0 KUP
        BCTR,EQ EMU
        COMI,R0 KDOWN
        BCTR,EQ EMD
        COMI,R0 KLEFT           ; (every load sets CC: branch first)
        BCTR,EQ EML
        LODA,R0 V_GX
        ADDI,R0 1
        BCTR,UN EMX
EML:    LODA,R0 V_GX
        SUBI,R0 1
EMX:    ANDI,R0 15
        STRA,R0 V_GX
        BCTR,UN EMOVED
EMU:    LODA,R0 V_GY
        SUBI,R0 1
        BCFR,LT EMY
        LODI,R0 5
        BCTR,UN EMY
EMD:    LODA,R0 V_GY
        ADDI,R0 1
        COMI,R0 6
        BCTR,LT EMY
        EORZ    R0
EMY:    STRA,R0 V_GY
EMOVED: LODI,R1 1
        BSTA,UN ECELL
        BSTA,UN SNDMOV
        ZBRR    *ZELOOP

ENOTMV: COMI,R0 KFIRE
        BCTR,EQ EADD
        COMI,R0 KCLR
        BCTA,EQ EDEL
        COMI,R0 KENT
        BCTA,EQ EOK
        COMI,R0 0
        BCTA,EQ EESC
        ZBRR    *ZELOOP

EADD:   LODA,R0 V_ELEN
        COMA,R0 V_EMAX
        BCFA,LT EBAD            ; full
        BSTA,UN ECHAR
        COMI,R0 $7F
        BCTA,EQ EBAD            ; the one cell that is not a character
        LODA,R1 V_ELEN
        STRA,R0 *V_EBUF,R1
        ADDI,R1 1
        STRA,R1 V_ELEN
        EORZ    R0
        STRA,R0 *V_EBUF,R1
        BSTA,UN SNDKEY
        BCTR,UN EREDO
EDEL:   LODA,R1 V_ELEN
        BCTA,EQ EBAD
        SUBI,R1 1
        STRA,R1 V_ELEN
        EORZ    R0
        STRA,R0 *V_EBUF,R1
        BSTA,UN SNDMOV
EREDO:  BSTA,UN EFIELD
        ZBRR    *ZELOOP
EBAD:   ZBSR    *ZSNDERR
        ZBRR    *ZELOOP
EOK:    ZBSR    *ZSNDSEL
        BSTR,UN EEXIT
        EORZ    R0
        RETC,UN
EESC:   ZBSR    *ZSNDBCK
        BSTR,UN EEXIT
        LODI,R0 1
        RETC,UN
EEXIT:  ZBSR    *ZWAITVB          ; ':' back into the preview slot
        LODI,R1 7
        LODI,R2 7
        BCTA,UN UDCLD

; ECHAR: R0 = the character under the grid cursor. Leaf.
ECHAR:  LODA,R0 V_GY
        RRL,R0
        RRL,R0
        RRL,R0
        RRL,R0
        IORA,R0 V_GX
        ADDI,R0 $20
        RETC,UN

; ECELL: redraw the grid cell under the cursor; R1 = 1 as the cursor.
; Calls three down. Clobbers R0-R3.
ECELL:  STRA,R1 V_ECUR
        LODA,R0 V_GY            ; R2 = the cell before (gy+3, gx)
        ADDI,R0 3
        RRL,R0
        RRL,R0
        RRL,R0
        RRL,R0
        ADDA,R0 V_GX
        SUBI,R0 1
        STRZ    R2
        BSTR,UN ECHAR
        BCTR,UN EPUTC

; EGRID: all 96 cells, the cursor's drawn as the cursor. Calls four
; down. Clobbers R0-R3, V_T5, V_T6.
EGRID:  BSTR,UN ECHAR
        STRA,R0 V_T5
        LODI,R0 $20
EG1:    STRA,R0 V_T6
        LODI,R1 0
        COMA,R0 V_T5
        BCFR,EQ EG2
        LODI,R1 1
EG2:    STRA,R1 V_ECUR
        ADDI,R0 $0F             ; the cell before row 3 + (c - $20)
        STRZ    R2
        LODA,R0 V_T6
        BSTR,UN EPUTC
        LODA,R0 V_T6
        ADDI,R0 1
        COMI,R0 $80
        BCTR,LT EG1
        RETC,UN

; EPUTC: draw character R0 at the cell after R2, as the grid cursor when
; V_ECUR is set. Calls two down. Clobbers R0-R3, V_T1; R2 ends on the cell.
EPUTC:  STRA,R2 V_ECEL
        STRA,R0 V_ECH
        LODI,R1 ANORM
        LODA,R0 V_ECUR
        BCTR,EQ EP1
        LODI,R1 ASEL
EP1:    LODA,R0 V_ECH
        COMI,R0 'a'
        BCTR,LT EP2
        COMI,R0 'z'+1
        BCFR,LT EP2
        IORI,R1 $40             ; lowercase: the accent ink
EP2:    STRA,R1 DCOLOR
        SUBI,R0 $20
        STRZ    R1
        LODA,R0 *PXLAT,R1       ; the raw entry, flags and all
        BCTR,LT EPSYM
        COMI,R0 GCOLON          ; ':' is symbol 7 here (its slot is the
        BCFR,EQ EPPUT           ;   preview)
        LODI,R0 $87
EPSYM:  LODA,R1 V_ECUR
        BCTR,EQ EPPLC
        ANDI,R0 $7F             ; the cursor: preview the real glyph
        STRZ    R2
        LODI,R1 7
        ZBSR    *ZWAITVB
        BSTA,UN UDCLD
        LODI,R0 GCOLON
        BCTR,UN EPPUT
EPPLC:  LODI,R0 GPLACE
EPPUT:  LODA,R2 V_ECEL
        ZBRR    *ZDPUTC

; EFIELD: row 1 = the tail of the value and a black cursor cell; row 2
; = "length/max". Calls three down. Clobbers R0-R3, V_T0, V_T5, V_FLD.
EFIELD: LODA,R0 V_ELEN
        SUBI,R0 15
        BCFR,LT EF1
        EORZ    R0
EF1:    STRA,R0 V_T5            ; index of the first char shown
        EORZ    R0
        STRA,R0 V_ECUR
        DAT     1,0
        LODI,R3 16
EF2:    LODA,R0 V_T5
        COMA,R0 V_ELEN
        BCFR,LT EF3
        STRZ    R1
        LODA,R0 *V_EBUF,R1
        BSTA,UN EPUTC
        BCTR,UN EF4
EF3:    BCTR,EQ EF30            ; the first cell past the end is the
        LODI,R0 ANORM           ;   cursor (CC is still the COMA's)
        BCTR,UN EF31
EF30:   LODI,R0 ASEL
EF31:   STRA,R0 SCREEN,R2,+
EF4:    LODA,R0 V_T5
        ADDI,R0 1
        STRA,R0 V_T5
        BDRR,R3 EF2
        DAT     2,10
        LODI,R3 6
        ZBSR    *ZDROWS
        DCOL    ANORM
        DAT     2,10
        LODA,R0 V_ELEN
        BSTA,UN DDEC
        LODI,R0 GSLASH
        ZBSR    *ZDPUTC
        LODA,R0 V_EMAX
        BCTA,UN DDEC

; EDITB: EDIT on the zone-B buffer whose low address byte is R2 (every
; edit buffer lives in page $1A), max length R3, title R0:R1.
EDITB:  STRA,R2 V_EBUF+1
        STRA,R3 V_EMAX
        LODI,R2 V_PATH>>8
        STRA,R2 V_EBUF
        BCTA,UN EDIT
