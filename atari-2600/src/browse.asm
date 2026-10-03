; cfgdir.asm -- CONFIG bank 1: the directory browser.
;
; Up and down move the cursor. Left and right page. FIRE descends into a
; folder or boots a file. SELECT goes back up a level, and up from the root
; goes back to the host screen.
;
; THE WORKING DIRECTORY IS NOT HERE. It is 256 bytes in the cartridge, and
; this console has 128 bytes of RAM with the stack in the top of them, so it
; could not be here. Descending is: append the entry's name to the cartridge's
; buffer, straight from the cartridge's reply window, and re-list. Going up is
; one store -- the cartridge knows where the last separator was, so this does
; not have to.
;
; No filename ever lands in console RAM either. A name goes cartridge -> text
; plane to be drawn, and cartridge -> path buffer to be entered, and this
; program only ever holds an index. That is the ColecoVision port's lesson
; taken as far as it goes.
;
; The cost is that moving the cursor re-lists the directory: with no names in
; RAM there is nothing to redraw from. Under emulation that is instant; on
; real hardware it is about a second a step, and the fix when it matters is a
; cartridge-side page cache, which is the same shape of problem the blit port
; already solves.

        CPU     6502
        INCLUDE "vcs.inc"

PAD3    EQU     $81
SAVSP   EQU     $82

        INCLUDE "fujinet.inc"
        INCLUDE "cfgdefs.inc"

        ORG     $1000

START:  lda     #2
        sta     VBLANK
        lda     #0
        sta     CFERR
        sta     CFSTEP
        sta     CFMOPN

; Coming back from the keyboard with a filter. A new pattern renumbers every
; entry in the directory, so the page and the cursor both have to go back to
; the start -- keeping them would point at a row that no longer exists.
        lda     CFFLAG
        and     #CFF_EDIT
        beq     STDRAW

        lda     CFFLAG
        and     #$FF-CFF_EDIT
        sta     CFFLAG
        lda     CFSUB
        cmp     #ED_OK
        bne     STDRAW

        lda     #FP_SEL1        ; is the new filter empty? then there is none
        sta     FNRSEL+FH_PATHO
        lda     FNPLNL
        beq     STNOFI
        lda     CFFLAG
        ora     #CFF_FILT
        bne     STSETF          ; always taken
STNOFI: lda     CFFLAG
        and     #$FF-CFF_FILT
STSETF: sta     CFFLAG
        lda     #FP_SEL0
        sta     FNRSEL+FH_PATHO
        lda     #0
        sta     CFPAGE
        sta     CFSEL

STDRAW: jsr     DRAW
        lda     #0
        sta     VBLANK
        jsr     DINIT
        jmp     DLOOP

; ---------------------------------------------------------------------------
APPVBL: jsr     INREPT
        sta     CFKEY

        lda     CFMOPN
        beq     AVLIST
        lda     CFKEY
        jsr     MNKEY
        sta     CFTMPB
        lda     CFMOPN
        bne     AVDONE
        lda     CFTMPB
        cmp     #MNNONE
        beq     AVMRED
        jsr     MNDISP          ; UP DIR, FILTER and INFO do not return
AVMRED: jmp     AVRE

AVLIST: lda     CFKEY
        and     #IN_DOWN
        beq     AV1
        lda     CFSEL
        clc
        adc     #1
        cmp     CFCNT
        bcs     AVDONE
        pha
        lda     CFSEL
        sta     CFTMPA
        pla
        sta     CFSEL
        jmp     AVMOVE

AV1:    lda     CFKEY
        and     #IN_UP
        beq     AV2
        lda     CFSEL
        beq     AVDONE
        sta     CFTMPA
        sec
        sbc     #1
        sta     CFSEL
        jmp     AVMOVE

AV2:    lda     CFKEY
        and     #IN_RIGHT
        beq     AV3
        lda     CFFULL          ; only if the page we are on was full: paging
        beq     AVDONE          ;   past the end NAKs for the rest of the
        inc     CFPAGE          ;   session on an SD host
        lda     #0
        sta     CFSEL
        jmp     AVRE

AV3:    lda     CFKEY
        and     #IN_LEFT
        beq     AV4
        lda     CFPAGE
        beq     GOUP            ; already at the first page: LEFT means BACK,
        dec     CFPAGE          ;   which is the obvious thing for it to mean
        lda     #0              ;   and saves burying the commonest action in
        sta     CFSEL           ;   a menu
        jmp     AVRE

AV4:    lda     CFKEY
        and     #IN_SEL
        beq     AV5
        jsr     MNOPEN
        rts

AV5:    lda     CFKEY
        and     #IN_FIRE
        beq     AVDONE
        jmp     PICK
AVDONE: rts

; Moving the cursor changes two characters and nothing else: no re-listing,
; and so no round trips. CFTMPA is the row it came from.
AVMOVE: lda     CFSEL
        clc
        adc     #ROW0
        tax
        lda     CFTMPA
        clc
        adc     #ROW0
        jmp     UIGUT

AVRE:   lda     #2              ; blank while we talk; a listing is many round
        sta     VBLANK          ;   trips and the frame will not finish
        jsr     DRAW
        lda     #0
        sta     VBLANK
        rts

; ---------------------------------------------------------------------------
; GOUP -- back up a level, or back to the host screen from the root.
GOUP:   lda     CFDEPTH
        bne     GOUP1
        lda     #BANKHST
        jmp     CFGOTO          ; does not return
GOUP1:  dec     CFDEPTH
        jsr     FNWPOP
        lda     #0
        sta     CFPAGE
        sta     CFSEL
        jmp     AVRE

; ---------------------------------------------------------------------------
; ODIR -- OPEN_DIRECTORY(host, <the cartridge's path>). Z set on success.
;
; One parameter, then a 256-byte payload emitted by the cartridge from its own
; buffer. FNWTX is a single store and the other 255 bytes never cross the bus.
ODIR:   lda     #FNDEVF
        sta     FNDEV
        lda     #FNCODIR
        sta     FNCMD
        lda     #1
        sta     FNNPR
        jsr     FNBEG
        lda     CFHOST
        jsr     FNPB

; The payload is "path\0filter\0", NUL-padded to EXACTLY 256 -- the server
; reads that many and a short one fails its read. Both strings are emitted by
; the cartridge from its own buffers; neither ever crosses into console RAM,
; which is the whole reason the filter needs a buffer of its own rather than
; sharing the working directory's.
        jsr     FNPBEG
        lda     #FP_SEL0
        sta     FNRSEL+FH_PATHO
        jsr     FNPWD
        lda     #0
        jsr     FNPCH           ; terminate the path

        lda     CFFLAG
        and     #CFF_FILT
        beq     ODIRF
        lda     #FP_SEL1
        sta     FNRSEL+FH_PATHO
        jsr     FNPWD
ODIRF:  lda     #0
        jsr     FNPCH           ; terminate the filter, empty or not
        jsr     FNPEND

        lda     #FP_SEL0        ; everything else assumes buffer 0 is live
        sta     FNRSEL+FH_PATHO
        jsr     FNGO
        sta     CFERR
        cmp     #FNEOK
        bne     ODE
        jsr     FNACK
        sta     CFERR
        cmp     #FNEOK
        beq     ODOK
ODE:    lda     #2
        sta     CFSTEP
        lda     #1
        rts
ODOK:   rts                     ; A is FNEOK, so Z is set

; ---------------------------------------------------------------------------
; SEEK -- SET_DIRECTORY_POSITION(CFPAGE * NROWS). Z set on success, and a no-op
; on page 0 so the common case costs no round trip.
;
; The position is ONE parameter of TWO bytes -- fujiDevice reads it as
; packet.param(0) declared uint16_t -- so it is {size 2, lo, hi}, not two
; one-byte parameters.
SEEK:   lda     CFPAGE
        bne     SEEK1
        lda     #FNEOK
        rts
SEEK1:  lda     #FNDEVF
        sta     FNDEV
        lda     #FNCSDPS
        sta     FNCMD
        lda     #1
        sta     FNNPR
        jsr     FNBEG
        jsr     POSLO           ; A = low byte, X = high byte
        jsr     FNPW
        jsr     FNGO
        sta     CFERR
        cmp     #FNEOK
        bne     SKE
        jsr     FNACK
        sta     CFERR
        cmp     #FNEOK
        beq     SKOK
SKE:    lda     #3
        sta     CFSTEP
        lda     #1
        rts
SKOK:   rts

; POSLO -- A = low byte and X = high byte of CFPAGE * NROWS. NROWS is 14, so
; 14 pages fit in a byte and the high byte only matters past entry 255.
POSLO:  lda     #0
        sta     CFWANT          ; scratch: the high byte accumulates here
        lda     CFPAGE
        beq     POSL2
        tax
        lda     #0
POSL1:  clc
        adc     #NROWS
        bcc     POSL3
        inc     CFWANT
POSL3:  dex
        bne     POSL1
POSL2:  ldx     CFWANT
        rts

; ---------------------------------------------------------------------------
; RDENT -- READ_DIR_ENTRY(NAMELN, 0). Z set on success; the entry stays in the
; reply window and is not copied anywhere.
RDENT:  lda     #FNDEVF
        sta     FNDEV
        lda     #FNCRDIR
        sta     FNCMD
        lda     #2
        sta     FNNPR
        jsr     FNBEG
        lda     #NAMELN
        jsr     FNPB
        lda     #0
        jsr     FNPB
        jsr     FNGO
        cmp     #FNEOK
        bne     RDE
        jsr     FNACK
        cmp     #FNEOK
        bne     RDE
        lda     #FNEOK
        rts
RDE:    sta     CFERR
        lda     #4
        sta     CFSTEP
        lda     #1
        rts

; ---------------------------------------------------------------------------
; ISCFG -- does the entry in the reply window end in ".cfg"? Z set if it does.
;
; This has to be done here rather than by the server's filter: the firmware's
; wildcard matcher understands only '*' and '?', so "not *.cfg" cannot be
; expressed as a pattern no matter what the user types. Every CONFIG in the
; family suppresses these client-side for the same reason -- a .bin and its
; .cfg sibling are one item to a person, and listing both doubles the pages.
ISCFG:  ldx     #0
ICF1:   lda     FNRPLY,x
        beq     ICF2
        inx
        cpx     #NAMELN
        bcc     ICF1
ICF2:   cpx     #4
        bcc     ICFNO
        dex
        lda     FNRPLY,x
        jsr     LOWER
        cmp     #'g'
        bne     ICFNO
        dex
        lda     FNRPLY,x
        jsr     LOWER
        cmp     #'f'
        bne     ICFNO
        dex
        lda     FNRPLY,x
        jsr     LOWER
        cmp     #'c'
        bne     ICFNO
        dex
        lda     FNRPLY,x
        cmp     #'.'
        rts                     ; Z is set exactly when it is a .cfg
ICFNO:  lda     #1
        cmp     #0              ; force Z clear
        rts

LOWER:  cmp     #'A'
        bcc     LOW9
        cmp     #'Z'+1
        bcs     LOW9
        clc
        adc     #32
LOW9:   rts

; ---------------------------------------------------------------------------
; RDVIS -- read the next entry that should be SHOWN, skipping .cfg siblings.
;
; Both the listing and the pick go through this, and they must: the row index
; only maps back onto a directory position if both count the same entries.
RDVIS:  jsr     RDENT
        bne     RDV9            ; a transport failure, already recorded
        jsr     FNEOF
        beq     RDVOK           ; the end marker: the caller has to see it
        jsr     ISCFG
        beq     RDVIS           ; a .cfg: silently take the next one
RDVOK:  lda     #FNEOK
RDV9:   rts

; ---------------------------------------------------------------------------
; DRAW -- re-list the current directory page.
DRAW:   jsr     FNCLS
        lda     #(TTITLE)&$FF
        sta     FNPTRL
        lda     #(TTITLE)>>8
        sta     FNPTRH
        lda     #0
        jsr     FNRSTR
; Row 1 is WHERE YOU ARE. The console cannot read the path buffer back -- the
; page it is written through is write-only -- so the cartridge renders it,
; tail-anchored, because the interesting end of "/games/atari/" is the right
; one and there are twelve columns. Before FN_BLIT_PATH this row could only
; show a page number.
        lda     #FP_SEL0
        sta     FNRSEL+FH_PATHO
        lda     FNPLNL
        sec
        sbc     #FNTCOL
        bcs     DRP1
        lda     #0
DRP1:   sta     FNRSEL+FH_BLTSL
        lda     #0
        sta     FNRSEL+FH_BLTSH
        lda     #1
        sta     FNRSEL+FH_BLTDL
        lda     #0
        sta     FNRSEL+FH_BLTDH
        lda     #FNTCOL
        sta     FNRSEL+FH_BLTCN
        lda     #FB_PATH
        sta     FNRSEL+FH_BLTGO

        jsr     ODIR
        bne     DRBAD
        jsr     SEEK
        bne     DRBAD

        lda     #0
        sta     CFCNT
        sta     CFFULL
        ldx     #0
DRLOOP: stx     CFWANT
        jsr     RDVIS
        bne     DRBAD
        jsr     FNEOF
        beq     DREND           ; the $7F,$7F marker: stop, and do NOT read on
        ldx     CFWANT

        txa
        clc
        adc     #ROW0
        jsr     FNROWA
        lda     CFWANT
        cmp     CFSEL
        bne     DRNC
        lda     #'>'
        jmp     DRC
DRNC:   lda     #' '
DRC:    sta     FNRSEL+FH_TCHR
        ldx     #0
        ldy     #FNTCOL-1
        jsr     PUTRPL
        jsr     FNENDR

        ldx     CFWANT
        inx
        stx     CFCNT
        cpx     #NROWS
        bcc     DRLOOP
        lda     #1              ; the page filled, so there may be another
        sta     CFFULL

DREND:  ldx     CFCNT
DRBL:   cpx     #NROWS
        bcs     DRHINT
        txa
        clc
        adc     #ROW0
        jsr     FNROWA
        jsr     FNENDR
        inx
        jmp     DRBL
DRHINT: lda     #(THINT)&$FF
        sta     FNPTRL
        lda     #(THINT)>>8
        sta     FNPTRH
        lda     #ROW0+NROWS+1
        jsr     FNRSTR
        ; The cursor may be past the end of a short page.
        lda     CFCNT
        beq     DROK
        cmp     CFSEL
        bcs     DROK
        sec
        sbc     #1
        sta     CFSEL
DROK:   lda     #0
        rts
DRBAD:  jsr     SHOWERR
        lda     #1
        rts

; PUTRPL -- append up to Y bytes of the reply window at offset X.
PUTRPL: lda     FNRPLY,x
        beq     PUTR2
        sta     FNRSEL+FH_TCHR
        inx
        dey
        bne     PUTRPL
PUTR2:  rts

; ---------------------------------------------------------------------------
; PICK -- descend into the selection, or boot it.
;
; The name is not in RAM, so the directory is re-opened and re-read up to the
; selection. What comes back is examined IN THE REPLY WINDOW: a trailing "/"
; means a folder.
PICK:   lda     #2
        sta     VBLANK
        jsr     ODIR
        beq     PK1
        jmp     PKBAD
PK1:    jsr     SEEK
        beq     PK2
        jmp     PKBAD
PK2:    lda     #0
        sta     CFWANT
PKSKIP: jsr     RDVIS
        beq     PKS1
        jmp     PKBAD
PKS1:   jsr     FNEOF
        bne     PKS2
        jmp     PKBAD           ; ran off the end: the selection is gone
PKS2:   lda     CFWANT
        cmp     CFSEL
        beq     PKGOT
        inc     CFWANT
        jmp     PKSKIP

; Folder or file? Walk to the NUL and look at the byte before it.
PKGOT:  ldx     #0
PKL1:   lda     FNRPLY,x
        beq     PKL2
        inx
        cpx     #NAMELN
        bcc     PKL1
PKL2:   cpx     #0
        bne     PKL3
        jmp     PKBAD           ; an empty name is nothing to act on
PKL3:   dex
        lda     FNRPLY,x
        cmp     #'/'
        beq     PKDIR
        jmp     PKFILE

; ---- descend ---------------------------------------------------------------
; The name goes from the cartridge's reply window straight into the
; cartridge's path buffer. It is already held with its trailing "/", which is
; exactly the form the buffer wants, so appending it is the whole operation.
PKDIR:  ldx     #0
        ldy     #NAMELN
        jsr     FNWRPL
        inc     CFDEPTH
        lda     #0
        sta     CFPAGE
        sta     CFSEL
        jsr     DRAW
        lda     #0
        sta     VBLANK
        rts

; ---- boot ------------------------------------------------------------------
; The filename is appended to the working directory, so buffer 0 now holds the
; FULL PATH -- which is the boot bank's entire input. On failure that bank
; pops the filename off again and this one is back where it was.
; Either way the filename is appended to the working directory first, so
; buffer 0 holds the FULL PATH of the highlighted file.
PKFILE: ldx     #0
        ldy     #NAMELN
        jsr     FNWRPL

        lda     CFCOPY
        cmp     #CP_MARK
        beq     PKMARK

        lda     #BT_GO
        sta     CFSUB
        lda     #BANKBOT
        jmp     CFGOTO          ; does not return

; Marking a source: copy the full path into buffer 2, where it will survive
; the user browsing anywhere at all -- including onto a different host --
; and then put buffer 0 back to the directory it was.
PKMARK: lda     #FP_SEED        ; scratch <- buffer 0, the full path
        sta     FNRSEL+FH_PATHO
        lda     #FP_SEL2
        sta     FNRSEL+FH_PATHO
        lda     #FP_CMIT        ; buffer 2 <- scratch
        sta     FNRSEL+FH_PATHO
        lda     #FP_SEL0
        sta     FNRSEL+FH_PATHO
        jsr     FNWPOP          ; drop the filename again
        lda     CFHOST
        sta     CFCSRC
        lda     #BANKHST        ; pick a destination host
        jmp     CFGOTO          ; does not return

PKBAD2: lda     #5
        sta     CFSTEP
PKBAD:  jsr     SHOWERR
        lda     #0
        sta     VBLANK
        rts

; ---------------------------------------------------------------------------
; MNDISP -- the browser's actions.
MNDISP: cmp     #0
        bne     MNDS1
        jmp     GOUP            ; may CFGOTO to the hosts and not return

MNDS1:  cmp     #1
        bne     MNDS2
        jmp     ASKFIL          ; does not return

MNDS2:  cmp     #2
        bne     MNDS3
        lda     #BANKINF
        jmp     CFGOTO          ; does not return

MNDS3:  cmp     #3
        bne     MNDS4
        lda     #CP_MARK        ; mark the highlighted file as the source
        sta     CFCOPY
        jmp     PICK            ; does not return

MNDS4:  cmp     #4
        bne     MNDS9
        lda     CFCOPY          ; nothing marked: say so rather than copying
        cmp     #CP_MARK        ;   something arbitrary
        bne     MNDS9
        lda     #BANKCPY
        jmp     CFGOTO          ; does not return
MNDS9:  rts

; ASKFIL -- edit the filter. The keyboard is seeded from the current one, so
; refining a pattern does not mean retyping it, and CANCEL cannot lose it: the
; scratch is a different buffer and only ACCEPT copies it back.
ASKFIL: lda     #FP_SEL1
        sta     FNRSEL+FH_PATHO
        lda     #FP_SEED
        sta     FNRSEL+FH_PATHO
        lda     FNPLNL
        sta     CFELEN
        lda     #FP_SEL3
        sta     FNRSEL+FH_PATHO
        lda     #ED_FILT
        sta     CFEFLD
        lda     #BANKDIR
        sta     CFRET
        lda     CFFLAG
        ora     #CFF_EDIT
        sta     CFFLAG
        lda     #BANKEDT
        jmp     CFGOTO          ; does not return

MNTITL: DB      "ACTIONS",0
MNCNT   EQU     5
MNTBL:  DB      "UP DIR",0
        DS      16-7
        DB      "FILTER",0
        DS      16-7
        DB      "INFO",0
        DS      16-5
        DB      "COPY",0
        DS      16-5
        DB      "COPY HERE",0
        DS      16-10

; ---------------------------------------------------------------------------
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

TTITLE: DB      "FN BROWSE",0
THINT:  DB      "SEL=UP",0

        INCLUDE "menu.inc"
        INCLUDE "ui.inc"
        INCLUDE "fujilib.inc"
        INCLUDE "fujidisp.inc"

        END
