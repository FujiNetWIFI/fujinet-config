; strings.inc -- every literal the screens print (block 2). DPRINT and
; PRS reach a string through DSTRP with an INDIRECT load, which carries
; the full 15 bits, so page-0 callers can name page-1 strings.
;
; Positioned strings (STRAT) start with their screen cell; a $01 byte
; toggles the accent ink. A legend is one 32-character string: row 11
; runs straight on into row 12.

; ---- titles (row 0) --------------------------------------------------------
TCFG:   STRAT   0,1
        DB      1,"FUJINET CONFIG",0
THOSTS: STRAT   0,1
        DB      1,"FUJINET HOSTS",0
TCPYH:  STRAT   0,2
        DB      1,"COPY TO HOST",0
TLOBBY: STRAT   0,3
        DB      1,"GAME LOBBY",0
TBOOT:  STRAT   0,4
        DB      1,"BOOTING",0
TINFO:  STRAT   0,2
        DB      1,"FUJINET INFO",0
TCONN:  STRAT   0,3
        DB      1,"CONNECTING",0
TSELN:  STRAT   0,1
        DB      1,"SELECT NETWORK",0
TSAVE:  STRAT   0,5
        DB      1,"SAVING",0
TEHOST: STRAT   0,0
        DB      1,"HOST NAME",0
TEFILT: STRAT   0,0
        DB      1,"FILTER",0
TESSID: STRAT   0,0
        DB      1,"NETWORK NAME",0
TEPASS: STRAT   0,0
        DB      1,"PASSWORD",0

; ---- fields (DPRTN, not positioned) ----------------------------------------
TEMPTY: DB      "--------",0
TOTHER: DB      "OTHER NETWORK",0

; ---- lines -----------------------------------------------------------------
TWAIT:  STRAT   5,0
        DB      "WAITING FOR LINK",0
TSCAN:  STRAT   10,0
        DB      "SCANNING",0
TCPING: STRAT   10,0
        DB      "COPYING",0
TEMPTD: STRAT   4,2
        DB      "EMPTY FOLDER",0
TFLT:   STRAT   9,0
        DB      1,"FLT:",1,0
TNOPOW: STRAT   12,0
        DB      "DO NOT POWER OFF",0
LANYK:  STRAT   12,0
        DB      1,"ANY KEY",1," BACK",0
LANYC:  STRAT   12,0
        DB      1,"ANY KEY",1," CANCEL",0
TNOCRT: STRAT   4,0
        DB      "NO FUJINET CART",0

; ---- legends (rows 11-12) ---------------------------------------------------
LHOST:  STRAT   11,0
        DB      1,"1",1," RENAME ",1,"3",1," INFO "
        DB      1,"7",1," WIFI  ",1,"9",1," LOBBY",0
LHCOPY: STRAT   11,0
        DB      1,"FIRE",1," PICK DEST  "
        DB      1,"CLR",1," CANCEL COPY",0
LBRWS:  STRAT   11,0
        DB      1,"1",1," UP  ",1,"4/6",1," PAGE  "
        DB      1,"5",1," COPY  ",1,"7",1," FILTER",0
LBCOPY: STRAT   11,0
        DB      1,"5",1," COPY HERE     "
        DB      1,"CLR",1," CANCEL COPY",0
LSCAN:  STRAT   11,0
        DB      1,"FIRE",1," PICK ",1,"0",1," SKIP"
        DB      1,"CLR",1," RESCAN",0
LEDIT:  STRAT   11,0
        DB      1,"FIRE",1," ADD ",1,"CLR",1," DEL"
        DB      1,"ENTER",1," OK  ",1,"0",1," ESC",0

; ---- info labels -------------------------------------------------------------
TLSSID: STRAT   1,0
        DB      1,"SSID",0
TLHOST: STRAT   3,0
        DB      1,"HOSTNAME",0
TLIP:   STRAT   5,0
        DB      1,"IP ADDRESS",0
TLGW:   STRAT   7,0
        DB      1,"GATEWAY",0
TLVER:  STRAT   9,0
        DB      1,"VERSION",0

; ---- messages (row 10; UFAIL adds a hex code in cols 14-15) -----------------
EHOSTS: STRAT   10,0
        DB      "HOSTS ERROR",0
EMHOST: STRAT   10,0
        DB      "HOST ERROR",0
EHWRT:  STRAT   10,0
        DB      "SAVE ERROR",0
EODIR:  STRAT   10,0
        DB      "FOLDER ERROR",0
TPLONG: STRAT   10,0
        DB      "PATH TOO LONG",0
TNODIR: STRAT   10,0
        DB      "FILES ONLY",0
TCPOK:  STRAT   10,0
        DB      "COPIED",0
ECOPY:  STRAT   10,0
        DB      "COPY FAILED",0
EPATH:  STRAT   10,0
        DB      "PATH ERROR",0
EMOUNT: STRAT   10,0
        DB      "MOUNT ERROR",0
ESTAGE: STRAT   10,0
        DB      "STAGE ERROR",0
EBTOOB: STRAT   10,0
        DB      "ROM TOO BIG",0
EBTRNC: STRAT   10,0
        DB      "TRUNCATED",0
ENCART: STRAT   10,0
        DB      "NOT A CART",0
EINFO:  STRAT   10,0
        DB      "INFO ERROR",0
TCFAIL: STRAT   10,0
        DB      "CONNECT FAILED",0
ESAVE:  STRAT   10,0
        DB      "SAVE FAILED",0

TCFGL:  DB      "cfg",0
TCFGU:  DB      "CFG",0

        INCLUDE "build/lobby.inc"

    IF BARTEST
SBTA:   STRAT   0,0
        DB      1,"CONFIG BARTEST",0
SBTROW: DB      "ENTRY (USA).BIN",0
SBTB:   STRAT   1,0
        DB      "WEAK NETWORK",0
    ENDIF
