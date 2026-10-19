; hosts.inc -- the host slot page: list, mount, rename, lobby, and the
; copy-destination skin (block 1).
;
; The list is drawn straight out of the READ_HOST_SLOTS reply window (8
; records x 32 bytes = exactly slice 0); nothing is buffered. The same
; stay-painted property powers rename and the lobby's slot claim:
; WRITE_HOST_SLOTS is streamed byte for byte FROM the window, with the
; edited slot substituted from V_HNAME, so no 256-byte RAM copy exists.
; The one rule: no commit between the fresh READ and the streamed WRITE.
;
; In copy mode (V_CMODE = 1) the page re-skins itself as the
; destination picker, as on the Astrocade and Intellivision.

HOSTPG: LODI,R0 16
        STRA,R0 V_HLW
        LODA,R0 V_CMODE
        BCFR,EQ HP0
        LODI,R0 THOSTS>>8
        LODI,R1 THOSTS&$FF
        BCTR,UN HP1
HP0:    LODI,R0 TCPYH>>8
        LODI,R1 TCPYH&$FF
HP1:    ZBSR    *ZUTITLE
        LODI,R3 FCRHSL
        ZBSR    *ZCDO0
        BCTR,EQ HP2
        STRZ    R2
        LODI,R0 EHOSTS>>8
        LODI,R1 EHOSTS&$FF
        ZBSR    *ZUFAIL
        ZBRR    *ZHOSTPG          ; keep trying: the ESP32 may be booting
HP2:    EORZ    R0
HPD1:   STRA,R0 V_CUR           ; the slot being drawn (ROWCEL's input)
        ZBSR    *ZROWCEL
        DCOL    AACC            ; the slot digit
        LODA,R0 V_CUR
        ADDI,R0 $11
        ZBSR    *ZDPUTC
        DCOL    ANORM
        STRA,R0 SCREEN,R2,+     ; a blank, then the name in cols 2-15
        LODA,R0 V_CUR
        BSTA,UN HSLOT           ; DSTRP = the name; CC EQ if empty
        BCFR,EQ HPD2
        SETSTR  TEMPTY
HPD2:   LODI,R1 14
        ZBSR    *ZDPRTN
        LODA,R0 V_CUR
        ADDI,R0 1
        COMI,R0 8
        BCTR,LT HPD1
        LODA,R0 V_CMODE
        BCTR,EQ HP3
        DCOL    AACC            ; copy mode: what is being copied
        SETSTR  V_SRC
        DAT     9,0
        LODI,R1 16
        ZBSR    *ZDTAIL
        PRINT   LHCOPY
        BCTR,UN HP4
HP3:    PRINT   LHOST
HP4:    LODI,R0 8
        STRA,R0 V_CNT
        LODA,R0 V_HOST
        ANDI,R0 7               ; RESET leaves RAM arbitrary
        STRA,R0 V_CUR
        BSTA,UN LHI

HKEY:   ZBSR    *ZLKEY
        LODA,R1 V_CUR
        STRA,R1 V_HOST
        COMI,R0 KFIRE
        BCTA,EQ HKOPEN
        COMI,R0 KENT
        BCTA,EQ HKOPEN
        COMI,R0 7
        BCTA,EQ HKWIFI
        COMI,R0 KCLR
        BCTA,EQ HKCLR
        LODA,R1 V_CMODE         ; the rest only outside copy mode
        BCFR,EQ HKEY
        COMI,R0 1
        BCTA,EQ HKREN
        COMI,R0 3
        BCTA,EQ INFOPG
        COMI,R0 9
        BCTA,EQ LOBBY
        BCTR,UN HKEY

; FIRE / Enter: mount the slot and browse its root.
HKOPEN: LODA,R0 V_CUR
        BSTA,UN HSLOT
        BCTA,EQ HKBAD           ; an empty slot
        BSTA,UN CMOUNT
        BCTR,EQ HKO1
        STRZ    R2
        LODI,R0 EMHOST>>8
        LODI,R1 EMHOST&$FF
        ZBSR    *ZUFAIL
        ZBRR    *ZHOSTPG
HKO1:   ZBSR    *ZSNDSEL
        BSTA,UN SRESET
        ZBRR    *ZSFDRAW
HKBAD:  ZBSR    *ZSNDERR
        BCTA,UN HKEY

; 7: WiFi setup (abandons a copy in progress).
HKWIFI: ZBSR    *ZSNDSEL
        EORZ    R0
        STRA,R0 V_CMODE
        ZBRR    *ZWSCAN

; Clear: cancel a copy in progress.
HKCLR:  LODA,R0 V_CMODE
        BCTA,EQ HKEY
        EORZ    R0
        STRA,R0 V_CMODE
        ZBSR    *ZSNDBCK
        ZBRR    *ZHOSTPG

; 1: rename the slot through the editor.
HKREN:  LODA,R0 V_CUR           ; V_HNAME = the slot's 32 bytes + NUL
        RRL,R0
        RRL,R0
        RRL,R0
        RRL,R0
        RRL,R0
        STRZ    R1
        LODI,R3 0
HKR1:   LODA,R0 *FRDP,R1
        STRA,R0 V_HNAME,R3
        ADDI,R1 1
        ADDI,R3 1
        COMI,R3 32
        BCTR,LT HKR1
        EORZ    R0
        STRA,R0 V_HNAME+32
        LODI,R0 V_HNAME>>8
        STRA,R0 V_EBUF
        LODI,R0 V_HNAME&$FF
        STRA,R0 V_EBUF+1
        LODI,R0 31
        STRA,R0 V_EMAX
        LODI,R0 TEHOST>>8
        LODI,R1 TEHOST&$FF
        BSTA,UN EDIT
        BCTA,EQ HKR2
        ZBRR    *ZHOSTPG          ; ESC: the slot is untouched
HKR2:   LODA,R0 V_HOST
        BSTA,UN HWRITE
        BCTA,EQ HOSTPG
        STRZ    R2
        LODI,R0 EHWRT>>8
        LODI,R1 EHWRT&$FF
        ZBSR    *ZUFAIL
        ZBRR    *ZHOSTPG

; HSLOT: DSTRP = slot R0's name in the window; CC EQ when it is empty.
; Leaf. Clobbers R0.
HSLOT:  RRL,R0
        RRL,R0
        RRL,R0
        RRL,R0
        RRL,R0
        STRA,R0 DSTRP+1
        LODI,R0 FNRDATA>>8
        STRA,R0 DSTRP
        LODA,R0 *DSTRP
        RETC,UN

; HWRITE: replace slot R0 with V_HNAME and write all 8 slots back: a
; fresh READ_HOST_SLOTS, then WRITE_HOST_SLOTS streamed from the window
; with that slot substituted (NUL-padded to 32). Results as CGO.
; Calls four down. Clobbers R0-R3, V_T2, V_T3.
HWRITE: STRA,R0 V_T2
        LODI,R3 FCRHSL
        ZBSR    *ZCDO0
        BCFA,EQ HWR9
        LODI,R3 FCWHSL
        LODI,R2 0
        ZBSR    *ZMSTART
        EORZ    R0
HWR1:   STRA,R0 V_T3            ; the slot being streamed
        COMA,R0 V_T2
        BCTR,EQ HWRED
        RRL,R0
        RRL,R0
        RRL,R0
        RRL,R0
        RRL,R0
        STRZ    R1
        LODI,R3 32
HWRC:   LODA,R0 *FRDP,R1
        STRZ    R2
        LODA,R0 *FTXP,R2
        ADDI,R1 1
        BDRR,R3 HWRC
        BCTR,UN HWRN
HWRED:  SETFS   V_HNAME
        LODI,R2 32
        BSTA,UN CTXFIX
HWRN:   LODA,R0 V_T3
        ADDI,R0 1
        COMI,R0 8
        BCTR,LT HWR1
        ZBRR    *ZCGO
HWR9:   RETC,UN

; 9: the Game Lobby -- find (or claim slot 8 for) the lobby host, mount
; it, boot the lobby image.
LOBBY:  LODI,R0 TLOBBY>>8
        LODI,R1 TLOBBY&$FF
        ZBSR    *ZUTITLE
        LODI,R3 FCRHSL
        ZBSR    *ZCDO0
        BCTA,EQ LB0
LBERR:  STRZ    R2
        LODI,R0 EMHOST>>8
        LODI,R1 EMHOST&$FF
        ZBSR    *ZUFAIL
        ZBRR    *ZHOSTPG
LB0:    EORZ    R0
LB1:    STRA,R0 V_T2            ; slot
        RRL,R0
        RRL,R0
        RRL,R0
        RRL,R0
        RRL,R0
        STRZ    R1
        BSTR,UN PCMPI
        BCTR,EQ LBFND
        LODA,R0 V_T2
        ADDI,R0 1
        COMI,R0 8
        BCTR,LT LB1
        LODI,R3 0               ; absent: write it into slot 8
LB2:    LODA,R0 *PLOBH,R3
        STRA,R0 V_HNAME,R3
        ADDI,R3 1
        COMI,R0 0
        BCFR,EQ LB2
        LODI,R0 7
        STRA,R0 V_T2
        BSTA,UN HWRITE
        BCFA,EQ LBERR
LBFND:  LODA,R0 V_T2
        STRA,R0 V_HOST
        BSTA,UN CMOUNT
        BCFA,EQ LBERR
        BCTA,UN BOOTLB

; PCMPI: R0 = 0 / CC EQ when the window string at offset R1 equals the
; lobby host name, ignoring case. Calls one down. Clobbers R0,R1,R3, V_T3.
PCMPI:  LODI,R3 0
PCI1:   LODA,R0 *PLOBH,R3
        BSTA,UN PFOLD
        STRA,R0 V_T3
        LODA,R0 *FRDP,R1
        BSTA,UN PFOLD
        COMA,R0 V_T3
        BCFR,EQ PCINE
        COMI,R0 0
        BCTR,EQ PCI9            ; both ended together
        ADDI,R1 1
        ADDI,R3 1
        BCTR,UN PCI1
PCINE:  LODI,R0 1
PCI9:   RETC,UN

PLOBH:  DWBE    TLOBH           ; build/lobby.inc, block 2
