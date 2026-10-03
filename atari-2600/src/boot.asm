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
; browser's old inline boot waited on FN_R_BOOT_STATE with the display frozen,
; and a large image over TNFS is many seconds of a console that looks hung.

        CPU     6502
        INCLUDE "vcs.inc"

PAD3    EQU     $81             ; the display kernel's 3-cycle pad target
SAVSP   EQU     $82

        INCLUDE "fujinet.inc"
        INCLUDE "cfgdefs.inc"

FNLSWAP EQU     1               ; this is the bank that hands the console over

BTROWP  EQU     2               ; the path being booted
BTROWB  EQU     5               ; the progress bar
BTROWE  EQU     18              ; the error line
BTROWH  EQU     20              ; "DO NOT POWER OFF"
BTFAILW EQU     180             ; frames to hold a failure before going back

        ORG     $1000

START:  lda     #2
        sta     VBLANK
        lda     #0
        sta     CFERR
        sta     CFSTEP
        sta     CFMOPN
        sta     CFN             ; last percentage drawn

        jsr     DRAW

        ; Both transactions happen with the screen still blanked: each is a
        ; round trip and neither will finish inside a frame.
        jsr     SDFP
        lda     CFERR
        bne     BTBAD
        jsr     MIMG
        lda     CFERR
        bne     BTBAD

        lda     #BT_GO
        sta     CFSUB
        lda     #0
        sta     VBLANK
        jsr     DINIT
        jmp     DLOOP

BTBAD:  lda     #BT_FAIL
        sta     CFSUB
        lda     #BTFAILW
        sta     CFTMR
        jsr     SHOWERR
        lda     #0
        sta     VBLANK
        jsr     DINIT
        jmp     DLOOP

; ---------------------------------------------------------------------------
; APPVBL -- watch the push. FN_R_BOOT_PCT is painted by the cartridge as the
; image arrives, so the bar costs no transaction at all.
APPVBL: lda     CFSUB
        cmp     #BT_FAIL
        beq     AVFAIL

        lda     FNBST
        cmp     #FNBRDY
        beq     AVGO
        cmp     #FNBFAIL
        beq     AVDIED

        lda     FNBPC           ; only redraw when it actually moved
        cmp     CFN
        beq     AVDONE
        sta     CFN
        jmp     BAR

AVGO:   jsr     FNBLK
        jmp     FNSWAP          ; does not return

AVDIED: lda     FNBER
        sta     CFERR
        lda     #6
        sta     CFSTEP
        lda     #BT_FAIL
        sta     CFSUB
        lda     #BTFAILW
        sta     CFTMR
        jsr     TFAILR
        jsr     SHOWERR
AVDONE: rts

; A failure is held on screen long enough to read, then the browser gets its
; directory back -- FP_POP drops the filename this bank was given.
AVFAIL: dec     CFTMR
        bne     AVDONE
        jsr     FNWPOP
        lda     #BANKDIR
        jmp     CFGOTO          ; does not return

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

; MIMG -- MOUNT_IMAGE. This is what starts the push.
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
        lda     #5
        sta     CFTMPA
        ; fall through

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

SHOWERR: lda    #BTROWE
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

        INCLUDE "ui.inc"
        INCLUDE "fujilib.inc"
        INCLUDE "fujidisp.inc"

        END
