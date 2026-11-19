; menu.asm -- CONFIG bank 6: the SELECT action menu, for the hosts, browser and
; keyboard screens.
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
; It is a bank of its own so that no screen carries the menu's code. A screen
; sets CFSUB to its MN_ number and switches here. The menu is an OVERLAY: the
; text planes are the cartridge's and survive the switch, so this paints over
; rows MNROW0-1 .. MNROW0+MNMAX-1 and leaves the rest as the screen drew it.
; Choosing or cancelling sets CFF_MENU, puts the item (or MNNONE) in CFSUB and
; switches back; the screen acts on it and repaints, because only it knows
; what was underneath.
;
; SELECT closes without choosing, so the same switch that opens the menu also
; backs out of it -- there is no other button to spare for "never mind".

        CPU     6502
        INCLUDE "vcs.inc"

        INCLUDE "fujinet.inc"
        INCLUDE "cfgdefs.inc"

MNROW0  EQU     12              ; first item row; the title sits above it
MNMAX   EQU     6               ; rows reserved, so a short menu still clears

        ORG     $1000

START:  lda     #2
        sta     VBLANK
        ldx     CFSUB
        lda     MNCNTT,x
        sta     CFMLEN
        lda     #0
        sta     CFMENU
        lda     #1
        sta     CFMOPN
        jsr     SNDMNU
        jsr     MNDRAW
        lda     #0
        sta     VBLANK
        jsr     DINIT
        jmp     DLOOP

; ---------------------------------------------------------------------------
APPVBL: jsr     INREPT
        jsr     MNKEY
        sta     CFTMPB
        lda     CFMOPN
        bne     AVDONE
        ldx     CFSUB           ; closed: back to the screen that asked
        lda     MNBANK,x
        sta     CFTMPA
        lda     CFTMPB
        sta     CFSUB
        lda     CFFLAG
        ora     #CFF_MENU
        sta     CFFLAG
        lda     CFTMPA
        jmp     CFGOTO          ; does not return
AVDONE: rts

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

        ldx     CFSUB
        ldy     MNSTRT,x        ; the table offset; FNROWA and FNENDR leave Y alone
        ldx     #0
MND1:   txa
        clc
        adc     #MNROW0
        jsr     FNROWA

        cpx     CFMLEN
        bcs     MND5            ; past the last item: leave the row blank

        cpx     CFMENU
        bne     MND2
        lda     #'>'
        bne     MND3            ; always taken
MND2:   lda     #' '
MND3:   sta     FNRSEL+FH_TCHR

MND4:   lda     MNTBL,y
        beq     MND45
        sta     FNRSEL+FH_TCHR
        iny
        bne     MND4
MND45:  iny                     ; past the NUL: the next item

MND5:   jsr     FNENDR
        inx
        cpx     #MNMAX
        bcc     MND1
        rts

; ---------------------------------------------------------------------------
; MNKEY -- feed the menu this frame's newly-pressed mask, in A.
;
; Returns the chosen item index, or MNNONE if nothing was chosen. Choosing or
; cancelling closes the menu (CFMOPN = 0).
MNKEY:  sta     CFTMPA
        and     #IN_DOWN
        beq     MNK1
        lda     CFMENU
        clc
        adc     #1
        cmp     CFMLEN
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

; ---------------------------------------------------------------------------
; The menus, by MN_ number: the bank to go back to, how many items, and where
; in MNTBL the labels start. Eleven usable columns, because column zero is the
; cursor gutter.
MNBANK: DB      BANKHST,BANKDIR,BANKEDT
MNCNTT: DB      4,5,3
MNSTRT: DB      0,MNB-MNTBL,MNE-MNTBL

MNTITL: DB      "ACTIONS",0
MNTBL:  DB      "INFO",0
        DB      "RENAME",0
        DB      "WIFI",0
        DB      "LOBBY",0
MNB:    DB      "UP DIR",0
        DB      "FILTER",0
        DB      "INFO",0
        DB      "COPY",0
        DB      "COPY HERE",0
MNE:    DB      "ACCEPT",0
        DB      "CANCEL",0
        DB      "CLEAR",0

; Row colours for the kernel: whatever the screen underneath had, this is the
; common denominator.
        RCOLS   ROWERR,ROWERR+2

        INCLUDE "fujidisp.inc"
        INCLUDE "fujilib.inc"

        END
