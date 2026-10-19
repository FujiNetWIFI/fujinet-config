; config.asm -- FujiNet CONFIG for the Emerson Arcadia 2001.
;
; The full CONFIG client: WiFi setup (scan / custom SSID / on-screen
; password entry / connect), the 8 host slots (list, mount, rename), the
; Game Lobby, directory browsing (paging, subfolders, .cfg suppression,
; filtering, a scrollbar), copying a file between hosts, an info screen,
; and boot with a progress bar -- the Arcadia counterpart of
; fujinet-config/astrocade, grown from the bring-up's testrom/fujicfg.asm
; and speaking the same cartridge mailbox.
;
; Layout: block 1 (CPU $0000-$0FFF) holds everything that touches the
; screen or RAM; block 2 (CPU $2000-$2AFF) shares 2650 page 1 with the
; mailbox and holds the transport, the Fuji command wrappers, the
; screens that mostly talk to the cart, and the tables and strings.
; Cross-page calls are BSTA/BCTA (15-bit); cross-page DATA goes through
; DWBE pointer words, and AS refuses a direct operand on the wrong page.
;
; Every state is entered with BCTA on whatever the return stack holds:
; the 2650's 8-entry stack wraps, so an abandoned frame costs nothing,
; and tools/checkdepth.py bounds every live call chain.
;
; RESET on the console restarts this program but not the cart, which is
; why sequence numbers always derive from ACKSEQ and START initialises
; every variable.

        CPU     2650

        INCLUDE "build/flags.inc"
        INCLUDE "macros.inc"

; ---- RAM ledger ---------------------------------------------------------
; 13-line mode leaves: zone A $18D0-$18EF, the island $18F8-$18FB, and
; zone B $1A00-$1AFF (the lower screen in 26-line mode, which CONFIG
; never uses). 292 bytes; every one of them is a line here.

; -- zone A --
V_HOST  EQU     $18D0           ; selected host slot 0-7
V_CUR   EQU     $18D1           ; list cursor row
V_CNT   EQU     $18D2           ; rows on this page / list length
V_DPOS  EQU     $18D3           ; u16 HI,LO: position of the page's top row
V_DNEXT EQU     $18D5           ; u16 HI,LO: position after its last row
V_CMODE EQU     $18D7           ; 0 normal, 1 picking a copy destination
V_CHOST EQU     $18D8           ; copy source host slot
V_TOT   EQU     $18D9           ; u16 HI,LO: entries in the dir, $FFFF unknown
V_FLD   EQU     $18DB           ; DPRTN/DDEC field counter
DCOLOR  EQU     $18DC           ; attribute DPUTC ORs in
V_KEY   EQU     $18DD           ; last raw control (edge detect)
V_T0    EQU     $18DE           ; leaf-helper scratch
V_T1    EQU     $18DF
; $18E0-$18EA is where the boot stub lands, so only state that is dead
; at the handover lives here.
V_RPT   EQU     $18E0           ; auto-repeat countdown
AXV     EQU     $18E1           ; cached vertical pot
AXH     EQU     $18E2           ; cached horizontal pot
AXSEL   EQU     $18E3           ; axis KEYRAW samples this frame
V_T2    EQU     $18E4           ; screen-level scratch (SBAR uses all)
V_T3    EQU     $18E5
V_T4    EQU     $18E6
V_T5    EQU     $18E7
V_T6    EQU     $18E8
V_T7    EQU     $18E9
V_T8    EQU     $18EA
FSTRP   EQU     $18EB           ; TX string pointer, big-endian
DSTRP   EQU     $18ED           ; display string pointer, big-endian
V_EOF   EQU     $18EF           ; the page reached the end of the dir
;       $18F0-$18F7 sprite positions (hardware)
V_NNET  EQU     $18F8           ; networks listed by the last scan
V_HLW   EQU     $18F9           ; DHILIT width (16 hosts, 14 files)
V_FRM   EQU     $18FA           ; frame counter (sweep, marquee)
V_SWP   EQU     $18FB           ; sweep phase
;       $18FC-$18FF UVI registers

; -- zone B --
V_PATH  EQU     $1A00           ; 88: current dir, leading + trailing '/'
V_HNAME EQU     $1A00           ; 33: rename (V_PATH is dead on hosts)
V_SSID  EQU     $1A00           ; 33: \ the WiFi phase; they run into
V_PASS  EQU     $1A21           ; 64: /  V_SRC, so WiFi cancels a copy
V_SRC   EQU     $1A58           ; 88: copy-source full path
V_FILT  EQU     $1AB0           ; 16: directory filter ("" = none)
V_FSCR  EQU     $1AC0           ; 16: filter edit scratch (ESC is free)
V_PSTK  EQU     $1AD0           ; 16: page-start ring, 8 x u16 HI,LO
V_ROWM  EQU     $1AE0           ; 8: per row b0-5 offset from DPOS,
                                ;    b6 directory
V_PHEAD EQU     $1AE8           ; ring head
V_PNUM  EQU     $1AE9           ; ring entries
V_EBUF  EQU     $1AEA           ; u16 HI,LO: the editor's buffer
V_EMAX  EQU     $1AEC           ; editor max length
V_ELEN  EQU     $1AED           ; editor current length
V_GX    EQU     $1AEE           ; editor grid cursor
V_GY    EQU     $1AEF
V_POS   EQU     $1AF0           ; u16 HI,LO: CDPOS's position
V_MAXL  EQU     $1AF2           ; CDE16's maxlen across CDPOS
V_TXN   EQU     $1AF3           ; payload bytes streamed (CPAD's count)
V_BSRC  EQU     $1AF4           ; boot came from: 0 browser, 1 lobby
V_SLEN  EQU     $1AF5           ; copy: source length
V_BOFF  EQU     $1AF6           ; copy: basename offset in V_SRC
V_BLEN  EQU     $1AF7           ; copy: basename length
V_XMOD  EQU     $1AF8           ; copy: extension mode
V_CTRY  EQU     $1AF9           ; connect polls so far
V_SCUR  EQU     $1AFA           ; browser cursor across a redraw
V_ERRC  EQU     $1AFB           ; the code UFAIL shows
V_NLEN  EQU     $1AFC           ; SNAMLN's result
;       $1AFD-$1AFF spare

; -- per-phase aliases of the scratch cells --
V_LTIM  EQU     V_T6            ; LCOMMIT: 64-frame units left
V_LSEQ  EQU     V_T7            ;   the ACKSEQ it waits for
V_LTIC  EQU     V_T8            ;   frames left in this unit

PATHMAX EQU     87              ; V_PATH / V_SRC capacity less the NUL
FILTMAX EQU     15

; ---- cartridge header ---------------------------------------------------
        ORG     $0000
        BCTA,UN START
        DB      $17             ; RETC,UN: the interrupt guard at $0003

; ---- zero-page vectors -----------------------------------------------------
; ZBSR / ZBRR reach $0000-$003F from ANY page in two bytes, and their
; indirect form jumps through a pointer word there -- so this table
; turns the 30 most-used unconditional calls and jumps (counted from the
; source) into 2-byte instructions: ZBSR *ZPRS is "BSTA,UN PRS".
ZPRS:    DWBE    PRS
ZHOSTPG: DWBE    HOSTPG
ZMPARB:  DWBE    MPARB
ZWAITVB: DWBE    WAITVB
ZMSTART: DWBE    MSTART
ZCDO0:   DWBE    CDO0
ZDPUTC:  DWBE    DPUTC
ZUTITLE: DWBE    UTITLE
ZUFAIL:  DWBE    UFAIL
ZBEEP:   DWBE    BEEP
ZDPRTN:  DWBE    DPRTN
ZCTXSTR: DWBE    CTXSTR
ZSNDSEL: DWBE    SNDSEL
ZSNDBCK: DWBE    SNDBCK
ZSFDRAW: DWBE    SFDRAW
ZROWCEL: DWBE    ROWCEL
ZBARPCT: DWBE    BARPCT
ZCGO:    DWBE    CGO
ZUMSG:   DWBE    UMSG
ZELOOP:  DWBE    ELOOP
ZDTAIL:  DWBE    DTAIL
ZLKEY:   DWBE    LKEY
ZWSCAN:  DWBE    WSCAN
ZKEYSCN: DWBE    KEYSCN
ZSFULL:  DWBE    SFULL
ZSNDERR: DWBE    SNDERR
ZSFRSET: DWBE    SFRSET
ZDROWS:  DWBE    DROWS
ZDHILIT: DWBE    DHILIT
ZDHEX:   DWBE    DHEX

START:  LODI,R0 $20             ; interrupts inhibited
        LPSU
        LODI,R0 $02             ; COM=1: logical compares; WC off
        LPSL
        EORZ    R0              ; RESET leaves RAM arbitrary
        STRA,R0 AXSEL
        STRA,R0 V_FRM
        STRA,R0 V_SWP
        STRA,R0 V_HOST
        STRA,R0 V_CMODE
        STRA,R0 V_FILT
        STRA,R0 V_PNUM
        STRA,R0 V_PHEAD
        STRA,R0 V_SCUR
        LODI,R0 JOYCTR          ; a cache that starts centred cannot
        STRA,R0 AXV             ;   report a phantom direction
        STRA,R0 AXH
        LODI,R0 KNONE
        STRA,R0 V_KEY
        BSTA,UN SRESET          ; path "/", paging cleared
        BSTA,UN DINI
    IF BARTEST
        BCTA,UN BTEST
    ENDIF
        BSTA,UN MCHECK
        BCTA,EQ CKWIFI
        PRINT   TNOCRT          ; no mailbox: show what was read, halt
        DAT     6,6
        LODA,R0 *FMAGFP
        ZBSR    *ZDHEX
        LODI,R1 1
        LODA,R0 *FMAGFP,R1
        ZBSR    *ZDHEX
HALTNC: BCTR,UN HALTNC

; ---- block 1 ------------------------------------------------------------
MB_DISP:
        INCLUDE "disp.inc"
MB_INPUT:
        INCLUDE "input.inc"
MB_BAR:
        INCLUDE "bar.inc"
MB_TXLIB:
        INCLUDE "txlib.inc"
MB_UI:
        INCLUDE "ui.inc"
MB_EDIT:
        INCLUDE "edit.inc"
MB_HOSTS:
        INCLUDE "hosts.inc"
MB_BROWSE:
        INCLUDE "browse.inc"
MB_COPY:
        INCLUDE "copy.inc"
    IF BARTEST
MB_BTEST:
        INCLUDE "bartest.inc"
    ENDIF
MB_END1:

; ---- block 2 ------------------------------------------------------------
        ORG     $2000
MB_MAILBOX:
        INCLUDE "mailbox.inc"
MB_WIFI:
        INCLUDE "wifi.inc"
MB_BOOT:
        INCLUDE "boot.inc"
MB_SOUND:
        INCLUDE "sound.inc"
MB_TABLES:
        INCLUDE "tables.inc"
MB_GLYPHS:
        INCLUDE "glyphs.inc"
MB_STRING:
        INCLUDE "strings.inc"
MB_END2:
