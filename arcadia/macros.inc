; macros.inc -- the assembler macros every module uses.
;
; Included first: AS resolves forward labels but not forward macros.
;
; Page rules these exist for: non-branch memory instructions reach only
; their own 8K page, so page-1 (block 2) code names page-0 RAM through
; DWBE pointer words, and page-0 (block 1) code names the mailbox the
; same way. Branch absolutes carry 15 bits and need no help.

; Big-endian word: what the 2650's indirection reads. DW is little-endian
; on this target, so never use DW for anything the CPU dereferences.
DWBE    MACRO   val
        DB      ((val)>>8)&$FF, (val)&$FF
        ENDM

; A positioned string starts with its screen cell minus one (the
; pre-increment convention of every screen store), then ASCII, then 0.
; Byte $01 inside a string toggles the accent ink.
STRAT   MACRO   row,col
        DB      ((row)*16+(col)-1)&$FF
        ENDM

; PRINT: draw a positioned string in the normal colour (6 bytes; works
; from either page: it only loads registers, and ZBSR always reaches
; page 0).
PRINT   MACRO   adr
        LODI,R0 (adr)>>8
        LODI,R1 (adr)&$FF
        ZBSR    *ZPRS
        ENDM

; Point DSTRP at a string (page 0 form, then page 1 form).
SETSTR  MACRO   adr
        LODI,R0 (adr)>>8
        STRA,R0 DSTRP
        LODI,R0 (adr)&$FF
        STRA,R0 DSTRP+1
        ENDM

SETST1  MACRO   adr
        LODI,R0 (adr)>>8
        STRA,R0 *PDSTRH
        LODI,R0 (adr)&$FF
        STRA,R0 *PDSTRL
        ENDM

; R2 = the cell before (row,col), for STRA,R0 SCREEN,R2,+ stores.
DAT     MACRO   row,col
        LODI,R2 ((row)*16+(col)-1)&$FF
        ENDM

DCOL    MACRO   attr
        LODI,R0 attr
        STRA,R0 DCOLOR
        ENDM

DCOL1   MACRO   attr
        LODI,R0 attr
        STRA,R0 *PDCOL
        ENDM

SETFS1  MACRO   adr             ; page-1 form of txlib.inc's SETFS
        LODI,R0 (adr)>>8
        STRA,R0 *PFSTRH
        LODI,R0 (adr)&$FF
        STRA,R0 *PFSTRL
        ENDM
