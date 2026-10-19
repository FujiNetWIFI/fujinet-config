; wifi.inc -- the power-on check, connect, scan / select / password /
; save (block 2).
;
; Mirrors astrocade/wifi.inc: at power-on go straight to the host list
; (WiFi disabled or already connected), wait out a connect on the
; stored SSID, or run the scan chain. Hosts key 7 comes back here to
; pick a different network. Page-1 code: page-0 RAM through the pointer
; words at the bottom of this file, the screen through block-1 helpers.

; ---- power-on ----------------------------------------------------------
CKWIFI: LODI,R0 TCFG>>8
        LODI,R1 TCFG&$FF
        ZBSR    *ZUTITLE
        PRINT   TWAIT
        LODI,R3 4               ; ~15 s for the ESP32 link; a key skips
CKW0:   LODI,R2 225
CKW1:   LODA,R0 FNSTATS
        ANDI,R0 1
        BCFR,EQ CKW2
        ZBSR    *ZKEYSCN          ; one frame (R2, R3 survive it)
        COMI,R0 KNONE
        BCFR,EQ CKW2
        BDRR,R2 CKW1
        BDRR,R3 CKW0
CKW2:
    IF WIFIFAKE
        ZBRR    *ZWSCAN           ; test builds: always set WiFi up
    ENDIF
        LODI,R3 FCWEN
        ZBSR    *ZCDO0
        BCFA,EQ HOSTPG          ; can't even ask: the host page copes
        LODA,R0 FNRDATA
        BCTA,EQ HOSTPG          ; WiFi disabled
        LODI,R3 FCWSTAT
        ZBSR    *ZCDO0
        BCFA,EQ HOSTPG
        LODA,R0 FNRDATA
        COMI,R0 3
        BCTA,EQ HOSTPG          ; already connected
        LODI,R3 FCGSSID
        ZBSR    *ZCDO0
        BCFA,EQ HOSTPG
        LODA,R0 FNRDATA
        BCTA,EQ WSCAN           ; nothing stored: set one up

; ---- connecting --------------------------------------------------------
WCONN:  LODI,R0 TCONN>>8
        LODI,R1 TCONN&$FF
        ZBSR    *ZUTITLE
        LODI,R3 FCGSSID         ; the SSID being tried
        ZBSR    *ZCDO0
        DCOL1   AACC
        SETST1  FNRDATA
        DAT     2,0
        LODI,R1 16
        ZBSR    *ZDPRTN
        PRINT   LANYC
        EORZ    R0
        STRA,R0 *PVCTRY
WCN1:   LODI,R3 FCWSTAT         ; ~20 polls, 2 s apart
        ZBSR    *ZCDO0
        BCFR,EQ WCN2
        LODA,R0 FNRDATA
        COMI,R0 3
        BCTA,EQ WCOK
WCN2:   LODA,R0 *PVCTRY         ; the bar: 5% a poll
        STRZ    R1
        RRL,R0
        RRL,R0
        ADDZ    R1
        ZBSR    *ZBARPCT
        LODI,R3 120
        BSTA,UN UPAUSE
        COMI,R0 KNONE
        BCFA,EQ WSCAN           ; a key: pick a network instead
        LODA,R0 *PVCTRY
        ADDI,R0 1
        STRA,R0 *PVCTRY
        COMI,R0 20
        BCTR,LT WCN1
        LODI,R0 TCFAIL>>8
        LODI,R1 TCFAIL&$FF
        ZBSR    *ZUMSG
        ZBRR    *ZWSCAN
WCOK:   LODI,R0 100
        ZBSR    *ZBARPCT
        BSTA,UN SNDOK
        ZBRR    *ZHOSTPG

; ---- scan and select ---------------------------------------------------
WSCAN:  EORZ    R0              ; V_SSID/V_PASS overlay V_SRC: no copy
        STRA,R0 *PVCMOD         ;   survives WiFi setup
        LODI,R0 TSELN>>8
        LODI,R1 TSELN&$FF
        ZBSR    *ZUTITLE
        PRINT   TSCAN
        LODI,R3 FCSCAN
        LODI,R2 0
        ZBSR    *ZMSTART
        LODI,R1 20              ; ~21 s, with the busy sweep
        BSTA,UN LCOMMIT
        BCTR,EQ WSC1
        EORZ    R0
        BCTR,UN WSC2
WSC1:   LODA,R0 FNRDATA
        COMI,R0 7               ; 7 networks + OTHER fill the list
        BCTR,LT WSC2
        LODI,R0 7
WSC2:   STRA,R0 *PVNNET
        LODI,R0 TSELN>>8        ; a clean page for the list
        LODI,R1 TSELN&$FF
        ZBSR    *ZUTITLE
        EORZ    R0
WSD1:   STRA,R0 *PVCUR          ; the row being drawn
        COMA,R0 *PVNNET
        BCTR,EQ WSDOTH
        STRZ    R1
        LODI,R3 FCSCRES
        BSTA,UN CDO1
        BCFR,EQ WSDN            ; a failed row stays blank
        ZBSR    *ZROWCEL
        DCOL1   ANORM
        SETST1  FNRDATA
        LODI,R1 14
        ZBSR    *ZDPRTN
        LODA,R1 *PVCUR
        ADDI,R1 1
        LODA,R0 FNRDATA+33      ; RSSI
        BSTA,UN SIGNAL
WSDN:   LODA,R0 *PVCUR
        ADDI,R0 1
        BCTR,UN WSD1
WSDOTH: ZBSR    *ZROWCEL
        DCOL1   AACC
        SETST1  TOTHER
        LODI,R1 14
        ZBSR    *ZDPRTN
        PRINT   LSCAN
        LODA,R0 *PVNNET
        ADDI,R0 1
        STRA,R0 *PVCNT
        LODI,R0 14
        STRA,R0 *PVHLW
        EORZ    R0
        STRA,R0 *PVCUR
        BSTA,UN LHI
WSLOOP: ZBSR    *ZLKEY
        COMI,R0 KFIRE
        BCTR,EQ WSSEL
        COMI,R0 KENT
        BCTR,EQ WSSEL
        COMI,R0 KCLR
        BCTA,EQ WSCAN
        COMI,R0 0
        BCFR,EQ WSLOOP
        ZBSR    *ZSNDBCK          ; 0: skip WiFi setup
        ZBRR    *ZHOSTPG

WSSEL:  ZBSR    *ZSNDSEL
        LODA,R0 *PVCUR
        COMA,R0 *PVNNET
        BCTR,EQ WSOTH
        STRZ    R1              ; fetch the network's name again; it
        LODI,R3 FCSCRES         ;   stays in the window through the
        BSTA,UN CDO1            ;   (transaction-free) editor
        BCFA,EQ WSCAN
        SETFS1  FNRDATA
        BCTR,UN WPASS
WSOTH:  EORZ    R0
        STRA,R0 *PVSSID
        LODI,R2 V_SSID&$FF
        LODI,R3 32
        LODI,R0 TESSID>>8
        LODI,R1 TESSID&$FF
        BSTA,UN EDITB
        BCFA,EQ WSCAN
        SETFS1  V_SSID

; the password, then SET_SSID: nparam >= 1 (value ignored) and exactly
; ssid[33] + password[64] = 97 payload bytes.
WPASS:  EORZ    R0
        STRA,R0 *PVPASS
        LODI,R2 V_PASS&$FF
        LODI,R3 63
        LODI,R0 TEPASS>>8
        LODI,R1 TEPASS&$FF
        BSTA,UN EDITB
        BCFA,EQ WSCAN
        LODI,R0 TSAVE>>8
        LODI,R1 TSAVE&$FF
        ZBSR    *ZUTITLE
        LODI,R3 FCSSID
        LODI,R2 1
        ZBSR    *ZMSTART
        LODI,R1 0
        ZBSR    *ZMPARB
        LODI,R2 33              ; FSTRP: the window or V_SSID
        BSTA,UN CTXFIX
        SETFS1  V_PASS
        LODI,R2 64
        BSTA,UN CTXFIX
        ZBSR    *ZCGO
        BCTA,EQ WCONN
        STRZ    R2
        LODI,R0 ESAVE>>8
        LODI,R1 ESAVE&$FF
        ZBSR    *ZUFAIL
        ZBRR    *ZWSCAN

; ---- info ------------------------------------------------------------------
; Hosts key 3: one GET_ADAPTERCONFIG_EXTENDED (240 bytes, slice 0), five
; label/value pairs drawn straight from the window.
INFOPG: ZBSR    *ZSNDSEL
        LODI,R0 TINFO>>8
        LODI,R1 TINFO&$FF
        ZBSR    *ZUTITLE
        LODI,R3 FCACFGX
        ZBSR    *ZCDO0
        BCTR,EQ IN1
        STRZ    R2
        LODI,R0 EINFO>>8
        LODI,R1 EINFO&$FF
        ZBSR    *ZUFAIL
        ZBRR    *ZHOSTPG
IN1:    EORZ    R0
IN2:    STRA,R0 *PVT5           ; table offset, 4 bytes an item
        STRZ    R3
        LODA,R0 INTAB+1,R3      ; the label
        STRZ    R1
        LODA,R0 INTAB,R3
        ZBSR    *ZPRS
        LODA,R3 *PVT5           ; the value, from the window
        LODA,R0 INTAB+2,R3
        STRA,R0 *PDSTRL
        LODI,R0 FNRDATA>>8
        STRA,R0 *PDSTRH
        LODA,R0 INTAB+3,R3
        STRZ    R2
        DCOL1   ANORM
        LODI,R1 16
        ZBSR    *ZDPRTN
        LODA,R0 *PVT5
        ADDI,R0 4
        COMI,R0 20
        BCTR,LT IN2
        PRINT   LANYK
        BSTA,UN GETKEY
        ZBSR    *ZSNDBCK
        ZBRR    *ZHOSTPG

INTAB:  DWBE    TLSSID          ; label, window offset, value cell
        DB      0,2*16-1
        DWBE    TLHOST
        DB      33,4*16-1
        DWBE    TLIP
        DB      140,6*16-1
        DWBE    TLGW
        DB      156,8*16-1
        DWBE    TLVER
        DB      125,10*16-1
