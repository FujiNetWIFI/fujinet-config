; cfgdefs.inc -- CONFIG's equates, RAM ledger and macros. Include after
; s2macro.inc and before any ORG.

; ---- FujiBus commands beyond fujinet.inc's ----
FNCRHST EQU     0F4H            ; READ_HOST_SLOTS: 8 x 32 names
FNCWHST EQU     0F3H            ; WRITE_HOST_SLOTS
FNCODIR EQU     0F7H            ; OPEN_DIRECTORY
FNCRDIR EQU     0F6H            ; READ_DIR_ENTRY
FNCCDIR EQU     0F5H            ; CLOSE_DIRECTORY
FNCSPOS EQU     0E4H            ; SET_DIRECTORY_POSITION
FNCSSID EQU     0FBH            ; SET_SSID: ssid[33] + password[64]
FNCSCAN EQU     0FDH            ; SCAN_NETWORKS
FNCSRES EQU     0FCH            ; GET_SCAN_RESULT: ssid[33], rssi
FNCWST  EQU     0FAH            ; GET_WIFISTATUS: 3 = connected
FNCCOPY EQU     0D8H            ; COPY_FILE: "src|dst", 1-based hosts

; GET_ADAPTERCONFIG_EXTENDED (fujiDevice.h, packed)
AC_SSID EQU     0               ; char[33]
AC_MAC  EQU     113             ; uint8_t[6]
AC_VER  EQU     125             ; char[15]
AC_SIP  EQU     140             ; char[16]

NROWS   EQU     8               ; list rows: screen rows 1-8

; READ_DIR_ENTRY in block mode: 4 pages (the whole reply window), groups of
; NROWS entries, each with full-length names and a directory flag.
DIRPAGES EQU    4
DIRMODE EQU     0C0H|NROWS
GL_DIR  EQU     80H             ; the folder glyph (def_glyphs)

ED_SSID EQU     0               ; what st_edit is filling in
ED_PASS EQU     1
ED_HOST EQU     2

; ---- RAM ledger: console RAM $0800-$09FF ----
; $0800-$080F  runtime (s2macro.inc V_*)
; $0810-$082F  the state below (32)
; $0830-$083F  V_EOFF (16)
; $0840-$0860  V_SSID (33)
; $0870-$08B0  V_EDIT (65)
; $0900-$09BF  V_PATH (192)
; $09C0-$09FF  stack (64), R2 from $09FF down
V_CUR   EQU     V_APP+00H       ; list cursor, 0..NROWS-1
V_NROW  EQU     V_APP+01H       ; rows on this page
V_OLD   EQU     V_APP+02H       ; the cursor before a move
V_ROW   EQU     V_APP+03H       ; the row a painter is drawing
V_I     EQU     V_APP+04H       ; a screen loop's counter
V_HOST  EQU     V_APP+05H       ; host slot being browsed
V_POSL  EQU     V_APP+06H       ; directory position of this page, LE
V_POSH  EQU     V_APP+07H
V_MORE  EQU     V_APP+08H       ; 1: a next page exists
V_NGRP  EQU     V_APP+09H       ; fb_parse: groups in the reply
V_GFLG  EQU     V_APP+0AH       ; fb_parse: the first group's flags
V_GCNT  EQU     V_APP+0BH       ; fb_parse: its entries
V_SCANN EQU     V_APP+0CH       ; networks the scan found
V_SCANB EQU     V_APP+0DH       ; first network on this page
V_EDWHAT EQU    V_APP+0EH       ; ED_*
V_EDLEN EQU     V_APP+0FH       ; characters in V_EDIT
V_EDMAX EQU     V_APP+10H       ; field width
V_OSK   EQU     V_APP+11H       ; keyboard cell 0-95: character $20 + cell
V_HSEL  EQU     V_APP+12H       ; slot being renamed, or the copy destination
V_MODE  EQU     V_APP+13H       ; 1: the host list picks a copy destination
V_SRCH  EQU     V_APP+14H       ; copy source host
V_FCUR  EQU     V_APP+15H       ; the browser's cursor while copying
V_ARG   EQU     V_APP+16H       ; a command wrapper's argument
V_N     EQU     V_APP+17H       ; s_copy's count
V_KOLD  EQU     V_APP+18H       ; key_wait: V_KLAST before in_get
V_KTO   EQU     V_APP+19H       ; key_wait: 1 when a timeout runs
V_PEND  EQU     V_APP+1AH       ; path_add: where V_PATH ended
V_SEEN  EQU     V_APP+1BH       ; 1 once the status screen has been shown
V_IRET  EQU     V_APP+1CH       ; 2: where st_info returns (hi, lo)
V_EOFF  EQU     V_APP+20H       ; 16: this page's name offsets in the reply (hi, lo)
V_SSID  EQU     V_APP+30H       ; 33
V_EDIT  EQU     V_APP+60H       ; 65: the editor's text
V_PATH  EQU     0900H           ; PATHMAX, page-aligned: its low byte is an index
PATHMAX EQU     192

        IF      (V_EDIT+65 > 0900H) || (V_PATH+PATHMAX > 09C0H)
        ERROR   "RAM ledger overlaps"
        ENDIF

; ---- macros ----
; RB.1 is HI(V_APP) for the whole run: every variable above is one LDI away.

; D = var.
LDV     MACRO   var
        LDI     LO(var)
        PLO     RB
        LDN     RB
        ENDM

; var = D. Leaves D, and RB on var. Uses RD.1.
STV     MACRO   var
        PHI     RD
        LDI     LO(var)
        PLO     RB
        GHI     RD
        STR     RB
        ENDM

; var = constant.
SETV    MACRO   var, val
        LDI     LO(var)
        PLO     RB
        LDI     val
        STR     RB
        ENDM

; RB = var.
PTV     MACRO   var
        LDI     LO(var)
        PLO     RB
        ENDM

; The 2-byte var = adr, high byte first (go_ind).
SETW    MACRO   var, adr
        LDI     LO(var)
        PLO     RB
        LDI     HI(adr)
        STR     RB
        INC     RB
        LDI     LO(adr)
        STR     RB
        ENDM

; Every state starts with an empty stack: R2.1 never leaves $09.
STATE   MACRO
        LDI     LO(STACK)
        PLO     R2
        ENDM

; One kdisp table entry.
KEYTO   MACRO   k, adr
        DB      k
        DW      adr
        ENDM

; FIT that also keeps out of $x000-$x3FF: `size` bytes start on the next
; page if they would cross one, and at $x400 if that page is $x000-$x3FF.
RFIT    MACRO   size
        IF      ((($) & 0FFH) + (size)) > 100H
        ORG     (($) + 0FFH) & 0FF00H
        ENDIF
        IF      (($) & 0C00H) == 0
        ORG     (($) & 0F000H) + 0400H
        ENDIF
        ENDM
