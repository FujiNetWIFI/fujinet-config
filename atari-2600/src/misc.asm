; misc.asm -- CONFIG bank 2: the two small screens, adapter info and copy.
;
; They share a bank because every bank carries its own display kernel and
; libraries, so a screen that is small still costs a whole 2K; sharing one
; frees a bank. CFSUB says which was asked for (MI_INFO, MI_COPY), set by
; whoever switches here, and stays put while the screen is up.
;
; The kernel reads ONE row colour table per bank, so both screens use the
; rows the same way: a light-blue label above a white value, an error row, a
; hint row.
;
; ---- info ------------------------------------------------------------------
; GET_ADAPTERCONFIG_EXTENDED, rendered straight out of the reply window. The
; reply is 240 bytes and the window is 512, so it lands in slice 0 whole and
; every field a client wants is below offset 256 -- which is why a single-byte
; index reaches all of them. Any button goes back to the host screen.
;
; ---- copy ------------------------------------------------------------------
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

FNLRRPL EQU     1

        INCLUDE "fujinet.inc"
        INCLUDE "cfgdefs.inc"

; Field offsets inside the packed AdapterConfigExtended, from
; lib/device/fujiDevice/fujiDevice.h.
AC_SSID EQU     0               ; char[33]
AC_HOST EQU     33              ; char[64]
AC_VER  EQU     125             ; char[15]
AC_SIP  EQU     140             ; char[16], already formatted as text
AC_SGW  EQU     156             ; char[16]
AC_SMSK EQU     172             ; char[16]
AC_SDNS EQU     188             ; char[16]
AC_SMAC EQU     204             ; char[18]

CPROWS  EQU     3               ; the source, under its label
CPROWD  EQU     5               ; the destination
CPROWM  EQU     7               ; what happened
CPROWE  EQU     18
CPWAIT  EQU     180             ; frames to hold the result before going back

        ORG     $1000

START:  lda     #2
        sta     VBLANK
        lda     #0
        sta     CFERR
        sta     CFSTEP
        sta     CFMOPN
        lda     CFSUB
        bne     CSTART
        jsr     IDRAW
        jmp     SHOW

CSTART: jsr     CDRAW
        jsr     DOCOPY
        lda     #CPWAIT
        sta     CFTMR
        lda     #CP_DONE
        sta     CFCOPY
SHOW:   lda     #0
        sta     VBLANK
        jsr     DINIT
        jmp     DLOOP

; ---------------------------------------------------------------------------
APPVBL: lda     CFSUB
        beq     IAPPVBL
        jmp     CAPPVBL
IAPPVBL: jsr     INSCAN
        jsr     SNDKEY
        and     #IN_FIRE|IN_SEL
        beq     IAVDONE
        lda     #BANKHST
        jmp     CFGOTO          ; does not return
IAVDONE: rts

; ---------------------------------------------------------------------------
IDRAW:   jsr     FNCLS
        lda     #(ITTITLE)&$FF
        sta     FNPTRL
        lda     #(ITTITLE)>>8
        sta     FNPTRH
        lda     #0
        jsr     FNRSTR

        lda     #FNDEVF
        sta     FNDEV
        lda     #FNCADPX
        sta     FNCMD
        lda     #0
        sta     FNNPR
        jsr     FNBEG
        jsr     FNGO
        sta     CFERR
        cmp     #FNEOK
        bne     IDRBAD
        jsr     FNACK
        sta     CFERR
        cmp     #FNEOK
        bne     IDRBAD

        ; Twelve columns will not hold a label and a value on one row, so each
        ; field gets a label row and a value row. Eight fields, sixteen rows,
        ; and the screen is twenty-one.
        ldx     #0              ; index into FIELDS
        ldy     #ROW0           ; the row being written
IDRF:    sty     CFWANT
        lda     FIELDS,x
        cmp     #$FF
        beq     IDRDONE
        sta     CFSEL           ; the reply offset, borrowed as scratch
        inx
        stx     CFCNT

        ; The label.
        lda     LBLLO-1,x
        sta     FNPTRL
        lda     LBLHI-1,x
        sta     FNPTRH
        lda     CFWANT
        jsr     FNRSTR

        ; The value, straight from the reply window.
        lda     CFWANT
        clc
        adc     #1
        pha
        ldx     CFSEL
        ldy     #FNTCOL
        pla
        jsr     FNRRPL

        ldx     CFCNT
        lda     CFWANT
        clc
        adc     #2
        tay
        cpy     #ROW0+18
        bcc     IDRF
IDRDONE: lda     #(ITHINT)&$FF
        sta     FNPTRL
        lda     #(ITHINT)>>8
        sta     FNPTRH
        lda     #ROWERR+1
        jsr     FNRSTR
        rts

IDRBAD:  jsr     SNDERR
        lda     #1
        sta     CFSTEP
        lda     #ROWERR
        jsr     FNROWA
        lda     #'E'
        sta     FNRSEL+FH_TCHR
        lda     CFSTEP
        jsr     FNHEX
        lda     #' '
        sta     FNRSEL+FH_TCHR
        lda     CFERR
        jsr     FNHEX
        jsr     FNENDR
        rts

; The fields, as reply offsets, terminated by $FF. Every one is below 256, so
; the offset is a byte and FNRRPL can reach it with X alone.
FIELDS: DB      AC_SSID, AC_SIP, AC_SGW, AC_SDNS, AC_SMAC, AC_VER, $FF

LBLLO:  DB      (LSSID)&$FF, (LIP)&$FF, (LGW)&$FF, (LDNS)&$FF
        DB      (LMAC)&$FF, (LVER)&$FF
LBLHI:  DB      (LSSID)>>8, (LIP)>>8, (LGW)>>8, (LDNS)>>8
        DB      (LMAC)>>8, (LVER)>>8

LSSID:  DB      "SSID",0
LIP:    DB      "IP",0
LGW:    DB      "GATEWAY",0
LDNS:   DB      "DNS",0
LMAC:   DB      "MAC",0
LVER:   DB      "VERSION",0
ITTITLE: DB      "FN INFO",0
ITHINT:  DB      "FIRE=BACK",0

; ---------------------------------------------------------------------------
; The result is held long enough to read, then the browser gets the user back
; where the copy was made. FIRE cuts the wait short.
CAPPVBL: jsr     INREPT
        and     #IN_FIRE
        bne     AVBACK
        dec     CFTMR
        bne     CAVDONE
AVBACK: lda     #CP_IDLE
        sta     CFCOPY
        lda     #BANKDIR
        jmp     CFGOTO          ; does not return
CAVDONE: rts

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
        ldy     #CPROWM
        jmp     MSG

CPFAIL: jsr     SHOWERR
        lda     #(TFAIL)&$FF
        ldx     #(TFAIL)>>8
        ldy     #CPROWM
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
; MSG -- a NUL-terminated string at A/X onto text row Y.
MSG:    sta     FNPTRL
        stx     FNPTRH
        tya
        jmp     FNRSTR

CDRAW:   jsr     FNCLS
        lda     #(CTTITLE)&$FF
        sta     FNPTRL
        lda     #(CTTITLE)>>8
        sta     FNPTRH
        lda     #0
        jsr     FNRSTR

        lda     #(TFROM)&$FF
        ldx     #(TFROM)>>8
        ldy     #CPROWS-1
        jsr     MSG
        lda     #(TTO)&$FF
        ldx     #(TTO)>>8
        ldy     #CPROWD-1
        jsr     MSG

        lda     #FP_SEL2        ; from
        jsr     PATHR2
        lda     #CPROWS
        jsr     PATHRW

        lda     #FP_SEL0        ; to
        jsr     PATHR2
        lda     #CPROWD
        jsr     PATHRW

        lda     #(CTHINT)&$FF
        sta     FNPTRL
        lda     #(CTHINT)>>8
        sta     FNPTRH
        lda     #ROWERR+1
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

SHOWERR: jsr    SNDERR
        lda     #CPROWE
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

CTTITLE: DB      "COPYING",0
TCOPY:  DB      "COPIED",0
TFAIL:  DB      "COPY FAILED",0
TFROM:  DB      "FROM",0
TTO:    DB      "TO",0
CTHINT:  DB      "FIRE=BACK",0

; Row colours for the kernel: each label above its value.
RCOLTB: DB      COLTTL,COLTXT
        REPT    6
        DB      COLPTH,COLTXT
        ENDM
        DB      COLTXT,COLTXT,COLTXT,COLTXT,COLERR,COLHNT,COLTXT
        IF      *-RCOLTB<>TROWS
        FATAL   "RCOLTB is not one byte per row"
        ENDIF

        INCLUDE "fujilib.inc"
        INCLUDE "fujidisp.inc"

        END
