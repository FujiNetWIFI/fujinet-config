; edit.asm -- CONFIG bank 3: the on-screen keyboard.
;
; Four things get typed on this console: a custom SSID, a WiFi passphrase, a
; host name, and a directory filter. None of them fits in console RAM -- a
; passphrase alone is 63 characters and there are about forty bytes free -- so
; THE STRING BEING TYPED LIVES IN THE CARTRIDGE, in path buffer 3, and this
; bank never holds more than the cursor.
;
; That is also why the value row is painted with FN_BLIT_PATH rather than
; composed here: the page the characters are written through is write-only, so
; the only way to see them again is to ask the cartridge to render them.
;
; THE CURSOR POSITION IS THE CHARACTER, as on the Intellivision. Twelve columns
; by eight rows is 96 cells and ASCII 32..127 is 96 characters, so the grid is
; exactly full with no shift key and no paging:
;
;       ch = 32 + row*12 + col,  and cell 95 ($7F) is the backspace.
;
; The Intellivision uses 6 rows of 16 for the same trick. Sixteen columns do
; not exist here, and the transpose is what makes the same idea fit.
;
; There is no inverse video on this display -- vcs_render_row() takes no
; attribute -- so the cursor cannot be a highlight. The row is shown by a '^'
; on the blank line beneath it, which is why the grid is double-spaced. The
; spacing is not a cost: 4x6 cells are far easier to read with a gap between
; the rows than without one.
;
; Entry contract. The caller sets CFEFLD (which field), CFRET (which bank to
; come back to), and has already prepared path buffer 3 -- FP_SEED to edit the
; existing value, FP_RST to start empty. On the way out this sets CFSUB to
; ED_OK or ED_NO and returns to CFRET. Accepting commits the scratch into the
; field's home buffer; cancelling does nothing at all, which is what makes
; cancel safe: the original was never touched.

        CPU     6502
        INCLUDE "vcs.inc"

PAD3    EQU     $81             ; the display kernel's 3-cycle pad target
SAVSP   EQU     $82

        INCLUDE "fujinet.inc"
        INCLUDE "cfgdefs.inc"

EDROWT  EQU     0               ; title
EDROWV  EQU     1               ; the value, tail-anchored
EDROWL  EQU     2               ; "LEN nn/mm"
EDROW0  EQU     4               ; first grid row; grid row r is EDROW0 + r*2
EDGROWS EQU     8
EDGCOLS EQU     FNTCOL          ; 12 -- the grid is exactly the screen width
EDROWH  EQU     20              ; the hint
EDDEL   EQU     $7F             ; the last cell: backspace

        ORG     $1000

START:  lda     #2
        sta     VBLANK

        lda     #0
        sta     EDCOL          ; grid column
        sta     EDROW          ; grid row
        sta     CFMOPN

        ; Everything this bank does is to buffer 3, so select it once.
        lda     #FP_SEL3
        sta     FNRSEL+FH_PATHO

        jsr     DRAW

        lda     #0
        sta     VBLANK
        jsr     DINIT
        jmp     DLOOP

; ---------------------------------------------------------------------------
APPVBL: jsr     INREPT
        sta     CFKEY

        lda     CFMOPN
        beq     AVGRID

        lda     CFKEY
        jsr     MNKEY
        sta     CFTMPB
        lda     CFMOPN
        bne     AVDONE
        lda     CFTMPB
        cmp     #MNNONE
        beq     AVMRED
        jsr     MNDISP          ; ACCEPT and CANCEL do not return
AVMRED: jsr     DRAW
        rts

AVGRID: lda     CFKEY
        and     #IN_RIGHT
        beq     AVG1
        lda     EDCOL
        cmp     #EDGCOLS-1
        bcs     AVDONE
        inc     EDCOL
        jmp     AVCAR

AVG1:   lda     CFKEY
        and     #IN_LEFT
        beq     AVG2
        lda     EDCOL
        beq     AVDONE
        dec     EDCOL
        jmp     AVCAR

AVG2:   lda     CFKEY
        and     #IN_DOWN
        beq     AVG3
        lda     EDROW
        cmp     #EDGROWS-1
        bcs     AVDONE
        jsr     EDCCLR          ; blank the caret row we are leaving
        inc     EDROW
        jmp     AVCAR

AVG3:   lda     CFKEY
        and     #IN_UP
        beq     AVG4
        lda     EDROW
        beq     AVDONE
        jsr     EDCCLR
        dec     EDROW
        jmp     AVCAR

AVG4:   lda     CFKEY
        and     #IN_SEL
        beq     AVG5
        jsr     MNOPEN
        rts

AVG5:   lda     CFKEY
        and     #IN_FIRE
        beq     AVDONE
        jsr     EDTYPE
AVDONE: rts

; Only the caret row moves, so only the caret row is redrawn. The grid itself
; never changes, which is the whole point of deriving it from the cursor.
AVCAR:  jsr     EDCARE
        rts

; ---------------------------------------------------------------------------
; EDCH -- the character under the cursor, in A.
; No temporary: row*12 is (row*3)*4, which is two adds and two shifts, and
; EDGRID walks the grid by moving the cursor variables and so needs both
; scratch bytes for its own loop.
EDCH:   lda     EDROW
        asl     a               ; row * 2
        clc
        adc     EDROW          ; row * 3
        asl     a               ; row * 6
        asl     a               ; row * 12  -- at most 84
        clc
        adc     EDCOL
        clc
        adc     #32
        rts

; ---------------------------------------------------------------------------
; EDTYPE -- FIRE. Append the character, or back one up.
EDTYPE: jsr     EDCH
        cmp     #EDDEL
        beq     EDBS
        sta     CFTMPA          ; the character to append

        ldy     CFEFLD
        lda     EDMAXT,y
        cmp     CFELEN
        beq     EDT9            ; full: refuse rather than silently drop it
        bcc     EDT9
        lda     CFTMPA
        sta     FNRSEL+FH_PATHC
        inc     CFELEN
        jmp     EDT8

EDBS:   lda     CFELEN
        beq     EDT9            ; already empty; bottoming out is ordinary
        dec     CFELEN
        lda     #FP_POPCH
        sta     FNRSEL+FH_PATHO

EDT8:   jsr     EDVAL
        jsr     EDLEN
EDT9:   rts

; ---------------------------------------------------------------------------
; EDVAL -- repaint the value row from the cartridge's own buffer.
;
; Tail-anchored: src is the length less one screenful, floored at zero, so a
; 63-character passphrase shows its end and a short one sits left-aligned.
; FN_BLIT_PATH space-fills past the end, so a value that just got shorter
; leaves no debris.
EDVAL:  lda     FNPLNL
        sec
        sbc     #EDGCOLS
        bcs     EDV1
        lda     #0
EDV1:   sta     FNRSEL+FH_BLTSL
        lda     #0
        sta     FNRSEL+FH_BLTSH
        lda     #EDROWV
        sta     FNRSEL+FH_BLTDL
        lda     #0
        sta     FNRSEL+FH_BLTDH
        lda     #EDGCOLS
        sta     FNRSEL+FH_BLTCN
        lda     #FB_PATH
        sta     FNRSEL+FH_BLTGO
        rts

; ---------------------------------------------------------------------------
; EDLEN -- "LEN nn/mm", so a field that is full says why it stopped taking
; characters instead of just ignoring the button.
EDLEN:  lda     #EDROWL
        jsr     FNROWA
        ldy     #0
EDL1:   lda     TLEN,y
        beq     EDL2
        sta     FNRSEL+FH_TCHR
        iny
        bne     EDL1
EDL2:   lda     CFELEN
        jsr     EDDEC
        lda     #'/'
        sta     FNRSEL+FH_TCHR
        ldy     CFEFLD
        lda     EDMAXT,y
        jsr     EDDEC
        jsr     FNENDR
        rts

; EDDEC -- A as up to two decimal digits, appended to the row being composed.
EDDEC:  ldx     #0
EDD1:   cmp     #10
        bcc     EDD2
        sec
        sbc     #10
        inx
        jmp     EDD1
EDD2:   pha
        txa
        beq     EDD3            ; no leading zero
        clc
        adc     #'0'
        sta     FNRSEL+FH_TCHR
EDD3:   pla
        clc
        adc     #'0'
        sta     FNRSEL+FH_TCHR
        rts

; ---------------------------------------------------------------------------
; EDGRID -- the eight grid rows. Drawn once; the cursor never changes them.
; EDCH derives the character from the cursor, so the cheapest way to walk the
; grid is to move the cursor over it and put it back afterwards. DRAW is also
; called when the action menu closes, where the cursor must survive.
EDGRID: lda     EDCOL
        sta     CFTMPA
        lda     EDROW
        sta     CFTMPB

        lda     #0
        sta     EDROW
EDG1:   lda     EDROW
        asl     a
        clc
        adc     #EDROW0
        jsr     FNROWA

        lda     #0
        sta     EDCOL
EDG2:   jsr     EDCH
        cmp     #EDDEL
        bne     EDG3
        lda     #'<'            ; the backspace cell, drawn as what it does
EDG3:   sta     FNRSEL+FH_TCHR
        inc     EDCOL
        lda     EDCOL
        cmp     #EDGCOLS
        bcc     EDG2

        jsr     FNENDR
        inc     EDROW
        lda     EDROW
        cmp     #EDGROWS
        bcc     EDG1

        lda     CFTMPA
        sta     EDCOL
        lda     CFTMPB
        sta     EDROW
        rts

; ---------------------------------------------------------------------------
; EDCARE -- draw the caret under the cursor's column.
; EDCCLR -- blank the caret row the cursor is leaving.
EDCARE: jsr     EDCROW
        ldy     #0
EDC1:   cpy     EDCOL
        bne     EDC2
        lda     #'^'
        bne     EDC3            ; always taken
EDC2:   lda     #' '
EDC3:   sta     FNRSEL+FH_TCHR
        iny
        cpy     #EDGCOLS
        bcc     EDC1
        jmp     FNENDR

EDCCLR: jsr     EDCROW
        jmp     FNENDR          ; an empty row composes as all spaces

; EDCROW -- begin the caret row belonging to the cursor's grid row.
EDCROW: lda     EDROW
        asl     a
        clc
        adc     #EDROW0+1
        jmp     FNROWA

; ---------------------------------------------------------------------------
DRAW:   jsr     FNCLS
        ldy     CFEFLD
        lda     EDTTLL,y
        sta     FNPTRL
        lda     EDTTLH,y
        sta     FNPTRH
        lda     #EDROWT
        jsr     FNRSTR

        jsr     EDGRID
        jsr     EDCARE
        jsr     EDVAL
        jsr     EDLEN

        lda     #(THINT)&$FF
        sta     FNPTRL
        lda     #(THINT)>>8
        sta     FNPTRH
        lda     #EDROWH
        jmp     FNRSTR

; ---------------------------------------------------------------------------
; MNDISP -- the menu. Only three things can happen to an edit.
MNDISP: cmp     #0
        bne     MNDS1

        ; ACCEPT. Commit the scratch into the field's home buffer, if it has
        ; one; a host name has none and is streamed straight out of the
        ; scratch by the bank that asked for it.
        ldy     CFEFLD
        lda     EDHOME,y
        cmp     #$FF
        beq     MNDSA
        sta     FNRSEL+FH_PATHO ; select the home buffer
        lda     #FP_CMIT
        sta     FNRSEL+FH_PATHO
MNDSA:  lda     #ED_OK
        sta     CFSUB
        lda     CFRET
        jmp     CFGOTO          ; does not return

MNDS1:  cmp     #1
        bne     MNDS2
        ; CANCEL. Nothing to undo: the original was never written to.
        lda     #ED_NO
        sta     CFSUB
        lda     CFRET
        jmp     CFGOTO          ; does not return

MNDS2:  ; CLEAR
        lda     #FP_RST
        sta     FNRSEL+FH_PATHO
        lda     #0
        sta     CFELEN
        rts

MNTITL: DB      "ACTIONS",0
MNCNT   EQU     3
MNTBL:  DB      "ACCEPT",0
        DS      16-7
        DB      "CANCEL",0
        DS      16-7
        DB      "CLEAR",0
        DS      16-6

; Field -> maximum characters. One less than the buffer the server reads, so
; there is always room for the NUL the payload is padded with.
EDMAXT: DB      32              ; ED_SSID: MAX_SSID_LEN
        DB      63              ; ED_PASS: MAX_WIFI_PASS_LEN - 1
        DB      31              ; ED_HOST: MAX_HOSTNAME_LEN - 1
        DB      20              ; ED_FILT

; Field -> the path op that selects its home buffer, or $FF for "no home".
EDHOME: DB      FP_SEL0         ; ED_SSID
        DB      FP_SEL1         ; ED_PASS
        DB      $FF             ; ED_HOST
        DB      FP_SEL1         ; ED_FILT

EDTTLL: DB      (TSSID)&$FF, (TPASS)&$FF, (THOST)&$FF, (TFILT)&$FF
EDTTLH: DB      (TSSID)>>8, (TPASS)>>8, (THOST)>>8, (TFILT)>>8

TSSID:  DB      "ENTER SSID",0
TPASS:  DB      "PASSWORD",0
THOST:  DB      "HOST NAME",0
TFILT:  DB      "FILTER",0
TLEN:   DB      "LEN ",0
THINT:  DB      "FIRE SEL=OK",0

        INCLUDE "menu.inc"
        INCLUDE "fujilib.inc"
        INCLUDE "fujidisp.inc"

        END
