; fujidisp.inc -- the 48-pixel text display, in colour, with a frame.
;
; The cartridge composes the glyphs (firmware/src/vcs_render.c) and publishes
; them as six 128-byte planes at $1800-$1AFF; this is the console half, which
; streams them into GRP0/GRP1 on a cycle-exact schedule.
;
; This copy has diverged from the other 2600 clients'. The six writes are the
; same writes in the same order, one cycle later each (see KERN), and the sixth
; line of every cell -- always blank in the font -- is no longer streamed: it is
; where the next row's colours and the selection bar are set up (see HK).
;
; DINIT sets the TIA up and positions the players. DLOOP runs frames forever.

; ---------------- kernel timing constants ----------------
; These are not taste. The 48-pixel block is six player copies 2.67 cycles
; apart and the kernel's stores are 3 cycles, so where the block sits decides
; whether each graphics update lands between copies or inside one. The window
; that satisfies all six groups is ONE PIXEL wide, and these are its
; coordinates, found by byte-comparing the raster against the renderer
; (emu/dispcheck.py) rather than by eye -- every wrong setting still looks like
; text.
;
; RESP0 lands on cycle 39, one later than the window was found at, and every
; kernel write is one cycle later too, so the window is the same window and the
; block is three pixels further right: x55..102, centred in the frame.
RDLY    EQU     7               ; DINIT's delay loop: 2 + 7*5 - 1 = 36 cycles
HM0     EQU     15              ; HMP0 = $F0: move P0 right one pixel, closing
                                ;   the gap to P1 (HMP1 stays 0) to eight
VDEL    EQU     1               ; vertical delay on both players

; NTSC: 3 vsync + 37 vblank + 192 visible + 30 overscan = 262.
VBTIME  EQU     43              ; TIM64T from line 3 to the end of line 39
OSLINES EQU     30
PADLINE EQU     (192-TLINES)/2  ; 33 above and below

; The frame is PF2 alone, mirrored: bit 0 is x48..51 and x108..111, so $01 is
; the sides and $FF the whole span between them.
PFSIDE  EQU     $01
FRMH    EQU     4               ; lines in the top and bottom edges

; ---------------- DINIT: TIA setup and player positioning ----------------
DINIT:
; Two players, three copies each, eight pixels apart: P0 draws groups 0,2,4
; and P1 groups 1,3,5, interleaving to 48 contiguous pixels. VDELP is what
; makes the six-write sequence land in the right copies.
        lda     #$03            ; NUSIZ: three copies, close
        sta     NUSIZ0
        sta     NUSIZ1
        lda     #VDEL
        sta     VDELP0
        sta     VDELP1
        IF      VDEL<>1
        FATAL   "DINIT stores VDEL into CTRLPF as its reflect bit"
        ENDIF
        sta     CTRLPF          ; mirrored playfield, players in front of it

; Player positioning, after batari Basic's std_kernel.asm: RESP0 then RESP1
; three cycles later (nine pixels, since two stores cannot be closer), and HMP0
; pulling P0 one pixel right to make it eight. The delay is a loop, so it must
; not cross a page: a taken branch that does costs a cycle per pass.
        sta     WSYNC
        ldx     #RDLY
DIN1:   dex
        bne     DIN1            ; 36
        IF      (DIN1>>8)<>((DIN1+3)>>8)
        FATAL   "DINIT's delay loop crosses a page"
        ENDIF
        sta     RESP0           ; 39
        sta     RESP1           ; 42
        lda     #HM0*16
        sta     HMP0
; An EARLY HMOVE -- WSYNC, then strobe -- rather than batari's late one at
; cycle 73. A late HMOVE does not apply the motion nibbles cleanly: nudging
; HMP0 by one here widened the player gap from 8 pixels to 9 (the block grew
; from 43 to 44 pixels of span) instead of sliding the block. With the early
; strobe the nibbles behave, and one-pixel steps are what this kernel needs --
; the window between group 0 and group 5 failing is a single pixel wide.
        sta     WSYNC
        sta     HMOVE

        rts

; ---------------- DLOOP: draw frames, forever ----------------
DLOOP:
; ---------------- frame ----------------
        lda     #2
        sta     VSYNC
        sta     WSYNC
        sta     WSYNC
        sta     WSYNC
        lda     #0
        sta     VSYNC

        lda     #2
        sta     VBLANK
        lda     #VBTIME
        sta     TIM64T
; The client's per-frame hook, called while the screen is still blanked and
; the raster is nowhere near the text. Input scanning belongs here; so does a
; transaction, with the caveat that a network round trip takes far longer than
; a frame and the picture will stall for it. The timer absorbs however long it
; takes, up to the end of the blanking.
        jsr     APPVBL
        jsr     VBTAIL          ; the bar's row, a sound tick, VBLANK off

; ---------------- the frame's top edge ----------------
        lda     #COLFRM
        sta     COLUPF
        ldx     #PADLINE-1
PADA:   sta     WSYNC
        cpx     #FRMH+1
        lda     #0
        sbc     #0              ; $FF on the last FRMH lines, else 0
        sta     PF2             ;  9, in HBLANK
        dex
        bne     PADA
        dex
        stx     ROWN            ; row -1: the next housekeeping line sets row 0

; The stack pointer is a spare register in the kernel, and that is not a
; flourish: the six player copies are 2.67 cycles apart while a zero-page
; store is 3, so the four late writes drift a full cycle across the block.
; Recovering a byte from zero page costs 3 cycles; `tsx` costs 2. That one
; cycle is the whole difference between a schedule that closes and one that
; does not -- it is why batari Basic's kernel parks a byte in S with `txs` and
; recovers it with `tsx`, and why the seventh write is `sty GRP0`: its value
; is a don't-care, so Y (the scanline counter) serves.
;
; Nothing here may use the stack between the txs and the restore below -- no
; JSR, and the 6507 has no interrupts to worry about.
        tsx
        stx     SAVSP
        ldy     #$FF            ; HK's iny makes it scanline 0
        bne     HKW             ; always: the last pad line is row 0's HK line

; ---------------- the kernel ----------------
; Five lines of ink per row. Y is the screen-wide scanline index and must
; survive the whole frame. The first store is absolute, four cycles, which is
; what makes every later one a cycle later than the window was measured at --
; matching RESP0's cycle 39.
KERN:   sta     WSYNC
        lda     TP0,y           ; 4
        DB      $8D,GRP0,$00    ; 8   sta GRP0, absolute: new GRP0 = group 0
        lda     TP1,y           ; 4   -> 12
        sta     GRP1            ; 3   -> 15   displays group 0
        lda     TP2,y           ; 4   -> 19
        sta     GRP0            ; 3   -> 22   displays group 1
        lda     TP4,y           ; 4   -> 26
        tax                     ; 2   -> 28
        txs                     ; 2   -> 30   park group 4 in S
        lda     TP3,y           ; 4   -> 34
        tax                     ; 2   -> 36
        lda     TP5,y           ; 4   -> 40
        stx     GRP1            ; 3   -> 43   displays group 2
        tsx                     ; 2   -> 45   recover group 4
        stx     GRP0            ; 3   -> 48   displays group 3
        sta     GRP1            ; 3   -> 51   displays group 4
        sty     GRP0            ; 3   -> 54   displays group 5; value unused
        iny                     ; 2   -> 56
        cpy     ROWEND          ; 3   -> 59
        bne     KERN            ; 3   -> 62

; ---------------- the housekeeping line ----------------
; The sixth line of a cell, blank in every glyph, so nothing is lost by not
; streaming it. It blanks the players, sets the playfield for the next row --
; the bar if that row holds the '>' (VBTAIL found it) or is the title, the
; frame's sides otherwise -- and the next row's text colour. The playfield
; stores land before x48, where PF2's first pixel is; COLUP0/1 change while
; the players are blank. Worst case, KERN's WSYNC ends by cycle 71.
HKW:    sta     WSYNC
        lda     #0              ;  2
        sta     GRP0            ;  5
        sta     GRP1            ;  8   with VDELP set, clearing both displayed
        sta     GRP0            ; 11   registers takes three writes
        iny                     ; 13   the next row's first scanline
        beq     HKB             ; 15   row 0: the title strip
        cpy     BARY            ; 18
        beq     HKB             ; 20
        ldx     #PFSIDE         ; 22
        lda     #COLFRM         ; 24
        bne     HKC             ; 27   always
HKB:    ldx     #$FF
        lda     #COLBAR
HKC:    sta     COLUPF          ; <=31
        stx     PF2             ; <=34, x <=34
        cpy     #TLINES
        beq     KEXIT
        tya
        adc     #TCELLH-1       ; carry clear: Y < TLINES
        sta     ROWEND
        inc     ROWN
        ldx     ROWN
        lda     RCOLTB,x        ; the bank's own colours, one per row
        sta     COLUP0
        sta     COLUP1
        bcc     KERN            ; <=68, always

; ---------------- the frame's bottom edge ----------------
KEXIT:  ldx     SAVSP
        txs
        ldx     #0
PADB:   sta     WSYNC
        cpx     #FRMH
        lda     #0
        sbc     #0              ; $FF on the first FRMH lines, else 0
        sta     PF2
        inx
        cpx     #PADLINE
        bne     PADB

        lda     #2
        sta     VBLANK
        ldx     #OSLINES
OSLP:   sta     WSYNC
        dex
        bne     OSLP
        jmp     DLOOP
