; menu.inc -- the SELECT action menu, included by every bank that has one.
;
; The Intellivision reaches info, the lobby, rename, filter, copy, up-a-level
; and cancel with a twelve-key keypad. This console has a joystick, one
; button, two console switches, and twelve columns to name anything in.
;
; A chord or a long press would fit in that budget. Neither PRINTS ITSELF, and
; on a console with no manual an undiscoverable binding is the same as no
; binding at all -- so SELECT opens a list instead, and every action arrives
; with its own name next to it.
;
; The menu is an OVERLAY: it paints over rows MNROW0-1 .. MNROW0+MNMAX-1 and
; the bank redraws its own screen when the menu closes. There is nothing to
; save and restore, because the text planes cannot be read back as characters
; anyway -- the same fact that makes the cursor a '>' rather than an inverse
; bar.
;
; Each bank supplies:
;   MNTBL   a table of labels, SIXTEEN bytes per entry, NUL-terminated within
;   MNCNT   how many of them
; and dispatches on the index this returns. Sixteen rather than a packed
; stride so the index is four shifts instead of a multiply; the banks that
; have a menu all have several hundred bytes free, and none of them has
; several hundred spare cycles.

MNROW0  EQU     12              ; first item row; the title sits above it
MNMAX   EQU     6               ; rows reserved, so a short menu still clears
MNNONE  EQU     $FF             ; "nothing chosen yet"

; ---------------------------------------------------------------------------
; MNOPEN -- start showing the menu at item 0.
MNOPEN: lda     #0
        sta     CFMENU
        lda     #1
        sta     CFMOPN
        ; fall through

; ---------------------------------------------------------------------------
; MNDRAW -- paint the title, the items, and blank filler down to MNMAX.
MNDRAW: lda     #MNROW0-1
        jsr     FNROWA
        ldy     #0
MNDT1:  lda     MNTITL,y
        beq     MNDT2
        sta     FNRSEL+FH_TCHR
        iny
        bne     MNDT1
MNDT2:  jsr     FNENDR

        ldx     #0
MND1:   txa
        clc
        adc     #MNROW0
        jsr     FNROWA

        cpx     MNCNT
        bcs     MND5            ; past the last item: leave the row blank

        cpx     CFMENU
        bne     MND2
        lda     #'>'
        bne     MND3            ; always taken
MND2:   lda     #' '
MND3:   sta     FNRSEL+FH_TCHR

        txa
        asl     a
        asl     a
        asl     a
        asl     a               ; item * 16
        tay
MND4:   lda     MNTBL,y
        beq     MND5
        sta     FNRSEL+FH_TCHR
        iny
        bne     MND4

MND5:   jsr     FNENDR
        inx
        cpx     #MNMAX
        bcc     MND1
        rts

; ---------------------------------------------------------------------------
; MNKEY -- feed the menu this frame's newly-pressed mask, in A.
;
; Returns the chosen item index, or MNNONE if nothing was chosen. Choosing or
; cancelling closes the menu (CFMOPN = 0); the CALLER then redraws its screen,
; because only the caller knows what was underneath.
;
; SELECT closes without choosing, so the same switch that opens the menu also
; backs out of it -- there is no other button to spare for "never mind".
MNKEY:  sta     CFTMPA
        and     #IN_DOWN
        beq     MNK1
        lda     CFMENU
        clc
        adc     #1
        cmp     MNCNT
        bcs     MNKNO
        sta     CFMENU
        jsr     MNDRAW
        jmp     MNKNO

MNK1:   lda     CFTMPA
        and     #IN_UP
        beq     MNK2
        lda     CFMENU
        beq     MNKNO
        sec
        sbc     #1
        sta     CFMENU
        jsr     MNDRAW
        jmp     MNKNO

MNK2:   lda     CFTMPA
        and     #IN_SEL
        beq     MNK3
        lda     #0
        sta     CFMOPN
        jmp     MNKNO           ; cancelled

MNK3:   lda     CFTMPA
        and     #IN_FIRE
        beq     MNKNO
        lda     #0
        sta     CFMOPN
        lda     CFMENU
        rts

MNKNO:  lda     #MNNONE
        rts
