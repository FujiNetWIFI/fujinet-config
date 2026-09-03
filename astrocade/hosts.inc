; hosts.inc -- the host slot page: list, mount, rename, lobby.
;
; The list is drawn straight out of the READ_HOST_SLOTS reply window (8
; records x 32 bytes = 256 = exactly slice 0); nothing is buffered. That
; same stay-painted property powers the rename and the lobby's slot claim:
; WRITE_HOST_SLOTS is streamed byte-for-byte FROM the reply window (with
; the edited slot substituted from V_ENTRY), so no 256-byte RAM mirror
; ever exists. The one rule: no transaction may run between the freshness
; READ and the streamed WRITE -- the editor is transaction-free, and
; HWRITE re-reads immediately before writing.
;
; In copy mode (V_CMODE=1) the page re-skins itself as the destination
; host picker, exactly like the INTV port.

HOSTPG: LD      A,(V_CMODE)
        OR      A
        LD      HL,THOSTS
        JR      Z,HP1
        LD      HL,TCPYH
HP1:    CALL    UTITLE
        LD      E,FCRHSL
        CALL    CDO0
        JR      NC,HP2
        LD      HL,EHOSTS
        CALL    UFAIL
        JR      HOSTPG          ; keep trying; the ESP32 may still be booting
HP2:    LD      B,0
HPD1:   PUSH    BC
        LD      A,B             ; slot digit, accent colour
        ADD     A,'1'
        LD      (HEXBUF),A
        XOR     A
        LD      (HEXBUF+1),A
        CALL    UROWY
        LD      E,LISTX
        LD      C,OPTHI
        LD      HL,HEXBUF
        SYSTEM  STRDIS
        POP     BC
        PUSH    BC
        LD      A,B             ; name from the reply window, slot*32
        RRCA
        RRCA
        RRCA
        LD      L,A
        LD      H,0
        LD      DE,FNRDATA
        ADD     HL,DE
        LD      A,(HL)
        OR      A
        JR      NZ,HPD2
        LD      HL,TEMPTY
HPD2:   LD      B,NAMELEN-2
        CALL    CPYSAN
        POP     BC
        PUSH    BC
        CALL    UROWY
        LD      E,LISTX+12
        LD      C,OPTFB
        LD      HL,LINBUF
        SYSTEM  STRDIS
        POP     BC
        INC     B
        LD      A,B
        CP      8
        JR      C,HPD1
        LD      A,(V_CMODE)
        OR      A
        LD      HL,THFOOT
        JR      Z,HP3
        LD      HL,THCFOOT
HP3:    CALL    UFOOT
        LD      A,(V_HOST)
        AND     7               ; RESET leaves RAM arbitrary
        LD      (V_HOST),A
        LD      B,A
        CALL    UCRON
HKEY:   CALL    INWAIT
        CP      EVUP
        JR      Z,HKU
        CP      EVDOWN
        JR      Z,HKD
        CP      EVTRIG
        JP      Z,HKT
        CP      EVEQ
        JP      Z,HKE
        CP      EVCE
        JP      Z,HKC
        CP      EVD0
        JR      C,HKEY
        CP      EVD0+10
        JR      NC,HKEY
        SUB     EVD0
        JP      HKDIG
HKU:    LD      A,(V_HOST)
        OR      A
        JR      Z,HKEY
        LD      B,A
        CALL    UCROFF
        LD      A,(V_HOST)
        DEC     A
HKSET:  LD      (V_HOST),A
        LD      B,A
        CALL    UCRON
        JR      HKEY
HKD:    LD      A,(V_HOST)
        CP      7
        JR      NC,HKEY
        LD      B,A
        CALL    UCROFF
        LD      A,(V_HOST)
        INC     A
        JR      HKSET

; keypad digits: 1-8 jump to a slot; 9 = info, 0 = lobby (normal mode).
HKDIG:  LD      C,A
        LD      A,(V_CMODE)
        OR      A
        JR      NZ,HKDIG1
        LD      A,C
        CP      9
        JP      Z,INFOPG
        OR      A
        JP      Z,LOBBY
HKDIG1: LD      A,C
        OR      A
        JR      Z,HKEY
        CP      9
        JR      NC,HKEY
        DEC     A               ; A = target slot (0-based)
        PUSH    AF              ; UCROFF -> DRWCH clobbers BC, so keep the
        LD      A,(V_HOST)      ; target on the stack across the erase
        LD      B,A
        CALL    UCROFF
        POP     AF
        JR      HKSET

; trigger: mount the slot and browse its root.
HKT:    LD      A,(V_HOST)
        CALL    CMOUNT
        JR      NC,HKT1
        LD      HL,EMHOST
        CALL    UFAIL
        JP      HOSTPG
HKT1:   CALL    SRESET
        JP      SFDRAW

; = : rename the slot through the grid editor (normal mode only).
HKE:    LD      A,(V_CMODE)
        OR      A
        JP      NZ,HKEY
        LD      A,(V_HOST)
        RRCA
        RRCA
        RRCA
        LD      L,A
        LD      H,0
        LD      DE,FNRDATA
        ADD     HL,DE
        LD      DE,V_ENTRY
        LD      BC,32
        LDIR
        XOR     A
        LD      (V_ENTRY+32),A
        LD      HL,TEHOST
        LD      A,31
        CALL    EDIT
        JP      C,HOSTPG        ; cancel leaves the slot untouched
        LD      A,(V_HOST)
        CALL    HWRITE
        JP      NC,HOSTPG
        LD      HL,EHWRT
        CALL    UFAIL
        JP      HOSTPG

; CE: cancel an in-flight copy.
HKC:    LD      A,(V_CMODE)
        OR      A
        JP      Z,HKEY
        XOR     A
        LD      (V_CMODE),A
        JP      HOSTPG

; HWRITE: replace slot A with V_ENTRY and write all 8 slots back.
; Fresh READ_HOST_SLOTS immediately first, then WRITE_HOST_SLOTS streamed
; from the reply window with slot A substituted (NUL-padded to 32).
; Carry set on failure.
HWRITE: LD      (V_TMP+2),A
        LD      E,FCRHSL
        CALL    CDO0
        RET     C
        LD      A,FNDFUJI
        LD      E,FCWHSL
        LD      L,0
        CALL    FNBEGIN
        LD      C,0             ; slot being streamed
HWL:    LD      A,(V_TMP+2)
        CP      C
        JR      Z,HWED
        LD      A,C
        RRCA
        RRCA
        RRCA
        LD      L,A
        LD      H,0
        LD      DE,FNRDATA
        ADD     HL,DE
        LD      B,32
HWCP:   LD      A,(HL)
        CALL    FNTXBYT
        INC     HL
        DJNZ    HWCP
        JR      HWN
HWED:   LD      HL,V_ENTRY
        LD      B,32
        CALL    CTXFIX
HWN:    INC     C
        LD      A,C
        CP      8
        JR      C,HWL
        JP      CGO

; keypad 0: find (or claim slot 8 for) the lobby host, mount it, boot the
; lobby ROM.
LOBBY:  LD      HL,TLOBBY
        CALL    UTITLE
        LD      E,FCRHSL        ; fresh read; the title repaint cost nothing
        CALL    CDO0
        JR      NC,LB0
        LD      HL,EHOSTS
        CALL    UFAIL
        JP      HOSTPG
LB0:    LD      C,0
LB1:    PUSH    BC
        LD      A,C
        RRCA
        RRCA
        RRCA
        LD      L,A
        LD      H,0
        LD      DE,FNRDATA
        ADD     HL,DE
        LD      DE,TLOBH
        CALL    PCMPI
        POP     BC
        JR      Z,LBFND
        INC     C
        LD      A,C
        CP      8
        JR      C,LB1
        LD      HL,TLOBH        ; not present: claim slot 8
        LD      DE,V_ENTRY
        CALL    USTRCP
        LD      A,7
        CALL    HWRITE
        JR      NC,LB2
        LD      HL,EHWRT
        CALL    UFAIL
        JP      HOSTPG
LB2:    LD      C,7
LBFND:  LD      A,C
        PUSH    BC
        CALL    CMOUNT
        POP     BC
        JR      NC,LB3
        LD      HL,EMHOST
        CALL    UFAIL
        JP      HOSTPG
LB3:    LD      A,C
        LD      (V_HOST),A
        LD      HL,TLOBP
        LD      DE,V_SRC
        CALL    USTRCP
        LD      A,C
        JP      BOOTGO

; USTRCP: copy the NUL-terminated string at HL to DE, NUL included.
; DE ends just past the NUL. Clobbers A,HL,DE.
USTRCP: LD      A,(HL)
        LD      (DE),A
        INC     HL
        INC     DE
        OR      A
        JR      NZ,USTRCP
        RET

; PCMPI: case-insensitive compare of the NUL-terminated strings at HL
; and DE. Z set when equal. Clobbers A,B,HL,DE.
PCMPI:  LD      A,(DE)
        CALL    PFOLD
        LD      B,A
        LD      A,(HL)
        CALL    PFOLD
        CP      B
        RET     NZ
        OR      A
        RET     Z
        INC     HL
        INC     DE
        JR      PCMPI

; PFOLD: fold a-z to A-Z.
PFOLD:  CP      'a'
        RET     C
        CP      'z'+1
        RET     NC
        SUB     20H
        RET

THOSTS: DB      "FUJINET HOSTS",0
TCPYH:  DB      "COPY TO HOST",0
TEMPTY: DB      "--------",0
THFOOT: DB      "T:OPEN =:EDIT 9:INF 0:LOB",0
THCFOOT: DB     "T:PICK DEST  CE:CANCEL",0
TEHOST: DB      "HOST NAME",0
TLOBBY: DB      "GAME LOBBY",0
TLOBH:  DB      "ec.tnfs.io",0
TLOBP:  DB      "/astrocade/lobby.bin",0
EHOSTS: DB      "HOSTS ERR",0
EMHOST: DB      "HOST ERR",0
EHWRT:  DB      "SAVE ERR",0
