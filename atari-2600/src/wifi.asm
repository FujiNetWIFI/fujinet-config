; wifi.asm -- CONFIG bank 4: WiFi.
;
; This is the bank the cold stub enters, and for most of its life it decides
; that there is nothing to do and leaves. The decision tree is the one every
; CONFIG runs (src/check_wifi.c, intv/st_wifi.bas):
;
;   GET_WIFI_ENABLED  == 0  ->  wifi is off by configuration; go to the hosts
;   GET_WIFISTATUS    == 3  ->  already associated; go to the hosts
;   GET_SSID[0]       != 0  ->  a network is stored; wait for it to come up
;   otherwise               ->  scan, pick one, type the passphrase, connect
;
; TWO THINGS ABOUT THE SCAN LIST ARE NOT OPTIONAL.
;
; The names are fetched ONE AT A TIME -- GET_SCAN_RESULT takes an index and
; returns one SSIDInfo -- so a full redraw is a dozen round trips. The
; Intellivision drew its list that way once and "the list sat frozen
; half-drawn for the better part of a second on every single press of the
; disc". So the list is drawn ONCE, and moving the cursor pokes two cells
; through FN_BLIT_TCELL instead of recomposing anything. No transaction is
; issued between the draw and the choice.
;
; And the SSID is parked in path buffer 0 the moment it is chosen, rather than
; left sitting in the reply window while the passphrase is typed. The window
; is only guaranteed stable between transactions; one stray round trip in
; between and SET_SSID would carry ninety-seven bytes of something else.
;
; Buffers 0 and 1 are the working directory and the filter everywhere else in
; this program. Here they are the SSID and the passphrase, which is safe
; because nothing is mounted yet and there is no directory to lose.

        CPU     6502
        INCLUDE "vcs.inc"

PAD3    EQU     $81             ; the display kernel's 3-cycle pad target
SAVSP   EQU     $82

        INCLUDE "fujinet.inc"
        INCLUDE "cfgdefs.inc"

WSROWE  EQU     18              ; the error line
WSROWH  EQU     20              ; the hint
WSTRIES EQU     20              ; connect polls, about two seconds apart
WSWAIT  EQU     120             ; frames between them

        ORG     $1000

START:  lda     #2
        sta     VBLANK
        lda     #0
        sta     CFERR
        sta     CFSTEP
        sta     CFMOPN

; Coming back from the keyboard. CFSUB carries the verdict and CFEFLD says
; which field it was, so this is where the SSID-then-passphrase chain is
; joined up.
        lda     CFFLAG
        and     #CFF_EDIT
        beq     STDISP

        lda     CFFLAG
        and     #$FF-CFF_EDIT
        sta     CFFLAG
        lda     CFSUB
        cmp     #ED_OK
        bne     STCAN           ; cancelled: back to the list

        lda     CFEFLD
        cmp     #ED_SSID
        bne     STPASS
        jmp     ASKPW           ; the SSID is in buffer 0; now the passphrase

STPASS: jsr     SETSSI          ; both halves are in place: save them
        lda     CFERR
        beq     STPOK
        lda     #WS_FAIL
        sta     CFSUB
        jmp     STDISP
STPOK:  lda     #0
        sta     CFN             ; poll counter
        sta     CFTMR
        lda     #WS_CONN
        sta     CFSUB
        jmp     STDISP

STCAN:  lda     #WS_SCAN
        sta     CFSUB

; ---------------------------------------------------------------------------
STDISP: lda     CFSUB
        cmp     #WS_CHECK
        bne     STD1
        jsr     DOCHK           ; may CFGOTO away and never return
        jmp     STDISP          ; it only returns having chosen a new CFSUB

STD1:   cmp     #WS_SCAN
        bne     STD2
        jsr     DOSCAN
        jmp     STDISP

STD2:   jsr     DRAW
        lda     #0
        sta     VBLANK
        jsr     DINIT
        jmp     DLOOP

; ---------------------------------------------------------------------------
APPVBL: lda     CFSUB
        cmp     #WS_CONN
        beq     AVCONN

        jsr     INREPT
        sta     CFKEY

        lda     CFSUB
        cmp     #WS_SELECT
        bne     AVDONE

        lda     CFKEY
        and     #IN_DOWN
        beq     AV1
        lda     CFSEL
        cmp     CFN             ; CFN is the OTHER row: the last one
        bcs     AVDONE
        pha
        clc
        adc     #1
        sta     CFSEL
        pla
        jsr     AVMOVE
        rts

AV1:    lda     CFKEY
        and     #IN_UP
        beq     AV2
        lda     CFSEL
        beq     AVDONE
        pha
        sec
        sbc     #1
        sta     CFSEL
        pla
        jsr     AVMOVE
        rts

AV2:    lda     CFKEY
        and     #IN_SEL
        beq     AV3
        lda     #BANKHST        ; skip wifi entirely and get on with it
        jmp     CFGOTO          ; does not return

AV3:    lda     CFKEY
        and     #IN_FIRE
        beq     AVDONE
        jmp     PICK            ; does not return
AVDONE: rts

; AVMOVE -- A is the row the cursor came FROM; CFSEL is where it went.
AVMOVE: clc
        adc     #WSROW0
        pha
        lda     CFSEL
        clc
        adc     #WSROW0
        tax
        pla
        jmp     UIGUT

; ---------------------------------------------------------------------------
; The connect poll. One GET_WIFISTATUS every WSWAIT frames, up to WSTRIES, so
; the screen keeps running between them instead of freezing for forty seconds.
AVCONN: lda     CFTMR
        beq     AVC1
        dec     CFTMR
        rts
AVC1:   lda     #WSWAIT
        sta     CFTMR

        lda     #2
        sta     VBLANK          ; a round trip will not finish inside a frame
        jsr     CWSTAT
        lda     #0
        sta     VBLANK
        lda     CFERR
        bne     AVC2
        lda     FNRPLY
        cmp     #WFASSOC
        bne     AVC2
        lda     #BANKHST
        jmp     CFGOTO          ; does not return

AVC2:   inc     CFN
        lda     CFN
        cmp     #WSTRIES
        bcc     AVC3
        lda     #WS_FAIL
        sta     CFSUB
        jsr     DRAW
AVC3:   rts

; ---------------------------------------------------------------------------
; PICK -- FIRE on the list.
PICK:   lda     CFSEL
        cmp     CFN
        beq     ASKSSI          ; the OTHER row: type a name

        ; A scanned network. Fetch it once more and park the SSID in buffer 0
        ; before anything else can repaint the reply window.
        lda     #2
        sta     VBLANK
        jsr     CSCANR
        lda     CFERR
        beq     PICK1
        jsr     SHOWERR
        lda     #0
        sta     VBLANK
        rts

PICK1:  lda     #FP_SEL0
        sta     FNRSEL+FH_PATHO
        lda     #FP_RST
        sta     FNRSEL+FH_PATHO
        ldx     #0
        ldy     #SSIDLN-1
        jsr     FNWRPL          ; reply -> cartridge, never through the console
        ; fall through

; ASKPW -- hand the passphrase to the keyboard, starting empty.
ASKPW:  lda     #FP_SEL3
        sta     FNRSEL+FH_PATHO
        lda     #FP_RST
        sta     FNRSEL+FH_PATHO
        lda     #0
        sta     CFELEN
        lda     #ED_PASS
        sta     CFEFLD
        jmp     TOEDIT

; ASKSSI -- "OTHER": type the network name.
ASKSSI: lda     #FP_SEL3
        sta     FNRSEL+FH_PATHO
        lda     #FP_RST
        sta     FNRSEL+FH_PATHO
        lda     #0
        sta     CFELEN
        lda     #ED_SSID
        sta     CFEFLD

TOEDIT: lda     #BANKWIF
        sta     CFRET
        lda     CFFLAG
        ora     #CFF_EDIT
        sta     CFFLAG
        lda     #BANKEDT
        jmp     CFGOTO          ; does not return

; ---------------------------------------------------------------------------
; DOCHK -- the decision tree. Leaves for the hosts bank, or sets CFSUB.
DOCHK:  jsr     FNCHK
        beq     DOC1
        lda     #FNENOC
        sta     CFERR
        lda     #WS_FAIL
        sta     CFSUB
        rts

DOC1:   jsr     CWENAB
        lda     CFERR
        bne     DOCSCN          ; cannot ask: fall back to scanning
        lda     FNRPLY
        bne     DOC2
        lda     #BANKHST        ; wifi is off by configuration
        jmp     CFGOTO          ; does not return

DOC2:   jsr     CWSTAT
        lda     CFERR
        bne     DOCSCN
        lda     FNRPLY
        cmp     #WFASSOC
        bne     DOC3
        lda     #BANKHST        ; already on
        jmp     CFGOTO          ; does not return

DOC3:   jsr     CGSSID
        lda     CFERR
        bne     DOCSCN
        lda     FNRPLY
        beq     DOCSCN          ; nothing stored
        lda     #0              ; stored: just wait for it
        sta     CFN
        sta     CFTMR
        lda     #WS_CONN
        sta     CFSUB
        rts

DOCSCN: lda     #WS_SCAN
        sta     CFSUB
        rts

; ---------------------------------------------------------------------------
; DOSCAN -- SCAN_NETWORKS, then settle on the list.
DOSCAN: lda     #WSROW0+2
        jsr     FNROWA
        ldy     #0
DSC1:   lda     TSCANN,y
        beq     DSC2
        sta     FNRSEL+FH_TCHR
        iny
        bne     DSC1
DSC2:   jsr     FNENDR

        jsr     CSCAN
        lda     CFERR
        beq     DSC3
        lda     #WS_FAIL
        sta     CFSUB
        rts

DSC3:   lda     FNRPLY
        cmp     #WSMAX
        bcc     DSC4
        lda     #WSMAX
DSC4:   sta     CFN             ; networks; the OTHER row sits at index CFN
        lda     #0
        sta     CFSEL
        lda     #WS_SELECT
        sta     CFSUB
        rts

; ---------------------------------------------------------------------------
; DRAW -- whichever screen CFSUB is on.
DRAW:   jsr     FNCLS
        ldy     CFSUB
        lda     WSTTLL,y
        sta     FNPTRL
        lda     WSTTLH,y
        sta     FNPTRH
        lda     #0
        jsr     FNRSTR

        lda     CFSUB
        cmp     #WS_SELECT
        beq     DRLIST

        ; WS_CONN and WS_FAIL are one line of explanation and nothing else.
        lda     #WSROW0+2
        jsr     FNROWA
        ldy     #0
DRW1:   lda     TWAIT,y
        beq     DRW2
        sta     FNRSEL+FH_TCHR
        iny
        bne     DRW1
DRW2:   jsr     FNENDR
        jmp     DRHINT

; The list, drawn ONCE. One GET_SCAN_RESULT per row here and none afterwards.
DRLIST: lda     #0
        sta     CFTMPB          ; row
DRL1:   lda     CFTMPB
        cmp     CFN
        bcs     DRLOTH

        jsr     CSCANR2         ; index in CFTMPB -> reply window
        lda     CFTMPB
        clc
        adc     #WSROW0
        jsr     FNROWA
        lda     CFTMPB
        cmp     CFSEL
        bne     DRL2
        lda     #'>'
        bne     DRL3            ; always taken
DRL2:   lda     #' '
DRL3:   sta     FNRSEL+FH_TCHR
        ldx     #0
        ldy     #FNTCOL-1
DRL4:   lda     FNRPLY,x
        beq     DRL5
        sta     FNRSEL+FH_TCHR
        inx
        dey
        bne     DRL4
DRL5:   jsr     FNENDR

        inc     CFTMPB
        lda     CFTMPB
        cmp     #WSMAX
        bcc     DRL1

; The OTHER row is always last, so a network the scan missed -- or a hidden
; one -- is still reachable.
DRLOTH: lda     CFN
        clc
        adc     #WSROW0
        jsr     FNROWA
        lda     CFN
        cmp     CFSEL
        bne     DRO1
        lda     #'>'
        bne     DRO2            ; always taken
DRO1:   lda     #' '
DRO2:   sta     FNRSEL+FH_TCHR
        ldy     #0
DRO3:   lda     TOTHER,y
        beq     DRO4
        sta     FNRSEL+FH_TCHR
        iny
        bne     DRO3
DRO4:   jsr     FNENDR

DRHINT: lda     #(THINT)&$FF
        sta     FNPTRL
        lda     #(THINT)>>8
        sta     FNPTRH
        lda     #WSROWH
        jsr     FNRSTR
        lda     CFERR
        beq     DRH9
        jsr     SHOWERR
DRH9:   rts

; ---------------------------------------------------------------------------
; The six wifi commands. Each parks its outcome in CFERR and its step in
; CFSTEP, the shape every transaction in this program uses.
CWENAB: lda     #FNCWENA
        ldx     #3
        jmp     NOARG

CWSTAT: lda     #FNCWSTA
        ldx     #4
        jmp     NOARG

CGSSID: lda     #FNCGSSI
        ldx     #5
        jmp     NOARG

CSCAN:  lda     #FNCSCAN
        ldx     #6
        ; fall through

; NOARG -- a FUJI command with no parameters and no payload. A is the command,
; X the step number to report if it fails.
NOARG:  stx     CFTMPA
        sta     FNCMD
        lda     #FNDEVF
        sta     FNDEV
        lda     #0
        sta     FNNPR
        jsr     FNBEG
        jmp     FINISH

; CSCANR -- GET_SCAN_RESULT for the row the cursor is on.
CSCANR: lda     CFSEL
        sta     CFTMPB
        ; fall through

; CSCANR2 -- GET_SCAN_RESULT for the index in CFTMPB. One 1-byte parameter.
CSCANR2: lda    #FNDEVF
        sta     FNDEV
        lda     #FNCSCNR
        sta     FNCMD
        lda     #1
        sta     FNNPR
        jsr     FNBEG
        lda     CFTMPB
        jsr     FNPB
        lda     #7
        sta     CFTMPA
        jmp     FINISH

; SETSSI -- SET_SSID. EXACTLY 97 bytes: ssid[33] then password[64], each
; emitted by the cartridge from its own buffer and padded here to width.
;
; nparam must be at least one even though the value is ignored -- rs232Fuji.cpp
; checks the count before it looks at anything else -- and FNPEND is
; deliberately NOT called: that pads to 256, and this payload is 97.
SETSSI: lda     #FNDEVF
        sta     FNDEV
        lda     #FNCSSSI
        sta     FNCMD
        lda     #1
        sta     FNNPR
        jsr     FNBEG
        lda     #1
        jsr     FNPB

        lda     #FP_SEL0
        sta     FNRSEL+FH_PATHO
        lda     #FP_TXRAW
        sta     FNRSEL+FH_PATHO
        lda     #SSIDLN
        sec
        sbc     FNPLNL
        jsr     PADN

        lda     #FP_SEL1
        sta     FNRSEL+FH_PATHO
        lda     #FP_TXRAW
        sta     FNRSEL+FH_PATHO
        lda     #PASSLN
        sec
        sbc     FNPLNL
        jsr     PADN

        lda     #8
        sta     CFTMPA
        ; fall through

; FINISH -- launch, and record what happened.
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

; PADN -- append A NUL bytes to the payload.
PADN:   tay
        beq     PADN9
        lda     #0
PADN1:  sta     FNTX
        dey
        bne     PADN1
PADN9:  rts

; ---------------------------------------------------------------------------
SHOWERR: lda    #WSROWE
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

WSTTLL: DB      (TCHK)&$FF, (TSCAN)&$FF, (TSEL)&$FF, (TCONN)&$FF, (TFAIL)&$FF
WSTTLH: DB      (TCHK)>>8, (TSCAN)>>8, (TSEL)>>8, (TCONN)>>8, (TFAIL)>>8

TCHK:   DB      "FN WIFI",0
TSCAN:  DB      "SCANNING",0
TSEL:   DB      "PICK NET",0
TCONN:  DB      "CONNECTING",0
TFAIL:  DB      "WIFI FAILED",0
TSCANN: DB      "SCANNING...",0
TOTHER: DB      "OTHER...",0
TWAIT:  DB      "PLEASE WAIT",0
THINT:  DB      "SEL=SKIP",0

        INCLUDE "ui.inc"
        INCLUDE "fujilib.inc"
        INCLUDE "fujidisp.inc"

        END
