; ram.inc -- where everything lives.
;
; Internal RAM (64 bytes):
;   $00-$07  register bank 0: the main program
;   $08-$17  the stack, 8 levels: the interrupts take up to 2
;   $18-$1F  register bank 1: the interrupts' own (r5: the next band, r4:
;            the staged row's Y, from the vblank to the last row)
;   $20-$3C  below
;   $3D-$3F  the BIOS's (its keyboard and IRQ routines)

ROWBUF  equ 020h                ; ROW_TAB: the next row, staged
FRAMES  equ 039h                ; vblanks, for the main loop's clock
QUIET   equ 03Ah                ; frames whose last band has run
MODE    equ 03Bh                ; MODE_*
SNDCNT  equ 03Ch                ; the sound: frames left (beep.inc)

    if ROWBUF+ROW_TAB > FRAMES
    error "ROWBUF has grown into FRAMES"
    endif

MODE_SETUP equ 002h             ; a new screen's objects (fixed.inc obj_setup)
MODE_PAL   equ 080h             ; a PAL console (the boot measures it)

; External RAM (128 bytes, $00-$7F, MOVX with P1_MAIN):
LINEBUF equ 000h                ; 16: a text line being built (disp.inc)
TXCOL   equ 010h                ; its colour, and for characters with bit 7 set
TXCOL2  equ 011h
OLDPG   equ 012h                ; farcall's caller's page
CURPG   equ 013h                ; the page mapped at $800
TMP0    equ 014h                ; a loop's index across calls that keep nothing
TMP1    equ 015h
TMP2    equ 016h
; the input (input.inc)
KLAST   equ 017h                ; BIOS readkey's memory (its r7)
KEY     equ 018h                ; the key pressed this frame (K_NONE: none)
JOY     equ 019h                ; the stick now: J_* bits
JOYP    equ 01Ah                ; last frame's
JOYEV   equ 01Bh                ; pressed since last frame
REP     equ 01Ch                ; auto-repeat countdown
; the transaction (mail.inc)
NSEQ    equ 01Dh                ; the sequence number posted
LTMP    equ 01Eh                ; the library's own scratch (p1.inc)
HRET    equ 01Fh                ; the help's way back: entry << 4 | page
RICON   equ 020h                ; row_line: sprite 1 (the folder) beside the row
; the screens' shared state
RETPG   equ 021h                ; the editor's and the help's way back: page,
RETEN   equ 022h                ; ... and entry
HELPCTX equ 023h                ; HC_*: whose help
EDKIND  equ 024h                ; ED_*: what the editor edits
EDMAX   equ 025h                ; its longest
EDLEN   equ 026h                ; EDBUF's length
EDRES   equ 027h                ; 1: accepted, 0: cancelled
EDCUR   equ 028h                ; the editor's cursor
SHIFT   equ 029h                ; the editor's shift state (p4.inc)
HOST    equ 02Ah                ; the host slot picked, 0-7
SEL     equ 02Bh                ; a list's cursor row
FLAGS   equ 02Ch                ; FL_*
; a screen's own (each screen's names for the same bytes)
SCR0    equ 02Dh                ; 16 bytes, $2D-$3C
; a list's long name, scrolling (the hosts, the browser)
MQOFF   equ SCR0                ; its first character shown
MQCLK   equ SCR0+1              ; frames to its next step
MQDIR   equ SCR0+2              ; 1 or -1
; the editor (p5.inc)
EBLINK  equ SCR0+3              ; the cursor's blink, frames
EMAGC   equ SCR0+4              ; the character the magnifier shows
EDIRTY  equ SCR0+5              ; lines due (ed_lazy): bit 0 the status, 1-6
EDIRTY2 equ SCR0+6              ; the field's rows, 7 the chart's row 7; and
                                ; bit 0 the chart's T1
; the help (p8.inc)
HPAGE   equ SCR0+3              ; its page
; info (p7.inc)
IPAGE   equ SCR0+3              ; the page shown
IMAX    equ SCR0+4              ; its longest value's overhang (the scroll)
; the WiFi screens (p6.inc)
WNUM    equ SCR0                ; networks found
WTOP    equ SCR0+1              ; the first one on the screen
WSTEP   equ SCR0+2              ; WS_*: what the editor came back with
WTRY    equ SCR0+3              ; connect: tries left
WCLK    equ SCR0+4              ; frames to the next try

EDBUF   equ 040h                ; 64: the text being edited (p4.inc), and
                                ; the browser's filter while it browses
EDBUF_LEN equ 64

FL_COPY equ 01h                 ; copying: the hosts and folders pick where to
FL_FILT equ 02h                 ; the browser has a filter (EDBUF)
FL_HELPQ equ 04h                ; the help was closed with ? (the editor types it)
; the copy's source (FL_COPY): its host and its entry's place in its folder
COPYH   equ 03Dh
COPYPL  equ 03Eh
COPYPH  equ 03Fh
; the browser (p3.inc): its own, past the filter in EDBUF's first 16
FILTMAX equ 15                  ; the filter's longest
FLEN    equ EDBUF+16            ; the filter's length (FL_FILT)
POSL    equ EDBUF+17            ; the page's first entry, counting hidden ones
POSH    equ EDBUF+18
NXTL    equ EDBUF+19            ; the next page's (NXTH bit 7: none)
NXTH    equ EDBUF+20
CNTL    equ EDBUF+21            ; the folder's entries (CNTH bit 7: uncounted)
CNTH    equ EDBUF+22
NVIS    equ EDBUF+23            ; rows on the page
PGN     equ EDBUF+24            ; pages in RING
MQLEN   equ EDBUF+25            ; the bar's name's length
ROWT    equ EDBUF+26            ; 8 x 2: each row's name in the reply, offset
                                ; low, high (bit 7: a folder)
RING    equ EDBUF+42            ; 8 x 2: the pages before (POS)
RINGN   equ 8
    if RING+2*RINGN > 80h
    error "the browser's RAM has run out"
    endif
PLO     equ SCR0+3              ; the count's probe: known there (2)
PHI     equ SCR0+5              ; ... and not (2)
GOFL    equ SCR0+3              ; the parse: the group's offset (2)
GOFH    equ SCR0+4
GSZL    equ SCR0+5              ; ... its size (2)
GSZH    equ SCR0+6
BLAST   equ SCR0+3              ; the boot: the percent drawn
BCELLS  equ SCR0+4              ; ... the bar's cells filled
BSTATE  equ SCR0+5              ; ... BOOT_STATE this frame
BWAITL  equ SCR0+6              ; ... frames left without progress (2)
BNAML   equ SCR0+11             ; the boot's file: its name in the reply (2)
BNAMH   equ SCR0+12
