; boot.asm -- CONFIG bank 5: mount the image and hand the console over.
;
; Entered with the cartridge's path buffer 0 holding the FULL PATH of the file
; to boot -- the browser appends the filename to the working directory before
; it crosses, the lobby writes a literal -- so this bank takes no argument and
; touches no console RAM to get one. That is also what makes failure cheap:
; FP_POP drops the filename again and the browser is back in its directory.
;
; What happens on success is worth stating plainly: MOUNT_IMAGE makes the
; server push the ROM to the cartridge, and FNSWAP then replaces every byte of
; the 4K window with it. The swap stub runs from console RAM because the code
; that triggered the swap is not there afterwards. Nothing returns.
;
; The progress bar is the reason this is a screen rather than a spin loop. The
; firmware pushes the whole image BEFORE it answers MOUNT_IMAGE, so the mount
; is launched and watched a frame at a time, never waited for.
;
; The bar is drawn twice. The playfield band behind it (BTPF, fujidisp.inc's
; FNLBAR) is the 6507's own, so it moves on any cartridge. The '#' cells are
; cell blits, which the RP2040 defers until the transaction ends.

        CPU     6502
        INCLUDE "vcs.inc"

        INCLUDE "fujinet.inc"
        INCLUDE "cfgdefs.inc"

FNLSWAP EQU     1               ; this is the bank that hands the console over
FNLWTX  EQU     1
FNLSEND EQU     1               ; MOUNT_IMAGE is launched, not waited for
FNLBAR  EQU     1               ; the kernel draws the progress band

BTROWP  EQU     2               ; the path being booted
BTROWB  EQU     5               ; the progress bar
BTROWE  EQU     18              ; the error line
BTROWH  EQU     20              ; "DO NOT POWER OFF"
BTFAILW EQU     180             ; frames to hold a failure before going back
BTWAITS EQU     75              ; seconds without progress before giving up:
                                ;   past the cart's own 60s, so it is the
                                ;   cart's timeout that normally shows
BTBARY  EQU     TCELLH*BTROWB   ; the bar's row, as the kernel's BARY counts
CFCELL  EQU     CFMENU          ; bar cells drawn: one tick per new one. This
                                ;   bank has no menu.

        ORG     $1000

START:  lda     #2
        sta     VBLANK
        lda     #0
        sta     CFERR
        sta     CFSTEP
        sta     CFMOPN
        sta     CFN             ; last percentage drawn
        sta     CFCELL
        lda     #CUEUP          ; it plays on through the round trip
        jsr     SNDCUE

        jsr     DRAW            ; the empty bar, which sets BTPF

        ; SET_DEVICE_FULLPATH is one short round trip, taken with the screen
        ; still blanked. MOUNT_IMAGE is the whole push, so it is launched and
        ; APPVBL watches it.
        jsr     SDFP
        lda     CFERR
        bne     BTBAD
        jsr     MIMG
        lda     #BT_WAIT
        sta     CFSUB
        jsr     WDRST
        jmp     BTRUN

BTBAD:  jsr     BTDIE
        jsr     SHOWERR
BTRUN:  lda     #0
        sta     VBLANK
        jsr     DINIT
        jmp     DLOOP

; ---------------------------------------------------------------------------
; APPVBL -- watch the push. FN_R_BOOT_PCT is painted by the cartridge as the
; image arrives, so the bar costs no transaction at all.
APPVBL: lda     CFSUB
        cmp     #BT_WAIT
        beq     AVWAIT
        cmp     #BT_FAIL
        beq     AVFAIL
        cmp     #BT_JING
        beq     AVJING

; BT_GO: the answer came after the whole push, so the boot state is final.
AVJUDG: lda     FNBST
        cmp     #FNBRDY
        beq     AVGO
        cmp     #FNBFAIL
        beq     AVDIED

; Answered with nothing pushed: the server mounted the file as a disk image.
        jsr     BTDIE
        jsr     SNDERR
        lda     #(TNOROM)&$FF
        sta     FNPTRL
        lda     #(TNOROM)>>8
        sta     FNPTRH
        lda     #BTROWE
        jmp     FNRSTR

AVGO:   ldy     #FNTCOL         ; the whole bar, while the cue plays
        jsr     BAR2
        jsr     FNBLK           ; a cue ahead of the swap: the cart arms it
                                ;   asynchronously, after this store
        lda     #BT_JING
        sta     CFSUB
        lda     #CUEOK
        jmp     SNDCUE

; The cue is let finish before the swap, which silences the TIA and leaves
; the sound to the game.
AVJING: lda     SNDVOL
        bne     AVDONE
        jmp     FNSWAP          ; does not return

AVDIED: lda     FNBER
        sta     CFERR
        lda     #6
        sta     CFSTEP
        jsr     BTDIE
        jsr     SHOWERR
AVDONE: rts

; BTDIE -- the failure screen's state and title; the caller says why.
BTDIE:  lda     #BT_FAIL
        sta     CFSUB
        lda     #BTFAILW
        sta     CFTMR
        jmp     TFAILR

; A failure is held on screen long enough to read, then the browser gets its
; directory back -- FP_POP drops the filename this bank was given.
AVFAIL: dec     CFTMR
        bne     AVDONE
        jsr     FNWPOP
        lda     #BANKDIR
        jmp     CFGOTO          ; does not return

; BT_WAIT: until FNACKS matches, no register write. A READY or FAILED seen
; now may be left from an earlier boot; only a transfer's percent is ours.
AVWAIT: lda     FNACKS
        cmp     FNSEQ
        beq     AVREP
        lda     FNBST
        cmp     #FNBXFR
        bne     AVDOG
        lda     FNBPC           ; only redraw when it actually moved
        cmp     CFN
        beq     AVDOG
        sta     CFN
        jsr     WDRST
        jmp     BAR

; No answer and no progress for BTWAITS seconds.
AVDOG:  dec     CFTMR
        bne     AVDONE
        lda     #60
        sta     CFTMR
        dec     CFMLEN
        bne     AVDONE
        lda     #FNEWAIT
        bne     AVMERR          ; always

; The answer, judged as FINISH judges one.
AVREP:  lda     FNERR
        bne     AVMERR
        jsr     FNACK
        bne     AVMERR
        lda     #BT_GO
        sta     CFSUB
        jmp     AVJUDG

AVMERR: sta     CFERR
        lda     #5
        sta     CFSTEP
        jsr     BTDIE
        jmp     SHOWERR

; WDRST -- restart the no-progress watchdog.
WDRST:  lda     #60
        sta     CFTMR
        lda     #BTWAITS
        sta     CFMLEN          ; this bank has no menu
        rts

; ---------------------------------------------------------------------------
; BAR -- a twelve-cell bar, one cell per whole 1/12th, poked a cell at a time
; so a moving bar never recomposes a row.
BAR:    lda     CFN
        cmp     #100
        bcc     BAR1
        lda     #99
BAR1:   ldy     #0              ; cells filled = pct * 12 / 100
BARL:   cmp     #9
        bcc     BAR2
        sec
        sbc     #9
        iny
        cpy     #FNTCOL
        bcc     BARL
BAR2:   sty     CFTMPB
        lda     BANDTB,y
        sta     BTPF
        cpy     CFCELL
        beq     BAR3
        sty     CFCELL
        lda     #CUETIK
        jsr     SNDCUE
BAR3:
        ldy     #0
BARD:   tya
        cmp     CFTMPB
        bcs     BARE
        lda     #'#'
        bne     BARF            ; always taken
BARE:   lda     #'.'
BARF:   ldx     #BTROWB
        jsr     UICELL
        iny
        cpy     #FNTCOL
        bcc     BARD
        rts

; ---------------------------------------------------------------------------
; SDFP -- SET_DEVICE_FULLPATH(slot, host, mode, <the full path>). One store
; emits all 256 payload bytes from the cartridge's own buffer.
SDFP:   lda     #FNDEVF
        sta     FNDEV
        lda     #FNCSDFP
        sta     FNCMD
        lda     #3
        sta     FNNPR
        jsr     FNBEG
        lda     #DEVSLOT
        jsr     FNPB
        lda     CFHOST
        jsr     FNPB
        lda     #FMREAD
        jsr     FNPB
        jsr     FNWTX
        lda     #4
        sta     CFTMPA
        jmp     FINISH

; MIMG -- launch MOUNT_IMAGE. This is what starts the push.
MIMG:   lda     #FNDEVF
        sta     FNDEV
        lda     #FNCMIMG
        sta     FNCMD
        lda     #2
        sta     FNNPR
        jsr     FNBEG
        lda     #DEVSLOT
        jsr     FNPB
        lda     #FMREAD
        jsr     FNPB
        jmp     FNSEND

FINISH: jsr     FNGO
        sta     CFERR
        cmp     #FNEOK
        bne     FINE
        jsr     FNACK
        sta     CFERR
        cmp     #FNEOK
        beq     FINOK
FINE:   lda     CFTMPA
        sta     CFSTEP
FINOK:  rts

; ---------------------------------------------------------------------------
DRAW:   jsr     FNCLS
        lda     #(TTITLE)&$FF
        sta     FNPTRL
        lda     #(TTITLE)>>8
        sta     FNPTRH
        lda     #0
        jsr     FNRSTR

; The path, tail-anchored: the interesting end of "/games/atari/thing.bin" is
; the right-hand end, and there are twelve columns.
        lda     #FP_SEL0
        sta     FNRSEL+FH_PATHO
        lda     FNPLNL
        sec
        sbc     #FNTCOL
        bcs     DRW1
        lda     #0
DRW1:   sta     FNRSEL+FH_BLTSL
        lda     #0
        sta     FNRSEL+FH_BLTSH
        lda     #BTROWP
        sta     FNRSEL+FH_BLTDL
        lda     #0
        sta     FNRSEL+FH_BLTDH
        lda     #FNTCOL
        sta     FNRSEL+FH_BLTCN
        lda     #FB_PATH
        sta     FNRSEL+FH_BLTGO

        lda     #0
        sta     CFN
        jsr     BAR

        lda     #(THINT)&$FF
        sta     FNPTRL
        lda     #(THINT)>>8
        sta     FNPTRH
        lda     #BTROWH
        jmp     FNRSTR

TFAILR: lda     #(TFAILT)&$FF
        sta     FNPTRL
        lda     #(TFAILT)>>8
        sta     FNPTRH
        lda     #0
        jmp     FNRSTR

SHOWERR: jsr    SNDERR
        lda     #BTROWE
        jsr     FNROWA
        lda     #'E'
        sta     FNRSEL+FH_TCHR
        lda     CFSTEP
        jsr     FNHEX
        lda     #' '
        sta     FNRSEL+FH_TCHR
        lda     CFERR
        jsr     FNHEX
        jmp     FNENDR

TTITLE: DB      "BOOTING",0
TFAILT: DB      "BOOT FAILED",0
THINT:  DB      "DO NOT POWER",0
TNOROM: DB      "NOT A ROM",0

; PF2 for each count of '#' cells: the band closes in from both sides. Bit 0
; is the frame's own side, so an empty bar is the frame.
BANDTB: DB      $01,$01,$03,$03,$07,$07,$0F,$1F,$1F,$3F,$3F,$7F,$FF
        IF      *-BANDTB<>FNTCOL+1
        FATAL   "BANDTB is not one byte per cell count"
        ENDIF

; Row colours for the kernel: the path, and the bar in green.
RCOLTB: DB      COLTTL,COLTXT,COLPTH,COLTXT,COLTXT,COLHI
        REPT    BTROWE-BTROWB-1
        DB      COLTXT
        ENDM
        DB      COLERR,COLTXT,COLHNT
        IF      *-RCOLTB<>TROWS
        FATAL   "RCOLTB is not one byte per row"
        ENDIF

        INCLUDE "ui.inc"
        INCLUDE "fujilib.inc"
        INCLUDE "fujidisp.inc"

        END
