; config.asm -- FujiNet CONFIG for the Bally Astrocade.
;
; The full-feature CONFIG client: WiFi setup (scan / custom SSID / on-screen
; password entry / connect), the 8 host slots (list, mount, rename), directory
; browsing (paging, subfolders, .cfg suppression, filtering), copying a file
; between hosts, an info screen, and boot with a progress bar -- the Astrocade
; counterpart of fujinet-config/intv, grown from the bring-up's testrom/
; fujicfg.asm and speaking the same cartridge mailbox (fujilib.inc).
;
; RESET on the console restarts this program but not the cart, which is why
; sequence numbers always derive from ACKSEQ (see fujilib.inc) and why no
; screen assumes RAM survived.
;
; Modules (INCLUDEd below, one translation unit):
;   fujicmd.inc  command wrappers on the fujilib transport
;   input.inc    hand controller + 24-key keypad polling, repeat, events
;   font.inc     byte-aligned 5x7 blitter; BIOS glyphs + lowercase set
;   ui.inc       rows, chevron cursor, footer, decimal, memset
;   edit.inc     the on-screen character-grid text editor
;   wifi.inc     ST_CHECK_WIFI / ST_CONNECT_WIFI / ST_SET_WIFI
;   hosts.inc    host slots: list / mount / rename / lobby / copy skin
;   browse.inc   ST_SELECT_FILE: paging, subdirs, filter, copy hooks
;   copy.inc     COPY_FILE with .cfg sibling, return-to-source
;   info.inc     adapter config display
;   boot.inc     SET_DEVICE_FULLPATH + MOUNT_IMAGE w/ progress, swap

        INCLUDE "HVGLIB.H"

; ---- geometry ---------------------------------------------------------
; LINES=80 frees 4C80H-4FFFH (896 bytes) for variables while keeping ten
; 8-pixel text rows: title, eight list rows, footer.
LINES   EQU     80
NROWS   EQU     8
TITLEY  EQU     1               ; y of the title row
LISTY0  EQU     10              ; y of list row 0
FOOTY   EQU     73              ; y of the footer row
LISTX   EQU     8               ; x (pixels) where list text starts
NAMELEN EQU     24              ; display width of a directory entry
LISTLEN EQU     36              ; READ_DIR_ENTRY maxlen for listings: enough
                                ; that the .cfg/.bin/'/' suffix survives the
                                ; crunch, and (being != 31) keeps the firmware
                                ; from prepending icon bytes
FULLLEN EQU     120             ; re-read width for paths that must open
DEVSLOT EQU     0

; ---- RAM map (screen RAM above the 80 visible lines) ------------------
; The V_SRC/V_SSID union is safe by construction: the WiFi screens are
; unreachable once ST_HOSTS has been entered, and copy (the V_SRC user)
; can only start from the file browser.
V_PATH  EQU     4C80H           ; 192  current dir path, NUL-term, leading '/'
V_SRC   EQU     4D40H           ; 224  copy-source full path / boot path
V_SSID  EQU     4D40H           ;  33  \ union with V_SRC (wifi phase only)
V_PASS  EQU     4D61H           ;  64  /
V_ENTRY EQU     4E20H           ; 128  editor accumulator / full-name re-read
V_FILT  EQU     4EA0H           ;  32  directory filter ("" = none)
V_EPOS  EQU     4EC0H           ;  16  per-row absolute dir position, 8 x 2 LE
V_EDIR  EQU     4ED0H           ;   8  per-row type: 0 file / 1 dir / 2 cart
V_PSTK  EQU     4ED8H           ;  16  page-start stack, 8 x 2 LE
; Persistent cursor / input state sits just below the stack, the way the
; bring-up's fujicfg keeps its state at 4F40H+; the big buffers stay lower.
; 4EE8H-4EFFH is unused spare.
V_TMP   EQU     4F00H           ; 8  scratch (rebuilt each use)
LINBUF  EQU     4F08H           ; 40  display line being built
HEXBUF  EQU     4F30H           ; 8
V_HOST  EQU     4F38H           ; selected host slot
V_CUR   EQU     4F39H           ; cursor row
V_CNT   EQU     4F3AH           ; entries on this page
V_DPOS  EQU     4F3BH           ; 2  dir position of the top row
V_DNEXT EQU     4F3DH           ; 2  dir position after the last row
V_PDEP  EQU     4F3FH           ; page stack depth
V_CMODE EQU     4F40H           ; 0 normal, 1 = picking a copy destination
V_CHOST EQU     4F41H           ; copy-source host slot
V_NNET  EQU     4F42H           ; networks found by the last scan
V_GX    EQU     4F43H           ; editor grid cursor
V_GY    EQU     4F44H
V_GLEN  EQU     4F45H           ; editor value length
V_GMAX  EQU     4F46H
V_GCASE EQU     4F47H           ; 0 upper, 1 lower
V_PDIR  EQU     4F48H           ; input: last emitted direction bit
V_PTRG  EQU     4F49H           ; input: previous trigger state
V_PKP   EQU     4F4AH           ; 4  input: previous keypad columns
V_RPT   EQU     4F4EH           ; input: auto-repeat countdown
; 4F4FH-4F5FH spare; stack floor 4F60H (96-byte budget, shallow call tree)
STACK   EQU     4FC0H
STUB    EQU     4FE0H           ; boot swap stub (6 bytes, runs from RAM)

; STRDIS/CHRDIS option bytes: bits 2-3 = fg color, bits 0-1 = bg color.
OPTFB   EQU     0CH             ; fg 3 (white) on bg 0
OPTHI   EQU     04H             ; fg 1 (accent)
OPTDIM  EQU     08H             ; fg 2 (dim)

; Blitter color specs: bits 1-0 = fg pixel code, bits 3-2 = bg pixel code.
GCNORM  EQU     03H             ; white on black
GCINV   EQU     0CH             ; black on white (cursor cell)

        ORG     FIRSTC
        DB      55H             ; menued-cartridge sentinel
        DW      MENUST          ; chain to the on-board SELECT GAME list
        DW      PRGNAM
        DW      PRGSTR
PRGNAM: DB      "FUJINET CONFIG"
        DB      0

PRGSTR: DI
        LD      SP,STACK
        SYSTEM  INTPC
        DO      SETOUT
        DB      LINES*2
        DB      0
        DB      8
        DO      COLSET
        DW      PALET
        DO      FILL            ; clear past the visible lines through the
        DW      NORMEM          ; variable region (0x4000-0x4EFF): the vars
        DW      0F00H           ; live in screen RAM above the display, where
        DB      0               ; the SELECT GAME menu leaves text that would
        EXIT                    ; otherwise read back as stale values. Stops
                                ; short of 0x4F00 so the running interpreter's
                                ; stack (near 0x4FC0) is left intact.

        XOR     A
        LD      (V_FILT),A
        LD      (V_CMODE),A
        LD      (V_HOST),A
        LD      (V_PDIR),A
        LD      (V_PTRG),A
        LD      (V_RPT),A
        LD      (V_PKP),A
        LD      (V_PKP+1),A
        LD      (V_PKP+2),A
        LD      (V_PKP+3),A
        LD      HL,V_PATH       ; path = "/"
        LD      (HL),'/'
        INC     HL
        LD      (HL),A

        CALL    FNCHECK
        JP      Z,CKWIFI

; No mailbox: the EPROM-diagnostic path. Show what we read and halt.
NOCARD: LD      HL,TNOCART
        CALL    UTITLE
        LD      A,(FNMAGF)
        LD      DE,LISTY0*256+40
        CALL    HEXAT
        LD      A,(FNMAGN)
        LD      DE,LISTY0*256+56
        CALL    HEXAT
HALTE:  JR      HALTE

; ---- shared failure helpers ------------------------------------------
; UFAIL: footer = message at HL + the code in A as hex, pause ~2 s.
; Returns to the caller, which decides where to go next.
UFAIL:  PUSH    AF
        CALL    UFOOT
        POP     AF
        LD      DE,FOOTY*256+140
        CALL    HEXAT
        JR      UPZ2
; UMSG: footer = message at HL, pause ~2 s (input skips).
UMSG:   CALL    UFOOT
UPZ2:   LD      DE,200
        JP      UPAUSE

TNOCART: DB     "NO FUJINET CART",0

PALET:  DB      0E7H,0F4H,7AH,00H       ; COL3..COL0: white/amber/blue/black
        DB      0E7H,0F4H,7AH,00H

        INCLUDE "fujicmd.inc"
        INCLUDE "input.inc"
        INCLUDE "font.inc"
        INCLUDE "ui.inc"
        INCLUDE "edit.inc"
        INCLUDE "wifi.inc"
        INCLUDE "hosts.inc"
        INCLUDE "browse.inc"
        INCLUDE "copy.inc"
        INCLUDE "info.inc"
        INCLUDE "boot.inc"
        INCLUDE "fujilib.inc"
        INCLUDE "fujidisp.inc"
