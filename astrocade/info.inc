; info.inc -- keypad 9: SSID / IP / firmware version.
;
; One GET_ADAPTERCONFIG_EXTENDED; the 240-byte reply fits slice 0 and is
; rendered straight from the window: ssid at +0 (drawn with the lowercase-
; capable blitter -- SSIDs are case-sensitive), fn_version at +125,
; sLocalIP at +140.

INFOPG: LD      HL,TINFO
        CALL    UTITLE
        LD      E,FCACFGX
        CALL    CDO0
        JR      NC,IN1
        LD      HL,EINFO
        CALL    UFAIL
        JP      HOSTPG
IN1:    LD      B,0
        LD      HL,TLSSID
        CALL    INLBL
        LD      HL,FNRDATA
        LD      DE,(LISTY0+8)*256+1
        LD      C,GCNORM
        LD      B,20
        CALL    DRWSTR
        LD      B,3
        LD      HL,TLIP
        CALL    INLBL
        LD      HL,FNRDATA+140
        LD      B,16
        CALL    CPYSAN
        LD      B,4
        CALL    INVAL
        LD      B,6
        LD      HL,TLVER
        CALL    INLBL
        LD      HL,FNRDATA+125
        LD      B,15
        CALL    CPYSAN
        LD      B,7
        CALL    INVAL
        LD      HL,TIFOOT
        CALL    UFOOT
INLOOP: CALL    INWAIT
        CP      EVTRIG
        JR      Z,INBACK
        CP      EVCE
        JR      NZ,INLOOP
INBACK: JP      HOSTPG

; INLBL: label at HL on list row B. INVAL: LINBUF on list row B.
INLBL:  PUSH    HL
        CALL    UROWY
        POP     HL
        LD      E,LISTX
        LD      C,OPTDIM
        SYSTEM  STRDIS
        RET
INVAL:  LD      C,OPTFB
        JP      USHOWR

TINFO:  DB      "FUJINET INFO",0
TLSSID: DB      "SSID",0
TLIP:   DB      "IP",0
TLVER:  DB      "VERSION",0
TIFOOT: DB      "T:BACK",0
EINFO:  DB      "INFO ERR",0
