; cfgdefs.inc -- what all three CONFIG banks agree on.
;
; Equates only, so this is included BEFORE the ORG, next to fujinet.inc.
;
; The banks do not share code, they share ZERO PAGE. A bank switch replaces
; every byte of $1000-$17FF -- there is no fixed code region to put a common
; library in, because the high 2K is the mailbox in its entirety -- so each
; bank carries its own copy of fujilib and fujidisp and the only thing that
; crosses is these variables and whatever the cartridge is holding.
;
; What the cartridge is holding matters more than what is here: the working
; directory is 256 bytes in the cart (see FN_HOT_PATH_CH), so descending into
; a subfolder costs this console nothing at all.

; The bank trampoline, at a FIXED address because the banks are assembled
; separately and each has to be able to call it by number. It is the first
; thing in the fixed tail, so cfgtail.asm ORGs here and the address cannot
; drift away from this equate.
CFGOTO  EQU     FNTAIL

; The rest of the tail's entry points, for the same reason. config.asm ORGs
; each one at its equate and fails the build if the routine before it runs
; into it.
SNDPOL  EQU     $1F27           ; FNGO's spin: a sound tick once a frame's time
SNDERR  EQU     $1F50           ; play the error beep
SNDMNU  EQU     $1F54           ; play the menu's two-tone
SNDCUE  EQU     $1F56           ; play cue A
SNDKEY  EQU     $1F6A           ; a click for input mask A; A is preserved
VBTAIL  EQU     $1F7C           ; the frame's blanking work, then VBLANK off

; Cue offsets into the tail's SNDTAB, checked there.
CUETIK  EQU     1               ; a stick step
CUEBLP  EQU     4               ; FIRE
CUEMNU  EQU     7               ; SELECT opens a menu
CUEERR  EQU     11              ; an error line
CUEUP   EQU     15              ; a boot begins
CUEOK   EQU     21              ; the image is in

; ---------------- the display ----------------
; FN_T_PLANE(0..5) from firmware/include/fuji_mailbox.h. Here rather than in
; fujidisp.inc because the tail reads them too, and the tail cannot include
; code.
TP0     EQU     $1800
TP1     EQU     $1880
TP2     EQU     $1900
TP3     EQU     $1980
TP4     EQU     $1A00
TP5     EQU     $1A80

TROWS   EQU     21              ; FN_T_ROWS
TCELLH  EQU     6               ; FN_T_CELL_H; the sixth line is always blank
TLINES  EQU     TROWS*TCELLH    ; 126 scanlines of text

; NTSC colours. The frame and the bar share COLUPF, so a bar row's stretch of
; the frame sides takes the bar colour.
        IFDEF   MONO
COLFRM  EQU     $01             ; black on screen, nonzero for the kernel
COLBAR  EQU     $01
COLTTL  EQU     $0E
COLPTH  EQU     $0E
COLTXT  EQU     $0E
COLERR  EQU     $0E
COLHNT  EQU     $0E
COLHI   EQU     $0E
        ELSE
COLFRM  EQU     $9A             ; light blue
COLBAR  EQU     $84             ; blue: the selection bar and the title strip
COLTTL  EQU     $1E             ; gold
COLPTH  EQU     $9C             ; light blue
COLTXT  EQU     $0E             ; white
COLERR  EQU     $46             ; red
COLHNT  EQU     $08             ; grey
COLHI   EQU     $C8             ; green: the progress bar, the keyboard caret
        ENDIF

; RCOLS -- a bank's row colour table, RCOLTB: the title, the path row, the
; list, and the bank's own error and hint rows. A bank with any other shape
; writes its 21 bytes out instead.
RCOLS   MACRO   ERR,HINT,{GLOBALSYMBOLS}
RCOLTB:
RCI     SET     0
        REPT    TROWS
        IF      RCI=0
        DB      COLTTL
        ELSEIF  RCI=1
        DB      COLPTH
        ELSEIF  RCI=ERR
        DB      COLERR
        ELSEIF  RCI=HINT
        DB      COLHNT
        ELSE
        DB      COLTXT
        ENDIF
RCI     SET     RCI+1
        ENDM
        ENDM

BANKHST EQU     0               ; host slots
BANKDIR EQU     1               ; the directory browser
BANKINF EQU     2               ; adapter info
BANKEDT EQU     3               ; the on-screen keyboard
BANKWIF EQU     4               ; wifi: check, scan, pick, connect
BANKBOT EQU     5               ; mount and boot, with a progress bar
BANKCPY EQU     6               ; host-to-host copy

; CFCOPY: where a copy has got to. The source's full path lives in cartridge
; buffer 2 from the moment it is marked, which is what lets the user browse
; anywhere -- including onto another host -- without losing it.
CP_IDLE EQU     0
CP_MARK EQU     1               ; a source is marked; pick a destination
CP_DONE EQU     2

; The boot bank's contract, and it is the whole reason it can be a bank of its
; own: THE CARTRIDGE'S PATH BUFFER 0 ALREADY HOLDS THE FULL PATH of the thing
; to boot. The browser appends the filename before it leaves; the lobby builds
; the path from a literal. Boot then needs no argument at all, and on failure
; FP_POP drops the filename and puts the browser back where it was.
BT_GO   EQU     0
BT_FAIL EQU     1
BT_JING EQU     2               ; the image is in; its cue is playing

; The wifi bank's sub-states, in CFSUB. Cold start lands on WS_CHECK because
; CFCOLD zeroes the zero page and WS_CHECK is zero -- which is deliberate:
; this is the first bank the cold stub enters.
WS_CHECK EQU    0
WS_SCAN EQU     1
WS_SELECT EQU   2
WS_CONN EQU     3
WS_FAIL EQU     4

WSMAX   EQU     12              ; networks listed; one screen, no paging
WSROW0  EQU     2               ; first list row; the OTHER row follows them

; Which field the keyboard is editing, in CFEFLD, and what it reports back in
; CFSUB. SSID and the passphrase commit into path buffers 0 and 1; the filter
; into buffer 1 as well, which is safe because WiFi setup and browsing never
; overlap. A host name commits nowhere -- the hosts bank streams it straight
; out of the scratch into the WRITE_HOST_SLOTS payload, so it never needs a
; home of its own.
ED_SSID EQU     0
ED_PASS EQU     1
ED_HOST EQU     2
ED_FILT EQU     3

; CFFLAG bits.
CFF_FILT EQU    $01             ; a filter is set
CFF_WIFI EQU    $02             ; wifi is enabled in the adapter's config
CFF_EDIT EQU    $04             ; an edit is outstanding; CFSUB has its verdict

ED_NO   EQU     0               ; cancelled
ED_OK   EQU     1               ; accepted

; $80-$A6 is the swap stub's landing ground and the transport's scratch;
; $A8-$B3 is ours. The stack runs down from $FF and must stay above $C0.
CFSEL   EQU     $AA             ; cursor row on the current screen
CFCNT   EQU     $AB             ; rows actually populated
CFHOST  EQU     $AC             ; the mounted host slot
CFPAGE  EQU     $AD             ; directory page, NROWS entries each
CFDEPTH EQU     $AE             ; how far we have descended
CFERR   EQU     $AF             ; the last FNE*
CFSTEP  EQU     $B0             ; where it went wrong, for the error line
CFWANT  EQU     $B1             ; entry index being sought while re-reading
CFFULL  EQU     $B2             ; the last page filled: paging forward is legal
CFKEY   EQU     $B3             ; this frame's newly-pressed input

; $B4-$BF is the parity work's own. Screens that are never live at the same
; time share bytes, which is why several of these are named twice: the action
; menu is not up while the keyboard grid is, and neither is up while the
; browser is bounce-scrolling a filename.
CFMOPN  EQU     $B4             ; nonzero while the action menu is showing
CFSUB   EQU     $B5             ; sub-state within the current bank
CFRET   EQU     $B6             ; bank to return to from a modal screen
CFCOPY  EQU     $B7             ; 0 idle, 1 source marked, 2 picking a host
CFCSRC  EQU     $B8             ; the copy source's host slot
CFFLAG  EQU     $B9             ; bit0 filter set, bit1 wifi enabled
CFTMR   EQU     $BA             ; pauses, connect polls, idle-before-scroll
CFN     EQU     $BB             ; list length: scan results, grid rows
CFMENU  EQU     $BC             ; action-menu cursor  | browser scroll offset
CFMLEN  EQU     $BD             ; action-menu length  | browser scroll direction
CFELEN  EQU     $BE             ; characters typed so far
CFEFLD  EQU     $BF             ; which field is being edited

; Scratch, inside the swap stub's landing ground. Safe because the stub is
; copied there only by FNSWAP, which never returns -- but for the same reason
; nothing here may hold anything a boot needs.
CFTMPA  EQU     $8C
CFTMPB  EQU     $8D

; The keyboard's grid cursor, and deliberately NOT the action menu's CFMENU /
; CFMLEN. The menu is modal OVER the grid, so both cursors are live at the
; same time -- sharing them let two presses aimed at the menu walk the grid's
; column instead, and every character typed afterwards came out two along.
; Nothing else is live while the keyboard is, so the landing ground will do.
EDCOL   EQU     $8F
EDROW   EQU     $90

; The display kernel's and the sound engine's, in the same landing ground and
; safe for the same reason: the boot bank lets a cue finish before FNSWAP.
ROWN    EQU     $91             ; the text row the kernel is on
ROWEND  EQU     $92             ; the scanline that row's ink ends at
BARY    EQU     $93             ; 6 * the row with a '>' in column 0, or $FF
SNDPTR  EQU     $94             ; the next note in SNDTAB; 0 is idle
SNDDUR  EQU     $95             ; frames left on this note, minus one
SNDVOL  EQU     $96             ; AUDV0 for this cue; 0 when nothing plays
SAVSP   EQU     $97             ; the stack pointer, while the kernel uses S

HOSTSLN EQU     32              ; MAX_HOSTNAME_LEN: the stride in the reply
NHOSTS  EQU     8               ; MAX_HOST_SLOTS
DEVSLOT EQU     0               ; this console has one thing to mount into
NAMELN  EQU     30              ; READ_DIR_ENTRY maxlen. NOT 31: at exactly 31
                                ;   the firmware prepends icon bytes.
NROWS   EQU     14              ; list rows
ROW0    EQU     2               ; the first of them
ROWERR  EQU     18              ; where a failure is reported
