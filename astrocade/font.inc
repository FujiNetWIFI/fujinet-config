; font.inc -- byte-aligned 5x7 character blitter, with lowercase.
;
; The BIOS font (5x7, 7 bytes per glyph, pattern in the top 5 bits) covers
; only 20H-63H -- no lowercase -- but SSIDs and WiFi passwords are case-
; sensitive, so this module draws characters itself: BIOS glyph data for
; 20H-63H, its own 26-glyph set for a-z. Cells are 8 pixels wide and drawn
; at byte-aligned positions (2 bytes per scanline at 2bpp), which is what
; makes the blit shift-free; STRDIS keeps its 6-pixel pitch everywhere that
; doesn't need lowercase or inverse-video cells.
;
; Color spec byte: bits 1-0 = foreground pixel code, bits 3-2 = background
; (GCNORM = white on black, GCINV = black on white for the cursor cell).

BIOSFNT EQU     08E4H           ; system ROM font base ('!' = 08EBH etc.)

; GADDR: HL = screen address of cell (D = y scanline, E = x byte pair).
; Clobbers B,C.
GADDR:  LD      H,0
        LD      L,D
        ADD     HL,HL
        ADD     HL,HL
        ADD     HL,HL           ; y*8
        LD      B,H
        LD      C,L
        ADD     HL,HL
        ADD     HL,HL           ; y*32
        ADD     HL,BC           ; y*40
        LD      B,0
        LD      C,E
        ADD     HL,BC
        ADD     HL,BC           ; + x*2
        LD      BC,NORMEM
        ADD     HL,BC
        RET

; GLYPHA: HL = glyph address for character A (7 pattern bytes).
; Unprintables become '.'. Clobbers A,D,E.
GLYPHA: CP      'a'
        JR      C,GLYB
        CP      'z'+1
        JR      NC,GLYB
        SUB     'a'
        LD      HL,LCFONT
        JR      GLYI
GLYB:   CP      20H
        JR      C,GLYDOT
        CP      64H
        JR      C,GLYOK
GLYDOT: LD      A,'.'
GLYOK:  SUB     20H
        LD      HL,BIOSFNT
GLYI:   LD      E,A             ; HL += A*7
        LD      D,0
        PUSH    HL
        LD      L,E
        LD      H,0
        ADD     HL,HL
        ADD     HL,HL
        ADD     HL,HL           ; *8
        OR      A
        SBC     HL,DE           ; *7
        POP     DE
        ADD     HL,DE
        RET

; EXPAND: pattern byte A -> D,E (8 pixels at 2bpp) using color spec C.
; Preserves C. Clobbers A,B.
EXPAND: PUSH    BC
        LD      B,8
        LD      DE,0
EXPL:   SLA     E
        RL      D
        SLA     E
        RL      D
        RLCA
        PUSH    AF
        LD      A,C
        JR      C,EXPF
        SRL     A
        SRL     A               ; background code
EXPF:   AND     3
        OR      E
        LD      E,A
        POP     AF
        DJNZ    EXPL
        POP     BC
        RET

; DRWCH: draw character A at cell (E = x byte pair 0-19, D = y scanline)
; with color spec C. Draws 8 scanlines (glyph + one background line).
; Clobbers everything incl. IX.
DRWCH:  PUSH    BC
        PUSH    DE
        CALL    GLYPHA
        PUSH    HL
        POP     IX
        POP     DE
        CALL    GADDR
        POP     BC
        LD      B,8
DRWR:   LD      A,B
        DEC     A
        JR      Z,DRWBL         ; 8th line: background only
        LD      A,(IX+0)
        INC     IX
        JR      DRWX
DRWBL:  XOR     A
DRWX:   PUSH    BC
        CALL    EXPAND
        POP     BC
        LD      (HL),D
        INC     HL
        LD      (HL),E
        LD      DE,39
        ADD     HL,DE
        DJNZ    DRWR
        RET

; DRWSTR: draw the NUL-terminated string at HL from cell (E,D), color C,
; at most B characters. Clobbers everything.
DRWSTR: LD      A,(HL)
        OR      A
        RET     Z
        PUSH    HL
        PUSH    BC
        PUSH    DE
        CALL    DRWCH
        POP     DE
        POP     BC
        POP     HL
        INC     HL
        INC     E
        DJNZ    DRWSTR
        RET

; ---- lowercase glyphs, same format as the BIOS font -------------------
LCFONT: DB      000H,000H,070H,008H,078H,088H,078H        ; a
        DB      080H,080H,0F0H,088H,088H,088H,0F0H        ; b
        DB      000H,000H,078H,080H,080H,080H,078H        ; c
        DB      008H,008H,078H,088H,088H,088H,078H        ; d
        DB      000H,000H,070H,088H,0F8H,080H,070H        ; e
        DB      030H,048H,040H,0E0H,040H,040H,040H        ; f
        DB      000H,078H,088H,088H,078H,008H,070H        ; g
        DB      080H,080H,0F0H,088H,088H,088H,088H        ; h
        DB      020H,000H,060H,020H,020H,020H,070H        ; i
        DB      010H,000H,030H,010H,010H,090H,060H        ; j
        DB      080H,080H,090H,0A0H,0C0H,0A0H,090H        ; k
        DB      060H,020H,020H,020H,020H,020H,070H        ; l
        DB      000H,000H,0D0H,0A8H,0A8H,0A8H,0A8H        ; m
        DB      000H,000H,0F0H,088H,088H,088H,088H        ; n
        DB      000H,000H,070H,088H,088H,088H,070H        ; o
        DB      000H,0F0H,088H,088H,0F0H,080H,080H        ; p
        DB      000H,078H,088H,088H,078H,008H,008H        ; q
        DB      000H,000H,0B0H,0C8H,080H,080H,080H        ; r
        DB      000H,000H,078H,080H,070H,008H,0F0H        ; s
        DB      040H,040H,0E0H,040H,040H,048H,030H        ; t
        DB      000H,000H,088H,088H,088H,098H,068H        ; u
        DB      000H,000H,088H,088H,088H,050H,020H        ; v
        DB      000H,000H,088H,0A8H,0A8H,0A8H,050H        ; w
        DB      000H,000H,088H,050H,020H,050H,088H        ; x
        DB      000H,088H,088H,088H,078H,008H,070H        ; y
        DB      000H,000H,0F8H,010H,020H,040H,0F8H        ; z
