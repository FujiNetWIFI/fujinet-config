; maria.s -- the 32 x 24 text and tile screen; see maria.h.
;
; Nothing here stores to $00-$1F: INPTCTRL is left unlocked.

        .include "zeropage.inc"

        .export _mt_init, _mt_off, _mt_on, _mt_sync, _mt_background
        .export _mt_palette, _mt_clear, _mt_at, _mt_put, _mt_puts
        .export _mt_fill, _mt_setpal, _mt_get, _mt_text
        .export _mt_map, _mt_attr, _mt_pal, _mt_frames
        .import _mt_tiles, popa, popax

BACKGRND = $20
P0C1    = $21
MWSYNC  = $24
MSTAT   = $28                   ; bit 7: vertical blank
DPPH    = $2C
DPPL    = $30
P4C1    = $31
CHARBASE = $34
OFFSET  = $38
CTRL    = $3C

COLS    = 32
ROWS    = 24
MAXRUNS = 12
SLOT    = 64                    ; bytes per display list slot
HDR_WM  = $E0                   ; extended header, indirect, write mode 1
DMA_ON  = $56                   ; DMA on, 2-byte characters, kangaroo, 320B
DMA_OFF = $7F
HPOS0   = 16                    ; (160 - 4 * COLS) / 2 in HPOS units: centred
PAL_LINES = 270                 ; NTSC MARIA draws 242 lines, PAL 292
TOP     = 4                     ; blank DLL entries above the rows
BOTTOM  = 4

        .segment "VIDEO"        ; page aligned, in the cart's RAM
_mt_map:  .res  ROWS * COLS     ; tile * 2 per cell
_mt_attr: .res  ROWS * COLS     ; palette select per cell, 3 pages on
dls:    .res    ROWS * 2 * SLOT ; two lists per row, in one page
dll:    .res    (TOP + ROWS + BOTTOM) * 3
nulldl: .res    2
dirty:  .res    ROWS            ; the row's palettes changed
slot:   .res    ROWS            ; the slot MARIA reads

        .zeropage
mrow:   .res    2               ; the cursor's row in the map
arow:   .res    2               ; ...and in the palette selects
ccol:   .res    1
crow:   .res    1

        .segment "BSS"
_mt_pal:    .res 1
_mt_frames: .res 1
brow:   .res    1
rpal:   .res    1
rstart: .res    1
rend:   .res    1
runs:   .res    1
oidx:   .res    1
syncrow: .res   1

        .code

; void __fastcall__ mt_init (unsigned char blank)
.proc _mt_init
        pha
        jsr     _mt_off
        jsr     measure
        lda     #0
        sta     nulldl
        sta     nulldl+1
        sta     _mt_frames

        ldx     #0              ; the blank lines above, NTSC or PAL
        ldy     _mt_pal
        beq     @top
        ldy     #TOP
@top:   lda     toplines,y
        sta     dll,x
        lda     #>nulldl
        sta     dll+1,x
        lda     #<nulldl
        sta     dll+2,x
        inx
        inx
        inx
        iny
        cpx     #TOP * 3
        bne     @top
        ldy     #0              ; the rows, each 8 lines
@row:   lda     #7
        sta     dll,x
        lda     dlhi,y
        sta     dll+1,x
        lda     dllo,y
        sta     dll+2,x
        lda     #0
        sta     slot,y
        inx
        inx
        inx
        iny
        cpy     #ROWS
        bne     @row
@bot:   lda     #15             ; and enough below to reach the blank
        sta     dll,x
        lda     #>nulldl
        sta     dll+1,x
        lda     #<nulldl
        sta     dll+2,x
        inx
        inx
        inx
        cpx     #(TOP + ROWS + BOTTOM) * 3
        bne     @bot

        pla
        jsr     _mt_clear
        jsr     rebuild

        lda     #>_mt_tiles
        sta     CHARBASE
        lda     #0
        sta     OFFSET
        lda     #>dll
        sta     DPPH
        lda     #<dll
        sta     DPPL
        jmp     _mt_on
.endproc

; Count MARIA's lines from the end of one vertical blank to the next.
.proc measure
@in:    bit     MSTAT
        bpl     @in
@out:   bit     MSTAT
        bmi     @out
        ldx     #0
        ldy     #0
@line:  sta     MWSYNC
        inx
        bne     @same
        iny
@same:  bit     MSTAT
        bpl     @line
        lda     #0
        cpy     #>PAL_LINES
        bcc     @set
        bne     @pal
        cpx     #<PAL_LINES
        bcc     @set
@pal:   lda     #1
@set:   sta     _mt_pal
        rts
.endproc

.proc _mt_off
        lda     #DMA_OFF
        sta     CTRL
        rts
.endproc

; DMA may only start in a vertical blank.
.proc _mt_on
@w:     bit     MSTAT
        bpl     @w
        lda     #DMA_ON
        sta     CTRL
        rts
.endproc

.proc _mt_sync
@in:    bit     MSTAT           ; let a blank already under way end
        bmi     @in
@out:   bit     MSTAT
        bpl     @out
        inc     _mt_frames
        ; fall through
.endproc

; Rebuild every marked row.
.proc rebuild
        ldx     #0
@r:     lda     dirty,x
        beq     @next
        lda     #0
        sta     dirty,x
        stx     syncrow
        jsr     build
        ldx     syncrow
@next:  inx
        cpx     #ROWS
        bne     @r
        rts
.endproc

; Row X's list into its spare slot: one header per run of cells sharing a
; palette, then publish it with one store to the row's DLL entry.
.proc build
        stx     brow
        lda     rowlo,x
        sta     ptr1            ; the row's palette selects
        sta     tmp1            ; ...and map, low byte
        lda     rowhi,x
        sta     tmp2            ; map, high byte
        clc
        adc     #>(_mt_attr - _mt_map)
        sta     ptr1+1
        lda     slot,x
        eor     #1
        sta     slot,x
        beq     @s0
        lda     #SLOT
@s0:    clc
        adc     dllo,x
        sta     ptr2
        lda     dlhi,x
        sta     ptr2+1

        ldy     #0
        sty     runs
        sty     oidx
@run:   cpy     #COLS
        bcs     @term
        lda     (ptr1),y
        sta     rpal
        sty     rstart
        lda     runs
        cmp     #MAXRUNS - 1
        bcc     @scan
        ldy     #COLS           ; the last run allowed takes the rest
        bne     @emit
@scan:  iny
        cpy     #COLS
        bcs     @emit
        lda     (ptr1),y
        cmp     rpal
        beq     @scan
@emit:  sty     rend
        ldy     oidx
        lda     tmp1
        clc
        adc     rstart
        sta     (ptr2),y        ; +0 map, low
        iny
        lda     #HDR_WM
        sta     (ptr2),y        ; +1
        iny
        lda     tmp2
        sta     (ptr2),y        ; +2 map, high
        iny
        lda     rstart
        sec
        sbc     rend            ; -width; the field is 32 - width
        and     #$1F
        ldx     rpal
        beq     @p0
        ora     #$80            ; palette 4
@p0:    sta     (ptr2),y        ; +3
        iny
        lda     rstart
        asl     a
        asl     a
        adc     #HPOS0
        sta     (ptr2),y        ; +4 hpos
        iny
        sty     oidx
        inc     runs
        ldy     rend
        jmp     @run

@term:  ldy     oidx
        lda     #0
        sta     (ptr2),y
        iny
        sta     (ptr2),y
        ldx     brow
        ldy     dllpos,x
        lda     ptr2
        sta     dll,y
        rts
.endproc

; void __fastcall__ mt_background (unsigned char c)
.proc _mt_background
        sta     BACKGRND
        rts
.endproc

; void __fastcall__ mt_palette (unsigned char p, c1, c2, c3)
.proc _mt_palette
        sta     tmp3
        jsr     popa
        sta     tmp2
        jsr     popa
        sta     tmp1
        jsr     popa
        tax
        beq     @p0
        ldx     #P4C1 - P0C1
@p0:    lda     tmp1
        sta     P0C1,x
        lda     tmp2
        sta     P0C1+1,x
        lda     tmp3
        sta     P0C1+2,x
        rts
.endproc

; void __fastcall__ mt_clear (unsigned char blank)
.proc _mt_clear
        asl     a
        ldx     #0
@c:     sta     _mt_map,x
        sta     _mt_map+$100,x
        sta     _mt_map+$200,x
        inx
        bne     @c
        txa
@a:     sta     _mt_attr,x
        sta     _mt_attr+$100,x
        sta     _mt_attr+$200,x
        inx
        bne     @a
        lda     #1
        ldx     #ROWS - 1
@d:     sta     dirty,x
        dex
        bpl     @d
        rts
.endproc

; void __fastcall__ mt_at (unsigned char col, unsigned char row)
.proc _mt_at
        cmp     #ROWS
        bcc     @ok
        lda     #ROWS - 1
@ok:    sta     crow
        tax
        lda     rowlo,x
        sta     mrow
        sta     arow
        lda     rowhi,x
        sta     mrow+1
        clc
        adc     #>(_mt_attr - _mt_map)
        sta     arow+1
        jsr     popa
        sta     ccol
        rts
.endproc

; void __fastcall__ mt_put (unsigned char tile)
.proc _mt_put
        ldy     ccol
        cpy     #COLS
        bcs     @done
        asl     a
        sta     (mrow),y
        inc     ccol
@done:  rts
.endproc

; void __fastcall__ mt_puts (const char *s)
; Indexes the string by column: ptr1 = s - ccol.
.proc _mt_puts
        sec
        sbc     ccol
        sta     ptr1
        txa
        sbc     #0
        sta     ptr1+1
        ldy     ccol
@c:     cpy     #COLS
        bcs     @done
        lda     (ptr1),y
        beq     @done
        asl     a
        sta     (mrow),y
        iny
        bne     @c
@done:  sty     ccol
        rts
.endproc

; unsigned char __fastcall__ mt_text (const char *s, unsigned char max)
; As mt_puts, at most `max` characters, and anything outside 32-126 is a
; space. Returns how many it wrote.
.proc _mt_text
        sta     tmp1
        jsr     popax
        sec
        sbc     ccol
        sta     ptr1
        txa
        sbc     #0
        sta     ptr1+1
        lda     ccol
        sta     tmp3
        clc
        adc     tmp1
        bcs     @cap
        cmp     #COLS + 1
        bcc     @end
@cap:   lda     #COLS
@end:   sta     tmp2            ; the column to stop at
        ldy     ccol
@c:     cpy     tmp2
        bcs     @done
        lda     (ptr1),y
        beq     @done
        cmp     #' '
        bcc     @blank
        cmp     #127
        bcc     @put
@blank: lda     #' '
@put:   asl     a
        sta     (mrow),y
        iny
        bne     @c
@done:  sty     ccol
        tya
        sec
        sbc     tmp3
        ldx     #0
        rts
.endproc

; void __fastcall__ mt_fill (unsigned char tile, unsigned char n)
.proc _mt_fill
        sta     tmp1
        jsr     popa
        asl     a
        ldy     ccol
        ldx     tmp1
        beq     @done
@f:     cpy     #COLS
        bcs     @done
        sta     (mrow),y
        iny
        dex
        bne     @f
@done:  sty     ccol
        rts
.endproc

; void __fastcall__ mt_setpal (unsigned char p, unsigned char n)
.proc _mt_setpal
        tax
        jsr     popa
        sta     tmp1
        txa
        beq     @done
        ldy     ccol
@s:     cpy     #COLS
        bcs     @done
        lda     (arow),y
        cmp     tmp1
        beq     @same
        lda     tmp1
        sta     (arow),y
        lda     #1
        sty     tmp2
        ldy     crow
        sta     dirty,y
        ldy     tmp2
@same:  iny
        dex
        bne     @s
@done:  rts
.endproc

; unsigned char __fastcall__ mt_get (unsigned char col, unsigned char row)
.proc _mt_get
        tax
        lda     rowlo,x
        sta     ptr1
        lda     rowhi,x
        sta     ptr1+1
        jsr     popa
        tay
        lda     (ptr1),y
        lsr     a
        ldx     #0
        rts
.endproc

        .rodata
toplines:
        .byte   15, 6, 0, 0     ; NTSC: 25 lines
        .byte   15, 15, 15, 1   ; PAL: 50
rowlo:
        .repeat ROWS, r
        .byte   <(_mt_map + r * COLS)
        .endrepeat
rowhi:
        .repeat ROWS, r
        .byte   >(_mt_map + r * COLS)
        .endrepeat
dllo:
        .repeat ROWS, r
        .byte   <(dls + r * 2 * SLOT)
        .endrepeat
dlhi:
        .repeat ROWS, r
        .byte   >(dls + r * 2 * SLOT)
        .endrepeat
dllpos:
        .repeat ROWS, r
        .byte   (TOP + r) * 3 + 2
        .endrepeat
