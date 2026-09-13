; ui.inc -- list helpers shared by the screens that have a cursor.
;
; There is no inverse video on this display: vcs_render_row() takes no
; attribute, so a selected row cannot be highlighted. Every list here marks
; its selection with a '>' in column 0 instead, exactly as the Channel F port
; does and for the same reason -- the text planes hold packed glyph pairs and
; cannot be read back as characters.
;
; Moving that '>' changes two characters. Recomposing the two rows to do it
; means HAVING their text, and on the WiFi screen a row's text arrives one
; GET_SCAN_RESULT at a time -- so every cursor step would cost two round
; trips, and a held stick would queue them faster than they could drain. The
; cartridge's FN_BLIT_TCELL pokes a single cell instead, which needs no text
; and no transaction at all.

; ---------------------------------------------------------------------------
; UICELL -- put character A at row X, column Y.
UICELL: sta     FNRSEL+FH_BLTSL         ; the character
        lda     #0
        sta     FNRSEL+FH_BLTSH

        txa                             ; dst = row * FNTCOL + col
        asl     a                       ; row * 2
        sta     CFTMPA
        txa
        clc
        adc     CFTMPA                  ; row * 3
        asl     a                       ; row * 6
        asl     a                       ; row * 12
        sty     CFTMPA
        clc
        adc     CFTMPA
        sta     FNRSEL+FH_BLTDL
        lda     #0
        sta     FNRSEL+FH_BLTDH
        sta     FNRSEL+FH_BLTCN         ; unused by this transform
        lda     #FB_TCELL
        sta     FNRSEL+FH_BLTGO
        rts

; ---------------------------------------------------------------------------
; UIGUT -- move a list's cursor gutter: blank column 0 of row A, then mark
; column 0 of row X. Four stores each and not one byte of the rows' text.
UIGUT:  stx     CFTMPB          ; the row to mark
        tax                     ; the row to clear
        ldy     #0
        lda     #' '
        jsr     UICELL          ; clobbers CFTMPA, never CFTMPB
        ldx     CFTMPB
        ldy     #0
        lda     #'>'
        jmp     UICELL
