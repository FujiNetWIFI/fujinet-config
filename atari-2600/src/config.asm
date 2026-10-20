; cfgtail.asm -- CONFIG's fixed half: the cold stub, the bank trampoline, and
; what every bank shares.
;
; $1800-$1F1F is the mailbox -- text planes, reply window, control page, TX
; page, status -- and the cartridge paints it. The client owns $1F20-$1FFB,
; which fuji_mailbox.h calls the fixed tail, plus the vectors.
;
; CFGOTO switches bank. The store that switches is the last instruction
; fetched from the OLD bank; the very next fetch comes from the new one. So the
; jump that follows it has to be at an address that does not move, or the
; program counter lands in whatever the new bank happens to have at that
; offset. This is the whole reason a fixed tail exists.
;
; CFCOLD is where the RESET vector points. It has to be here for the same
; reason plus one more: this console has no reset line to the cartridge, so
; the RESET switch restarts the 6507 with WHATEVER BANK WAS LAST SELECTED
; still mapped. A cold stub in bank 0 would simply not be there.
;
; The sound engine and the frame's blanking work are here because every bank
; needs them and a copy in each would cost seven times over. Their entry
; points are fixed in cfgdefs.inc, the same way CFGOTO's is.

        CPU     6502
        INCLUDE "vcs.inc"
        INCLUDE "fujinet.inc"
        INCLUDE "cfgdefs.inc"

; AT -- continue at a fixed entry point, failing the build if the routine
; before it has grown into it.
AT      MACRO   ADDR
        IF      * > ADDR
        FATAL   "the fixed tail: a routine overruns the next entry point"
        ENDIF
        ORG     ADDR
        ENDM

; CUE -- mark where a cue starts, failing the build if cfgdefs.inc disagrees.
CUE     MACRO   OFF
        IF      *-SNDTAB <> OFF
        FATAL   "SNDTAB: a cue is not at its offset in cfgdefs.inc"
        ENDIF
        ENDM

        ORG     CFGOTO

; ---------------------------------------------------------------------------
; CFGOTO -- select bank A and enter it at $1000.
;
; `sta FNRSEL+FH_BANK,x` is the documented-safe indexed form: the base low byte
; is $80 and X is a bank number, so the index cannot carry, which means the
; dummy read that STA abs,X always performs lands on the same address as the
; write. One parked access, and a bank op ignores its data anyway.
        tax
        sta     FNRSEL+FH_BANK,x
        jmp     $1000

; ---------------------------------------------------------------------------
; SNDPOL -- FNGO calls this while it spins. No frames are drawn while a
; transaction is out, so without it a cue would hold its note for the whole
; round trip. The RIOT timer stands in for the frame. Preserves Y.
        AT      SNDPOL
        bit     TIMINT
        bpl     SNDRTS
        lda     #19             ; 19 * 1024 cycles: about a frame
        sta     T1024T

; SNDTIK -- one frame of sound, on TIA channel 0. AUDV0 is written every time,
; so nothing can leave a tone running. Preserves Y.
SNDTIK: dec     SNDDUR
        bpl     SKV
        ldx     SNDPTR
        lda     SNDTAB,x
        beq     SKOFF
        sta     AUDF0           ; the TIA takes bits 0-4
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        sta     SNDDUR          ; bits 5-7: frames - 1
        inc     SNDPTR
        bne     SKV             ; always: SNDTAB is far shorter than 256
SKOFF:  sta     SNDVOL          ; the terminator: silence, parked on it
SKV:    lda     SNDVOL
        sta     AUDV0
SNDRTS: rts

; ---------------------------------------------------------------------------
; SNDERR, SNDMNU, SNDCUE -- start a cue. A new cue replaces the one playing.
; Clobbers X.
        AT      SNDERR
        lda     #CUEERR
        bne     SNDCUE          ; always
        AT      SNDMNU
        lda     #CUEMNU
        AT      SNDCUE
        tax
        lda     SNDTAB,x
        sta     AUDC0           ; the TIA takes bits 0-3
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        sta     SNDVOL          ; bits 4-7
        inx
        stx     SNDPTR
        lda     #0
        sta     SNDDUR          ; the next tick starts the first note
        rts

; ---------------------------------------------------------------------------
; SNDKEY -- the click for a newly-pressed input mask in A: a tick for the
; stick, a blip for FIRE. A is preserved, flags and all, so INREPT can end
; here.
        AT      SNDKEY
        pha
        and     #IN_FIRE|INDIRS
        beq     SK9
        cmp     #IN_FIRE
        lda     #CUETIK
        bcc     SK1
        lda     #CUEBLP
SK1:    jsr     SNDCUE
SK9:    pla
        rts

; ---------------------------------------------------------------------------
; VBTAIL -- the end of the frame's blanking, called by DLOOP after APPVBL with
; TIM64T already counting down to the first visible line.
;
; It finds the selection bar's row. The '>' in column 0 IS the selection on
; every list, so reading the bar off the planes means no screen has to tell
; the kernel anything, and an open menu's cursor wins because it is the lowest.
; '>' is the only glyph whose left column reads $40 on line 1 and $20 on line 2.
        AT      VBTAIL
        ldy     #TCELLH*17      ; the menu's last row
VBT1:   lda     TP0+1,y
        and     #$E0
        cmp     #$40
        bne     VBT2
        lda     TP0+2,y
        and     #$E0
        cmp     #$20
        beq     VBT3
VBT2:   tya
        sec
        sbc     #TCELLH
        tay
        cpy     #TCELLH*2       ; rows 0 and 1 are a title and a path
        bcs     VBT1
        ldy     #$FF            ; no row: never equal to 6 * a row
VBT3:   sty     BARY
        jsr     SNDTIK
VBT4:   bit     TIMINT
        bpl     VBT4
        sta     WSYNC
        lda     #0
        sta     VBLANK
        rts

; ---------------------------------------------------------------------------
; SNDTAB -- the cues. A cue is a header byte, AUDC | volume << 4, then notes
; of AUDF | (frames - 1) << 5, then 0. With AUDC 4, f = 15700 / (AUDF + 1).
SNDTAB: DB      0               ; idle: where SNDPTR starts, zeroed
        CUE     CUETIK
        DB      $44,$04,0       ; 3.1 kHz, one frame
        CUE     CUEBLP
        DB      $64,$31,0       ; A5, two frames
        CUE     CUEMNU
        DB      $64,$57,$51,0   ; E5 then A5
        CUE     CUEERR
        DB      $8C,$F7,$F7,0   ; AUDC 12: 218 Hz, sixteen frames
        CUE     CUEUP
        DB      $64,$3D,$37,$33,$2E,0 ; C5 E5 G5 C6, quick
        CUE     CUEOK
        DB      $84,$BD,$B7,$B3,$EE,0 ; C5 E5 G5, then a long C6

; ---------------------------------------------------------------------------
; CFCOLD -- power-on and RESET.
CFCOLD: sei
        cld
        ldx     #0              ; zero $00-$FF -- the TIA, sound included,
        txa                     ;   and all of RAM -- through the stack, and
CFCL1:  dex                     ;   leave SP at $FF
        txs
        pha
        bne     CFCL1

        lda     #2
        sta     VBLANK          ; blanked until a bank starts its display

; The gate is opened here, inline, because CFGOTO needs it: banking is a
; control-page op and the control page is dead until an ordered pair of stores
; carrying two specific values arrives. A 7800's BIOS probing cartridge space
; cannot produce that pair, which is the point of the gate.
        lda     #FNAM1
        sta     FNRSEL+FH_ARM1
        lda     #FNAM2
        sta     FNRSEL+FH_ARM2

        ; The path buffers are CARTRIDGE state and survive a console reset, so
        ; a cold start has to empty them or the browser comes back somewhere
        ; the host screen does not expect -- and a stale filter or a stale
        ; copy source would outlive the run that set them.
        ;
        ; ALL FOUR, and the SELECTION too: which buffer is live is cartridge
        ; state as well, so a reset can land here with any of them selected
        ; and an FP_RST on its own would empty whichever that happened to be.
        ; The loop ends on buffer 0, which is where every bank expects to
        ; start. FP_SEL0..FP_SEL3 are contiguous by construction.
        ldx     #FP_SEL3
CFCL2:  txa
        sta     FNRSEL+FH_PATHO
        lda     #FP_RST
        sta     FNRSEL+FH_PATHO
        dex
        cpx     #FP_SEL0
        bcs     CFCL2

; The wifi bank is the entry point, not the hosts bank: it is the one that
; decides whether there is anything to do. On a FujiNet that is already
; associated -- which is the common case -- it issues two commands and leaves
; for the hosts screen without drawing anything.
        lda     #BANKWIF
        jmp     CFGOTO

        IF      * > $1FFC
        FATAL   "the fixed tail runs into the vectors"
        ENDIF

        ORG     $1FFC
        DW      CFCOLD
        DW      CFCOLD

        END
