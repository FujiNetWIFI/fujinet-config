; copy.asm -- CONFIG bank 6: copy a file from one host to another.
;
; Three presses, spread over three screens, and the state that survives them
; is all in the cartridge:
;
;   1. FIRE on a file with COPY chosen from the browser's menu. The full path
;      goes into cartridge buffer 2 and CFCOPY becomes CP_MARK.
;   2. The hosts screen comes up in copy livery. Picking a slot mounts it and
;      opens the browser on it, using the ordinary mount path -- there is no
;      separate destination-host screen the way src/destination_host_slot.c
;      is one.
;   3. COPY HERE in the destination directory runs COPY_FILE.
;
; TWO THINGS ABOUT THE PAYLOAD.
;
; It is "srcfullpath|destdir" at EXACT length -- no NUL, no padding to 256.
; COPY_FILE is the one command that reads packet.dataAsString() instead of
; transaction_get(), so padding would bury NULs inside the destination name.
;
; And the destination is the DIRECTORY, ending in its separator. The server
; appends the source's basename itself when it sees that trailing '/'
; (fujiDevice.cpp:1117), so the client does not have to find the last
; separator in a string it cannot read. The Intellivision port does that work
; itself; here it is free.
;
; The slots are 1-BASED. fujicore_copy_file_success() rejects 0 outright and
; then decrements, so a 0-based slot number is silently the wrong host.

        CPU     6502
        INCLUDE "vcs.inc"

PAD3    EQU     $81             ; the display kernel's 3-cycle pad target
SAVSP   EQU     $82

        INCLUDE "fujinet.inc"
        INCLUDE "cfgdefs.inc"

CPROWS  EQU     2               ; the source
CPROWD  EQU     5               ; the destination
CPROWM  EQU     8               ; what happened
CPROWE  EQU     18
CPROWH  EQU     20
CPWAIT  EQU     180             ; frames to hold the result before going back

        ORG     $1000

START:  lda     #2
        sta     VBLANK
        lda     #0
        sta     CFERR
        sta     CFSTEP
        sta     CFMOPN

        jsr     DRAW
        jsr     DOCOPY

        lda     #CPWAIT
        sta     CFTMR
        lda     #CP_DONE
        sta     CFCOPY
        lda     #0
        sta     VBLANK
        jsr     DINIT
        jmp     DLOOP

; ---------------------------------------------------------------------------
; The result is held long enough to read, then the browser gets the user back
; where the copy was made. FIRE cuts the wait short.
APPVBL: jsr     INREPT
        and     #IN_FIRE
        bne     AVBACK
        dec     CFTMR
        bne     AVDONE
AVBACK: lda     #CP_IDLE
        sta     CFCOPY
        lda     #BANKDIR
        jmp     CFGOTO          ; does not return
AVDONE: rts

; ---------------------------------------------------------------------------
; DOCOPY -- COPY_FILE, then the .cfg sibling.
DOCOPY: jsr     CFILE
        lda     CFERR
        bne     CPFAIL

; The sibling. A ROM and its .cfg are one item to a person, so a copy that
; moved only half of it would arrive unbootable. Both ends are patched in
; place -- three characters at each -- which is exact because every extension
; involved is three characters long.
;
; A MISS IS NORMAL AND IS NOT A FAILURE: most files have no sibling. Only the
; first copy's result is reported.
        jsr     TOCFG
        jsr     CFILE
        lda     #(TCOPY)&$FF
        ldx     #(TCOPY)>>8
        jmp     MSG

CPFAIL: jsr     SHOWERR
        lda     #(TFAIL)&$FF
        ldx     #(TFAIL)>>8
        jmp     MSG

; TOCFG -- turn "...bin" into "...cfg" in buffer 2 by popping three characters
; and appending three. The buffer is the only place the name exists, and this
; is why FN_PATH_POPCH is a character rather than a component.
TOCFG:  lda     #FP_SEL2
        sta     FNRSEL+FH_PATHO
        ldy     #3
TCF1:   lda     #FP_POPCH
        sta     FNRSEL+FH_PATHO
        dey
        bne     TCF1
        lda     #'c'
        sta     FNRSEL+FH_PATHC
        lda     #'f'
        sta     FNRSEL+FH_PATHC
        lda     #'g'
        sta     FNRSEL+FH_PATHC
        rts

; ---------------------------------------------------------------------------
; CFILE -- COPY_FILE(src+1, dst+1, "srcfullpath|destdir").
CFILE:  lda     #FNDEVF
        sta     FNDEV
        lda     #FNCCOPY
        sta     FNCMD
        lda     #2
        sta     FNNPR
        jsr     FNBEG

        lda     CFCSRC          ; 1-BASED, both of them
        clc
        adc     #1
        jsr     FNPB
        lda     CFHOST
        clc
        adc     #1
        jsr     FNPB

        lda     #FP_SEL2        ; the source's full path, exact length
        sta     FNRSEL+FH_PATHO
        lda     #FP_TXRAW
        sta     FNRSEL+FH_PATHO
        lda     #'|'
        sta     FNTX
        lda     #FP_SEL0        ; the destination DIRECTORY, trailing '/' and
        sta     FNRSEL+FH_PATHO ;   all: the server names the file from the
        lda     #FP_TXRAW       ;   source's basename itself
        sta     FNRSEL+FH_PATHO

        lda     #7
        sta     CFTMPA
        jsr     FNGO
        sta     CFERR
        cmp     #FNEOK
        bne     CFE
        jsr     FNACK
        sta     CFERR
        cmp     #FNEOK
        beq     CFOK
CFE:    lda     CFTMPA
        sta     CFSTEP
CFOK:   rts

; ---------------------------------------------------------------------------
; MSG -- a NUL-terminated string at A/X onto the result row.
MSG:    sta     FNPTRL
        stx     FNPTRH
        lda     #CPROWM
        jmp     FNRSTR

DRAW:   jsr     FNCLS
        lda     #(TTITLE)&$FF
        sta     FNPTRL
        lda     #(TTITLE)>>8
        sta     FNPTRH
        lda     #0
        jsr     FNRSTR

        lda     #FP_SEL2        ; from
        jsr     PATHR2
        lda     #CPROWS
        jsr     PATHRW

        lda     #FP_SEL0        ; to
        jsr     PATHR2
        lda     #CPROWD
        jsr     PATHRW

        lda     #(THINT)&$FF
        sta     FNPTRL
        lda     #(THINT)>>8
        sta     FNPTRH
        lda     #CPROWH
        jmp     FNRSTR

; PATHR2 -- select buffer A and work out the tail-anchored source offset.
PATHR2: sta     FNRSEL+FH_PATHO
        lda     FNPLNL
        sec
        sbc     #FNTCOL
        bcs     PR21
        lda     #0
PR21:   sta     FNRSEL+FH_BLTSL
        lda     #0
        sta     FNRSEL+FH_BLTSH
        rts

; PATHRW -- render the selected buffer into text row A.
PATHRW: sta     FNRSEL+FH_BLTDL
        lda     #0
        sta     FNRSEL+FH_BLTDH
        lda     #FNTCOL
        sta     FNRSEL+FH_BLTCN
        lda     #FB_PATH
        sta     FNRSEL+FH_BLTGO
        rts

SHOWERR: lda    #CPROWE
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

TTITLE: DB      "COPYING",0
TCOPY:  DB      "COPIED",0
TFAIL:  DB      "COPY FAILED",0
THINT:  DB      "FIRE=BACK",0

        INCLUDE "fujilib.inc"
        INCLUDE "fujidisp.inc"

        END
