; boot.inc -- SET_DEVICE_FULLPATH + MOUNT_IMAGE with a progress bar,
; then the swap (block 2: page-0 RAM through mailbox.inc's pointer
; words, the status page direct).
;
; MOUNT_IMAGE makes the ESP32 pull the file and push it to the cart
; over the DBC side-channel BEFORE it ACKs, so the commit is done by
; hand (BCOMMIT): SEQ written, then -- while ACKSEQ is still pending --
; only the status page is read (boot state / percent, never a hotspot,
; which would queue a transaction) and the bar repainted. The cart's
; own deadline for a mount is 60 s; ours is 75 s with no progress.
; Under MAME the push runs inside the SEQ read itself, so the bar can
; only jump to 100%.
;
; This cart's firmware does not reset the boot registers when a mount
; starts, so the verdict is read AFTER the ACK, and a FAILED before it
; is ignored. ACK + still IDLE means the server mounted the file as a
; plain disk and pushed nothing (not a cartridge image). A READY left
; over from a RESET between READY and the swap reads as fresh; nothing
; on this protocol version can tell the two apart.
;
; On READY: BOOTLOCK, the 11-byte stub into RAM at $18E0, and a jump to
; it. The cart serves the staged image from the stub's read of the swap
; hotspot on; the stub's BCTA to $0000 lands in it. CONFIG_BOOT ($D9)
; is never sent: CONFIG lives in the cart's flash.

STUB    EQU     $18E0
BTIMO   EQU     75*60           ; frames with no progress before giving up

; BOOTSEL: boot the browser's cursor row from V_HOST.
BOOTSEL: ZBSR   *ZSFULL
        BCFA,EQ SFERR
        EORZ    R0
        STRA,R0 *PVBSRC
        BSTA,UN BTSCR
        SETST1  FNRDATA         ; the name, before any commit repaints it
        BSTA,UN BTNAME
        BSTA,UN BTSDFP          ; path = V_PATH + the window name
        SETFS1  V_PATH
        ZBSR    *ZCTXSTR
        SETFS1  FNRDATA
        ZBSR    *ZCTXSTR
        BSTA,UN BTSDF2
        BSTA,UN CCDIR
        BCTR,UN BOOTMI

; BOOTLB: boot the lobby image from V_HOST (already mounted).
BOOTLB: LODI,R0 1
        STRA,R0 *PVBSRC
        BSTA,UN BTSCR
        SETST1  TLOBP
        BSTA,UN BTNAME
        BSTA,UN BTSDFP
        SETFS1  TLOBP
        ZBSR    *ZCTXSTR
        BSTA,UN BTSDF2

BOOTMI: EORZ    R0              ; the empty bar
        ZBSR    *ZBARPCT
        LODI,R3 FCMIMG
        LODI,R2 2
        ZBSR    *ZMSTART
        LODI,R1 0               ; device slot 0
        ZBSR    *ZMPARB
        LODI,R1 1               ; read-only
        ZBSR    *ZMPARB
        BSTA,UN BCOMMIT
        BCTR,EQ BTVERD
        STRZ    R2              ; a NAK or timeout: the cart's verdict,
        LODA,R0 FNBSTAT         ;   if it failed the push, names the
        COMI,R0 FBFAIL          ;   cause (it refuses an image > 8K at
        BCTA,EQ BTBERR          ;   the stream open, and the server
        LODZ    R2              ;   then NAKs the mount)
        BCTA,UN BTMERR
BTVERD: LODA,R0 FNBSTAT         ; the verdict, now that it is fresh
        COMI,R0 FBREADY
        BCTA,EQ BREADY
        COMI,R0 FBFAIL
        BCTA,EQ BTBERR
        COMI,R0 FBIDLE
        BCTA,EQ BTNCRT
        LODI,R3 0               ; still XFER: up to ~30 s more
BTW1:   LODI,R2 7
BTW2:   ZBSR    *ZWAITVB
        BDRR,R2 BTW2
        LODA,R0 FNBSTAT
        COMI,R0 FBREADY
        BCTR,EQ BREADY
        COMI,R0 FBFAIL
        BCTA,EQ BTBERR
        BDRR,R3 BTW1
        LODI,R0 FETIMO
        BCTA,UN BTMERR

BREADY: LODI,R0 100
        ZBSR    *ZBARPCT
        BSTA,UN SNDFUJI
        EORZ    R0              ; a silent machine for the next image
        STRA,R0 *PPITCH         ;   (this also leaves multicolor)
        STRA,R0 *PVOLUM
        LODI,R1 FRBOOTL
        LODI,R2 FNBLMAG
        BSTA,UN MREGWR
        LODI,R3 STUBLEN
STCPY:  LODA,R0 STUBSRC-1,R3
        STRA,R0 *PSTUBM,R3
        BDRR,R3 STCPY
        BCTA,UN STUB

; The stub, as bytes: it runs at $18E0, and AS would refuse its
; page-0 operand assembled here in page 1.
;       EORZ R0 / LPSU / LPSL       PSU/PSL as at power-on
;       LODA,R0 *$18E9              the swap read: the cart flips here
;       BCTA,UN $0000               into the staged image
;       DWBE FNSWAP                 (the pointer word at $18E9)
STUBSRC: DB     $20,$92,$93,$0C,$98,$E9,$1F,$00,$00
        DWBE    FNSWAP
STUBLEN EQU     $-STUBSRC

; the failures: a message and back where the boot came from.
BTBERR: LODA,R0 FNBERR
        STRZ    R2
        COMI,R0 1
        BCFR,EQ BTBE2
        LODI,R0 EBTOOB>>8
        LODI,R1 EBTOOB&$FF
        BCTR,UN BTFAIL
BTBE2:  COMI,R0 2
        BCFR,EQ BTBE3
        LODI,R0 EBTRNC>>8
        LODI,R1 EBTRNC&$FF
        BCTR,UN BTFAIL
BTBE3:  LODI,R0 ESTAGE>>8
        LODI,R1 ESTAGE&$FF
        BCTR,UN BTFAIL
BTNCRT: STRZ    R2
        LODI,R0 ENCART>>8
        LODI,R1 ENCART&$FF
        BCTR,UN BTFAIL
BTPERR: STRZ    R2
        LODI,R0 EPATH>>8
        LODI,R1 EPATH&$FF
        BCTR,UN BTFAIL
BTMERR: STRZ    R2
        LODI,R0 EMOUNT>>8
        LODI,R1 EMOUNT&$FF
BTFAIL: ZBSR    *ZUFAIL
        LODA,R0 *PVBSRC
        BCTA,EQ SFDRAW
        ZBRR    *ZHOSTPG

; BTSCR: the boot screen frame. Calls three down.
BTSCR:  LODI,R0 TBOOT>>8
        LODI,R1 TBOOT&$FF
        ZBSR    *ZUTITLE
        PRINT   TNOPOW
        RETC,UN

; BTNAME: the tail of the string at (DSTRP) on row 2. Calls two down.
BTNAME: DCOL1   AACC
        DAT     2,0
        LODI,R1 16
        ZBRR    *ZDTAIL

; BTSDFP: begin SET_DEVICE_FULLPATH(slot 0, V_HOST, read); the caller
; streams the path, then BTSDF2 pads and commits it (a failure does not
; return: it reports and leaves). Calls two down.
BTSDFP: LODI,R3 FCSDFP
        LODI,R2 3
        ZBSR    *ZMSTART
        LODI,R1 0
        ZBSR    *ZMPARB
        LODA,R1 *PVHOST
        ZBSR    *ZMPARB
        LODI,R1 1
        ZBSR    *ZMPARB
        EORZ    R0
        STRA,R0 *PVTXN
        RETC,UN
BTSDF2: BSTA,UN CPAD
        ZBSR    *ZCGO
        BCFA,EQ BTPERR
        RETC,UN

; BCOMMIT: commit the staged MOUNT_IMAGE and repaint the bar from the
; status page while it is outstanding. Results as CGO. Calls three down.
; Clobbers R0-R3, V_T2-V_T5 (V_T2 doubles as BARPCT's own copy of the
; percent, which is the same value).
BCOMMIT: LODI,R0 $FF
        STRA,R0 *PVT2           ; the percent last drawn
        EORZ    R0
        STRA,R0 *PVT4           ; frames without progress, HI,LO
        STRA,R0 *PVT5
        BSTA,UN MSEQ
        STRA,R0 *PVT3           ; the ACKSEQ that means "done"
BCL:    LODA,R0 FNACKSQ
        COMA,R0 *PVT3
        BCTA,EQ BCACK
        LODA,R0 FNBPCT
        COMA,R0 *PVT2
        BCTR,EQ BCL1
        STRA,R0 *PVT2           ; progress: repaint, restart the clock
        ZBSR    *ZBARPCT
        EORZ    R0
        STRA,R0 *PVT4
        STRA,R0 *PVT5
BCL1:   ZBSR    *ZWAITVB
        LODA,R0 *PVT5
        ADDI,R0 1
        STRA,R0 *PVT5
        BCFR,EQ BCL2
        LODA,R0 *PVT4
        ADDI,R0 1
        STRA,R0 *PVT4
BCL2:   LODA,R0 *PVT4
        COMI,R0 BTIMO>>8
        BCFR,EQ BCL
        LODA,R0 *PVT5
        COMI,R0 BTIMO&$FF
        BCFR,EQ BCL
        LODI,R0 FETIMO
        RETC,UN
BCACK:  LODA,R0 FNERR
        BCTA,UN CGOCHK
