; cfghost.asm -- CONFIG bank 0: the host slots.
;
; READ_HOST_SLOTS returns MAX_HOSTS * MAX_HOSTNAME_LEN = 8 * 32 = 256 bytes,
; which lands in slice 0 of the 512-byte reply window whole. Slot n's name is
; at reply offset n*32, so a row index IS a slot index and there is no
; row-to-slot map to keep -- which matters, because there is nowhere to keep
; one. Empty slots are drawn as such and refuse to mount.
;
; FIRE mounts the slot, points the cartridge's working directory at "/", and
; enters the browser. SELECT opens the action menu (see menu.inc), which is
; how everything the Intellivision reaches with a keypad digit is reached
; here.

        CPU     6502
        INCLUDE "vcs.inc"

PAD3    EQU     $81             ; the display kernel's 3-cycle pad target
SAVSP   EQU     $82

        INCLUDE "fujinet.inc"
        INCLUDE "cfgdefs.inc"

        ORG     $1000

; Entered from CFGOTO with the gate already open and the zero page already
; cleared by CFCOLD -- but NOT on a return from another bank, so nothing here
; may assume a zeroed variable it did not set itself.
START:  lda     #2
        sta     VBLANK
        lda     #0
        sta     CFERR
        sta     CFSTEP
        sta     CFMOPN

; Coming back from the keyboard is not a cold start: the cursor has to stay
; where the user left it, and CFSUB carries the verdict. Everything else about
; this bank's state is still in zero page, and -- the part that matters -- the
; eight host names are still sitting in the reply window, because the keyboard
; issues no transactions at all. That is what makes a rename cost one round
; trip instead of two.
        lda     CFFLAG
        and     #CFF_EDIT
        beq     STCOLD

        lda     CFFLAG
        and     #$FF-CFF_EDIT
        sta     CFFLAG
        lda     CFSUB
        cmp     #ED_OK
        bne     STCHK
        jsr     WRHOST
        jmp     STCHK

STCOLD: lda     #0
        sta     CFSEL
        sta     CFDEPTH

STCHK:  jsr     FNCHK
        beq     HAVE
        lda     #FNENOC
        sta     CFERR
        jmp     SHOWN

HAVE:   jsr     DRAW
SHOWN:  lda     #0
        sta     VBLANK
        jsr     DINIT
        jmp     DLOOP

; ---------------------------------------------------------------------------
APPVBL: jsr     INREPT
        sta     CFKEY

        lda     CFMOPN
        beq     AVLIST

; The menu owns the stick while it is up. MNKEY redraws itself as the cursor
; moves; it closes on a choice or on SELECT, and either way the screen
; underneath has to be repainted, because the planes cannot be read back.
        lda     CFKEY
        jsr     MNKEY
        sta     CFTMPB
        lda     CFMOPN
        bne     AVDONE
        lda     CFTMPB
        cmp     #MNNONE
        beq     AVMRED
        jsr     MNDISP          ; may CFGOTO away and never return
AVMRED: jsr     DRAW
        rts

AVLIST: lda     CFKEY
        and     #IN_DOWN
        beq     AV1
        lda     CFSEL
        clc
        adc     #1
        cmp     #NHOSTS
        bcs     AVDONE
        sta     CFSEL
        jmp     AVCUR
AV1:    lda     CFKEY
        and     #IN_UP
        beq     AV2
        lda     CFSEL
        beq     AVDONE
        sec
        sbc     #1
        sta     CFSEL
        jmp     AVCUR
AV2:    lda     CFKEY
        and     #IN_SEL
        beq     AV3
        jsr     MNOPEN
        rts
AV3:    lda     CFKEY
        and     #IN_FIRE
        beq     AVDONE
        jmp     PICK
AVDONE: rts

; Moving the cursor redraws the two rows that changed and nothing else. The
; names are still sitting in the cartridge's reply window -- READ_HOST_SLOTS
; has not been re-issued -- so this costs no round trip and no RAM.
AVCUR:  jsr     DRAWL
        rts

; ---------------------------------------------------------------------------
; MHOST -- MOUNT_HOST(CFHOST). Shared by the slot picker and the lobby, which
; is the only reason it is a routine: they mount the same way, and the lobby's
; whole trick is that it reaches an ordinary host slot by an unordinary route.
MHOST:  lda     #FNDEVF
        sta     FNDEV
        lda     #FNCMHST
        sta     FNCMD
        lda     #1
        sta     FNNPR
        jsr     FNBEG
        lda     CFHOST
        jsr     FNPB
        jsr     FNGO
        sta     CFERR
        cmp     #FNEOK
        bne     MHE
        jsr     FNACK
        sta     CFERR
        cmp     #FNEOK
        beq     MHOK
MHE:    lda     #3
        sta     CFSTEP
MHOK:   lda     CFERR
        rts

; ---------------------------------------------------------------------------
; RDHOST -- READ_HOST_SLOTS. Z set on success; the reply stays in the window.
RDHOST: lda     #FNDEVF
        sta     FNDEV
        lda     #FNCRHST
        sta     FNCMD
        lda     #0
        sta     FNNPR
        jsr     FNBEG
        jsr     FNGO
        sta     CFERR
        cmp     #FNEOK
        bne     RDHE
        jsr     FNACK
        sta     CFERR
        cmp     #FNEOK
        beq     RDHOK
RDHE:   lda     #1
        sta     CFSTEP
        lda     #1
        rts
RDHOK:  lda     #0
        rts

; ---------------------------------------------------------------------------
; DRAW -- the whole screen.
; With a copy pending this screen is a destination picker, and it says so.
; Picking a slot still goes through the ordinary mount-and-browse path -- there
; is no separate destination-host screen the way src/destination_host_slot.c
; is one -- so the title is the only thing that changes.
DRAW:   jsr     FNCLS
        lda     CFCOPY
        cmp     #CP_MARK
        bne     DRWT
        lda     #(TCOPYT)&$FF
        sta     FNPTRL
        lda     #(TCOPYT)>>8
        sta     FNPTRH
        jmp     DRWT2
DRWT:   lda     #(TTITLE)&$FF
        sta     FNPTRL
        lda     #(TTITLE)>>8
        sta     FNPTRH
DRWT2:  lda     #0
        jsr     FNRSTR

        jsr     RDHOST
        bne     DRBAD
        lda     #NHOSTS
        sta     CFCNT
        jsr     DRAWL
        lda     #(THINT)&$FF
        sta     FNPTRL
        lda     #(THINT)>>8
        sta     FNPTRH
        lda     #ROW0+NHOSTS+1
        jsr     FNRSTR
        lda     #0
        rts
DRBAD:  jsr     SHOWERR
        lda     #1
        rts

; DRAWL -- the eight list rows, from the reply window still in place.
DRAWL:  ldx     #0
DRL1:   stx     CFWANT
        txa
        clc
        adc     #ROW0
        jsr     FNROWA

        ldx     CFWANT
        cpx     CFSEL
        bne     DRLNC
        lda     #'>'
        jmp     DRLC
DRLNC:  lda     #' '
DRLC:   sta     FNRSEL+FH_TCHR

        ; Reply offset = slot * 32. Eight slots at 32 bytes is 256, so the
        ; high byte is never needed and a single-byte index reaches them all.
        lda     CFWANT
        asl     a
        asl     a
        asl     a
        asl     a
        asl     a               ; * HOSTSLN
        tax
        lda     FNRPLY,x
        bne     DRLNM
        ; An empty slot. Say so rather than leaving a blank the cursor can
        ; sit on with no explanation.
        lda     #(TEMPTY)&$FF
        sta     FNPTRL
        lda     #(TEMPTY)>>8
        sta     FNPTRH
        jsr     PUTSTR
        jmp     DRLEND
DRLNM:  ldy     #FNTCOL-1
        jsr     PUTRPL
DRLEND: jsr     FNENDR
        ldx     CFWANT
        inx
        cpx     #NHOSTS
        bcc     DRL1
        rts

; ---------------------------------------------------------------------------
; PICK -- mount the selected host and enter the browser.
PICK:   lda     #2
        sta     VBLANK

        ; Refuse an empty slot. The name is in the reply window from the last
        ; READ_HOST_SLOTS, which nothing has disturbed.
        lda     CFSEL
        asl     a
        asl     a
        asl     a
        asl     a
        asl     a
        tax
        lda     FNRPLY,x
        bne     PICK1
        lda     #0
        sta     VBLANK
        rts                     ; nothing mounted, nothing said: the row
                                ;   already reads (EMPTY)
PICK1:  lda     CFSEL
        sta     CFHOST
        jsr     MHOST
        lda     CFERR
        bne     PICKB

        ; The working directory: the root of the host we just mounted. It
        ; lives in the CARTRIDGE, so the browser inherits it across the bank
        ; switch without a byte of it passing through console RAM.
        jsr     FNWRST
        lda     #'/'
        jsr     FNWCH

        lda     #0
        sta     CFPAGE
        sta     CFSEL
        sta     CFDEPTH
        lda     #BANKDIR
        jmp     CFGOTO          ; does not return

PICKB:  lda     #2
        sta     CFSTEP
        jsr     SHOWERR
        lda     #0
        sta     VBLANK
        rts

; ---------------------------------------------------------------------------
; PUTSTR -- append the string at FNPTRL/H to the row being composed.
; (FNRSTR selects the row itself; these do not, so a cursor can go first.)
PUTSTR: ldy     #0
PUTS1:  lda     (FNPTRL),y
        beq     PUTS2
        sta     FNRSEL+FH_TCHR
        iny
        cpy     #FNTCOL-1
        bne     PUTS1
PUTS2:  rts

; PUTRPL -- append up to Y bytes of the reply window at offset X.
PUTRPL: lda     FNRPLY,x
        beq     PUTR2
        sta     FNRSEL+FH_TCHR
        inx
        dey
        bne     PUTRPL
PUTR2:  rts

SHOWERR:
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

; ---------------------------------------------------------------------------
; MNDISP -- act on the chosen menu item.
MNDISP: cmp     #0
        bne     MNDS1
        lda     #BANKINF
        jmp     CFGOTO          ; does not return

MNDS1:  cmp     #1
        bne     MNDS2
        jmp     RENAME          ; does not return

MNDS2:  cmp     #2
        bne     MNDS3
        ; Reconfigure wifi. Straight to the scan, not to the wifi bank's own
        ; check: the check exists to decide whether setup is needed, and asking
        ; for it from here means it is.
        lda     #WS_SCAN
        sta     CFSUB
        lda     #BANKWIF
        jmp     CFGOTO          ; does not return

MNDS3:  cmp     #3
        bne     MNDS9
        jmp     LOBBY           ; does not return on success
MNDS9:  rts

; ---------------------------------------------------------------------------
; LOBBY -- boot the Game Lobby.
;
; The lobby is a ROM on a particular TNFS host, so this is not a new mechanism
; at all: find that host among the eight slots, mount it the ordinary way,
; point the working directory at the ROM, and hand over to the boot bank. The
; only wrinkle is that the host may not be in the slots yet, in which case the
; last one is claimed for it -- the same thing every sibling port does, and
; the reason it is the LAST slot is that it is the least likely to be in use.
LOBBY:  lda     #2
        sta     VBLANK

        lda     #0
        sta     CFTMPB          ; slot being examined
LBS1:   lda     CFTMPB          ; reply offset = slot * HOSTSLN
        asl     a
        asl     a
        asl     a
        asl     a
        asl     a
        tax
        ldy     #0
LBS2:   lda     FNRPLY,x
        jsr     LOWER           ; host names are not case-sensitive
        cmp     LBHOST,y
        bne     LBSNX
        inx
        iny
        cpy     #LBHLEN
        bcc     LBS2
        lda     FNRPLY,x        ; and it has to END there, not merely start
        beq     LBGOT
LBSNX:  inc     CFTMPB
        lda     CFTMPB
        cmp     #NHOSTS
        bcc     LBS1
        jmp     LBCLAM

LBGOT:  lda     CFTMPB
        sta     CFSEL
        jmp     LBMNT

; Not there: put it in the last slot. WRHOST substitutes the edit scratch for
; the slot CFSEL names, so the name goes in the same way a rename does.
LBCLAM: lda     #FP_SEL3
        sta     FNRSEL+FH_PATHO
        lda     #FP_RST
        sta     FNRSEL+FH_PATHO
        ldy     #0
LBC1:   lda     LBHOST,y
        beq     LBC2
        sta     FNRSEL+FH_PATHC
        iny
        bne     LBC1
LBC2:   lda     #NHOSTS-1
        sta     CFSEL
        jsr     WRHOST
        lda     CFERR
        bne     LBBAD
        jsr     RDHOST          ; the reply window is stale after a write
        bne     LBBAD

LBMNT:  lda     CFSEL
        sta     CFHOST
        jsr     MHOST
        lda     CFERR
        bne     LBBAD

; The full path of the ROM, straight into the cartridge's buffer 0, which is
; exactly what the boot bank expects to find there.
        lda     #FP_SEL0
        sta     FNRSEL+FH_PATHO
        jsr     FNWRST
        ldy     #0
LBP1:   lda     LBPATH,y
        beq     LBP2
        jsr     FNWCH
        iny
        bne     LBP1
LBP2:   lda     #BT_GO
        sta     CFSUB
        lda     #BANKBOT
        jmp     CFGOTO          ; does not return

LBBAD:  jsr     SHOWERR
        lda     #0
        sta     VBLANK
        rts

LOWER:  cmp     #'A'
        bcc     LOW9
        cmp     #'Z'+1
        bcs     LOW9
        clc
        adc     #32
LOW9:   rts

; ---------------------------------------------------------------------------
; RENAME -- hand the selected slot's name to the keyboard.
;
; The scratch is seeded a character at a time straight out of the reply
; window, so the current name never occupies console RAM. CFELEN has to be
; seeded too: it is the keyboard's mirror of a length it cannot read back.
RENAME: lda     #FP_SEL3
        sta     FNRSEL+FH_PATHO
        lda     #FP_RST
        sta     FNRSEL+FH_PATHO

        lda     CFSEL           ; reply offset = slot * HOSTSLN
        asl     a
        asl     a
        asl     a
        asl     a
        asl     a
        tax
        ldy     #0
RNM1:   lda     FNRPLY,x
        beq     RNM2
        sta     FNRSEL+FH_PATHC
        inx
        iny
        cpy     #HOSTSLN-1
        bcc     RNM1
RNM2:   sty     CFELEN

        lda     #ED_HOST
        sta     CFEFLD
        lda     #BANKHST
        sta     CFRET
        lda     CFFLAG
        ora     #CFF_EDIT
        sta     CFFLAG
        lda     #BANKEDT
        jmp     CFGOTO          ; does not return

; ---------------------------------------------------------------------------
; WRHOST -- WRITE_HOST_SLOTS. All eight slots, 256 bytes, because there is no
; single-slot write in the protocol.
;
; Seven of them are copied straight back out of the reply window READ_HOST_SLOTS
; filled, and the eighth is emitted by the cartridge from the edit scratch.
; Nothing is buffered anywhere: on a console with 128 bytes of RAM a 256-byte
; payload has to be assembled as it is sent.
WRHOST: lda     #FNDEVF
        sta     FNDEV
        lda     #FNCWHST
        sta     FNCMD
        lda     #0
        sta     FNNPR
        jsr     FNBEG
        jsr     FNPBEG

        lda     #0
        sta     CFTMPB          ; slot
        ldx     #0              ; reply offset, slot * HOSTSLN
WRH1:   lda     CFTMPB
        cmp     CFSEL
        beq     WRHNEW

        ldy     #HOSTSLN        ; verbatim, NUL padding and all
WRH2:   lda     FNRPLY,x
        jsr     FNPCH
        inx
        dey
        bne     WRH2
        jmp     WRHNXT

WRHNEW: lda     #FP_SEL3
        sta     FNRSEL+FH_PATHO
        lda     #FP_TXRAW
        sta     FNRSEL+FH_PATHO
        lda     FNPCNT          ; the cartridge emitted these; count them
        clc
        adc     FNPLNL
        sta     FNPCNT

        lda     #HOSTSLN        ; pad the field out to its full width
        sec
        sbc     FNPLNL
        beq     WRH4            ; exactly full: nothing to pad
        tay
        lda     #0
WRH3:   jsr     FNPCH
        dey
        bne     WRH3
WRH4:   txa                     ; step the reply cursor over the slot we
        clc                     ;   replaced rather than copied
        adc     #HOSTSLN
        tax

WRHNXT: inc     CFTMPB
        lda     CFTMPB
        cmp     #NHOSTS
        bcc     WRH1

        jsr     FNPEND
        jsr     FNGO
        sta     CFERR
        cmp     #FNEOK
        bne     WRHE
        jsr     FNACK
        sta     CFERR
        cmp     #FNEOK
        beq     WRHOK
WRHE:   lda     #2
        sta     CFSTEP
WRHOK:  rts

MNTITL: DB      "ACTIONS",0
MNCNT   EQU     4

; Sixteen bytes an entry; see menu.inc. Eleven usable columns, because column
; zero is the cursor gutter.
MNTBL:  DB      "INFO",0
        DS      16-5
        DB      "RENAME",0
        DS      16-7
        DB      "WIFI",0
        DS      16-5
        DB      "LOBBY",0
        DS      16-6

; The lobby's host and ROM. ONE literal each, so there is one place to correct
; when the path on the server is confirmed -- the Intellivision uses
; ec.tnfs.io + /intv/lobby.rom, and this is the 2600's equivalent.
LBHOST: DB      "ec.tnfs.io",0
LBHLEN  EQU     10
LBPATH: DB      "/a2600/lobby.bin",0

TTITLE: DB      "FN HOSTS",0
TCOPYT: DB      "COPY TO",0
TEMPTY: DB      "(EMPTY)",0
THINT:  DB      "SEL=MENU",0

        INCLUDE "menu.inc"
        INCLUDE "fujilib.inc"
        INCLUDE "fujidisp.inc"

        END
