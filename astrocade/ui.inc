; ui.inc -- screen scaffolding shared by every state.
;
; The layout is ten 8-pixel rows: title (y=1), eight list rows starting at
; LISTY0, footer (y=FOOTY). List text goes through LINBUF + STRDIS at its
; native 6-pixel pitch (26 columns); the selection cursor is a chevron in
; the byte-aligned gutter column, drawn with the font.inc blitter so moving
; it never needs the row's text again (directory rows would otherwise cost
; a mailbox round trip apiece to repaint).

; MSET: fill BC bytes at HL with A. Clobbers everything.
MSET:   LD      (HL),A
        DEC     BC
        LD      A,B
        OR      C
        RET     Z
        LD      D,H
        LD      E,L
        INC     DE
        LDIR
        RET

; Clear the whole screen / the list area / the footer band.
UCLRA:  LD      HL,NORMEM
        LD      BC,LINES*BYTEPL
        JR      UCLR0
UCLRL:  LD      HL,NORMEM+9*BYTEPL
        LD      BC,64*BYTEPL
        JR      UCLR0
UCLRF:  LD      HL,NORMEM+72*BYTEPL
        LD      BC,8*BYTEPL
UCLR0:  XOR     A
        JP      MSET

; UTITLE: clear the screen and show the title string at HL.
UTITLE: PUSH    HL
        CALL    UCLRA
        POP     HL
        LD      DE,TITLEY*256+2
        LD      C,OPTFB
        SYSTEM  STRDIS
        RET

; UFOOT: replace the footer with the string at HL.
UFOOT:  PUSH    HL
        CALL    UCLRF
        POP     HL
        LD      DE,FOOTY*256+2
        LD      C,OPTFB
        SYSTEM  STRDIS
        RET

; UROWY: D = y scanline of list row B.
UROWY:  LD      A,B
        ADD     A,A
        ADD     A,A
        ADD     A,A
        ADD     A,LISTY0
        LD      D,A
        RET

; USHOWR: display LINBUF on list row B with STRDIS options C.
USHOWR: PUSH    BC
        CALL    UROWY
        POP     BC
        LD      E,LISTX
        LD      HL,LINBUF
        SYSTEM  STRDIS
        RET

; UCRON / UCROFF: draw / erase the chevron cursor on list row B.
UCRON:  LD      A,'>'
        JR      UCR1
UCROFF: LD      A,' '
UCR1:   PUSH    AF
        CALL    UROWY
        POP     AF
        LD      E,0
        LD      C,GCNORM
        JP      DRWCH

; USTRLN: C = length of the NUL-terminated string at HL (HL ends on the
; NUL). Clobbers A.
USTRLN: LD      C,0
USL1:   LD      A,(HL)
        OR      A
        RET     Z
        INC     HL
        INC     C
        JR      USL1

; UTAIL: copy the LAST (up to) B characters of the string at HL into
; LINBUF, sanitized -- for paths too long to fit a row. Clobbers everything.
; USTRLN returns the length in C and leaves B alone, so no save is needed
; (and saving would clobber the length we just measured).
UTAIL:  CALL    USTRLN          ; C = length, HL -> the NUL, B kept = max
        LD      A,C
        CP      B
        JR      C,UTAI1
        LD      A,B
UTAI1:  LD      E,A
        LD      D,0
        OR      A
        SBC     HL,DE
        LD      B,A
        JP      CPYSAN

; UPCT: render A (0-100) as "NN%" into HEXBUF, NUL-terminated, no
; leading zeros. Clobbers A,B,D,E,H.
UPCT:   LD      DE,HEXBUF
        CP      100
        JR      C,UPC1
        LD      A,'1'
        LD      (DE),A
        INC     DE
        LD      A,'0'
        LD      (DE),A
        INC     DE
        LD      (DE),A
        INC     DE
        JR      UPC9
UPC1:   LD      B,'0'
UPC2:   CP      10
        JR      C,UPC3
        SUB     10
        INC     B
        JR      UPC2
UPC3:   LD      H,A
        LD      A,B
        CP      '0'
        JR      Z,UPC4
        LD      (DE),A
        INC     DE
UPC4:   LD      A,H
        ADD     A,'0'
        LD      (DE),A
        INC     DE
UPC9:   LD      A,'%'
        LD      (DE),A
        INC     DE
        XOR     A
        LD      (DE),A
        RET
