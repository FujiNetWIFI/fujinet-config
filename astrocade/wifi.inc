; wifi.inc -- ST_CHECK_WIFI / ST_CONNECT_WIFI / ST_SET_WIFI.
;
; Mirrors the INTV flow: at power-on decide between going straight to the
; host list (wifi disabled or already connected), waiting out a connect on
; the stored SSID, or running the scan / select / password / save chain.
; The WiFi screens are unreachable once ST_HOSTS has been entered, which
; is what makes the V_SSID/V_PASS union with V_SRC safe.

; ---- ST_CHECK_WIFI ----------------------------------------------------
CKWIFI: LD      HL,TTITLE
        CALL    UTITLE
        LD      HL,TWAIT
        CALL    UFOOT
        LD      DE,1500         ; ~15 s for the ESP32 link; input skips
CKW1:   LD      A,(FNSTATS)
        RRA
        JR      C,CKW2
        PUSH    DE
        CALL    INPOLL
        POP     DE
        OR      A
        JR      NZ,CKW2
        DEC     DE
        LD      A,D
        OR      E
        JR      NZ,CKW1
CKW2:   LD      E,FCWEN
        CALL    CDO0
        JR      C,CKWH          ; can't even ask: let the host page cope
        LD      A,(FNRDATA)
        OR      A
        JR      Z,CKWH          ; wifi disabled
        LD      E,FCWSTAT
        CALL    CDO0
        JR      C,CKWH
        LD      A,(FNRDATA)
        CP      3
        JR      Z,CKWH          ; already connected
        LD      E,FCGSSID
        CALL    CDO0
        JR      C,CKWH
        LD      A,(FNRDATA)
        OR      A
        JP      Z,WSCAN         ; nothing stored: set one up
        LD      HL,FNRDATA      ; stored SSID: wait for the connect
        LD      DE,V_SSID
        LD      BC,33
        LDIR
        JR      WCONN
CKWH:   JP      HOSTPG

; ---- ST_CONNECT_WIFI --------------------------------------------------
WCONN:  LD      HL,TCONN
        CALL    UTITLE
        LD      HL,V_SSID
        LD      DE,LISTY0*256+0
        LD      C,GCNORM
        LD      B,20
        CALL    DRWSTR
        LD      HL,TABORT
        CALL    UFOOT
        LD      B,20            ; ~20 x 2 s
WCN1:   PUSH    BC
        LD      E,FCWSTAT
        CALL    CDO0
        POP     BC
        JR      C,WCN2
        LD      A,(FNRDATA)
        CP      3
        JP      Z,HOSTPG
WCN2:   PUSH    BC
        LD      DE,200
        CALL    UPAUSE
        POP     BC
        OR      A
        JR      NZ,WSCAN        ; user abort: pick a network instead
        DJNZ    WCN1
        LD      HL,TCFAIL
        CALL    UMSG
        JR      WSCAN

; ---- ST_SET_WIFI ------------------------------------------------------
WSCAN:  LD      HL,TSELN
        CALL    UTITLE
        LD      HL,TSCAN
        CALL    UFOOT
        LD      E,FCSCAN
        CALL    CDO0
        JR      C,WSC0
        LD      A,(FNRDATA)
        CP      NROWS-1         ; 7 networks + the OTHER row fill the list
        JR      C,WSC1
        LD      A,NROWS-1
        JR      WSC1
WSC0:   XOR     A
WSC1:   LD      (V_NNET),A
        LD      B,0
WSD1:   LD      A,(V_NNET)
        CP      B
        JR      Z,WSDOTH
        PUSH    BC
        LD      A,B
        LD      E,FCSCRES
        CALL    CDO1
        POP     BC
        JR      C,WSDN          ; failed row stays blank
        PUSH    BC
        LD      HL,FNRDATA
        LD      B,20
        CALL    CPYSAN
        POP     BC
        PUSH    BC
        LD      C,OPTFB
        CALL    USHOWR
        POP     BC
        PUSH    BC
        LD      A,(FNRDATA+33)  ; RSSI, negative dBm
        LD      HL,TBAR1
        CP      0BBH            ; worse than -70
        JR      C,WSB1
        LD      HL,TBAR2
        CP      0CAH            ; worse than -55
        JR      C,WSB1
        LD      HL,TBAR3
WSB1:   POP     BC
        PUSH    BC
        CALL    UROWY
        LD      E,138
        LD      C,OPTDIM
        SYSTEM  STRDIS
        POP     BC
WSDN:   INC     B
        JR      WSD1
WSDOTH: CALL    UROWY
        LD      E,LISTX
        LD      C,OPTHI
        LD      HL,TOTHER
        SYSTEM  STRDIS
        LD      HL,TWSFOOT
        CALL    UFOOT
        XOR     A
        LD      (V_CUR),A
        LD      B,0
        CALL    UCRON
WSLOOP: CALL    INWAIT
        CP      EVUP
        JR      Z,WSMU
        CP      EVDOWN
        JR      Z,WSMD
        CP      EVTRIG
        JR      Z,WSSEL
        CP      EVCE
        JP      Z,WSCAN
        JR      WSLOOP

WSMU:   LD      A,(V_CUR)
        OR      A
        JR      Z,WSLOOP
        LD      B,A
        CALL    UCROFF
        LD      A,(V_CUR)
        DEC     A
WSSET:  LD      (V_CUR),A
        LD      B,A
        CALL    UCRON
        JR      WSLOOP
WSMD:   LD      A,(V_NNET)
        LD      C,A
        LD      A,(V_CUR)
        CP      C
        JR      NC,WSLOOP       ; already on OTHER
        LD      B,A
        CALL    UCROFF
        LD      A,(V_CUR)
        INC     A
        JR      WSSET

WSSEL:  LD      A,(V_NNET)
        LD      C,A
        LD      A,(V_CUR)
        CP      C
        JR      Z,WSOTH
        LD      E,FCSCRES       ; re-fetch the chosen network's name
        CALL    CDO1
        JP      C,WSCAN
        LD      HL,FNRDATA
        LD      DE,V_SSID
        LD      BC,33
        LDIR
        JR      WPASS
WSOTH:  XOR     A
        LD      (V_ENTRY),A
        LD      HL,TESSID
        LD      A,32
        CALL    EDIT
        JP      C,WSCAN
        LD      HL,V_ENTRY
        LD      DE,V_SSID
        LD      BC,33
        LDIR

WPASS:  XOR     A
        LD      (V_ENTRY),A
        LD      HL,TEPASS
        LD      A,63
        CALL    EDIT
        JP      C,WSCAN
        LD      HL,V_ENTRY
        LD      DE,V_PASS
        LD      BC,64
        LDIR

; WS_DONE: SET_SSID needs nparam >= 1 (value ignored) and exactly
; ssid[33] + password[64] = 97 payload bytes.
        LD      HL,TSAVING
        CALL    UTITLE
        LD      A,FNDFUJI
        LD      E,FCSSID
        LD      L,1
        CALL    FNBEGIN
        XOR     A
        CALL    FNPARB
        LD      HL,V_SSID
        LD      B,33
        CALL    CTXFIX
        LD      HL,V_PASS
        LD      B,64
        CALL    CTXFIX
        CALL    CGO
        JP      NC,WCONN
        LD      HL,TSFAIL
        CALL    UMSG
        JP      WSCAN

TTITLE: DB      "FUJINET CONFIG",0
TWAIT:  DB      "WAITING FOR NETWORK LINK",0
TCONN:  DB      "CONNECTING",0
TABORT: DB      "ANY INPUT CANCELS",0
TCFAIL: DB      "CONNECT FAILED",0
TSELN:  DB      "SELECT NETWORK",0
TSCAN:  DB      "SCANNING...",0
TOTHER: DB      "OTHER (ENTER SSID)",0
TWSFOOT: DB     "T:PICK  CE:RESCAN",0
TESSID: DB      "ENTER SSID",0
TEPASS: DB      "ENTER PASSWORD",0
TSAVING: DB     "SAVING...",0
TSFAIL: DB      "SAVE FAILED",0
TBAR1:  DB      "*",0
TBAR2:  DB      "**",0
TBAR3:  DB      "***",0
