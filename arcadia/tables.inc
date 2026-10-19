; tables.inc -- lookup tables (block 2), reached from block 1 through
; the page-0 pointer words PXLAT and PKTAB.

; ---- ASCII $20-$7F -> glyph ---------------------------------------------
; Bit 7 set = a symbol the charset lacks: DXLAT draws it as GPLACE, and
; the low bits are its glyphs.inc index, which the editor loads into the
; preview slot. ':' is glyph 7 in the CONFIG bank itself ($3F).
XLAT:   DB      $00,$88,$89,$8A,$37,GPCT,$8B,$8C        ; spc ! " # $ % & '
        DB      GLPAR,GRPAR,$8D,$36,$35,GDASH,$34,GSLASH ; ( ) * + , - . /
        DB      $10,$11,$12,$13,$14,$15,$16,$17         ; 0-7
        DB      $18,$19,GCOLON,$8E,$8F,$90,$91,$92      ; 8 9 : ; < = > ?
        DB      $93,$1A,$1B,$1C,$1D,$1E,$1F,$20         ; @ A-G
        DB      $21,$22,$23,$24,$25,$26,$27,$28         ; H-O
        DB      $29,$2A,$2B,$2C,$2D,$2E,$2F,$30         ; P-W
        DB      $31,$32,$33,$94,GBSLSH,$95,$96,GUSCOR   ; X Y Z [ \ ] ^ _
        DB      $97,$1A,$1B,$1C,$1D,$1E,$1F,$20         ; ` a-g
        DB      $21,$22,$23,$24,$25,$26,$27,$28         ; h-o
        DB      $29,$2A,$2B,$2C,$2D,$2E,$2F,$30         ; p-w
        DB      $31,$32,$33,$98,$99,$9A,$9B,$86         ; x y z { | } ~ del

; ---- keypad column nibble -> key code -------------------------------------
; Highest bit wins, so a mashed pair cannot alias to a third key. 4/6/8
; are the arrow keys (see input.inc).
KTAB:   DB      KNONE,KCLR,7,7,KLEFT,KLEFT,KLEFT,KLEFT  ; $1900: 1 4 7 Clear
        DB      1,1,1,1,1,1,1,1
        DB      KNONE,0,KDOWN,KDOWN,5,5,5,5             ; $1901: 2 5 8 0
        DB      2,2,2,2,2,2,2,2
        DB      KNONE,KENT,9,9,KRIGHT,KRIGHT,KRIGHT,KRIGHT ; $1902: 3 6 9 Ent
        DB      3,3,3,3,3,3,3,3
