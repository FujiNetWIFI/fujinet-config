; browse.inc -- ST_SELECT_FILE: the directory browser.
;
; Stateless by construction, like the INTV port: every page draw is
; MOUNT_HOST + OPEN_DIRECTORY(path+filter) + one SET_DIRECTORY_POSITION /
; READ_DIR_ENTRY pair per row + CLOSE_DIRECTORY, drawn straight from the
; reply window. Only per-row metadata survives: the absolute directory
; position (16-bit -- big directories exist) and the entry type. *.cfg
; entries are suppressed client-side; directories arrive with a trailing
; '/' from the firmware (there is no separate flag) and get the accent
; color; .bin carts stay white; everything else is dimmed.
;
; Display names are crunched to NAMELEN, so anything that must actually
; OPEN (subdir descent, boot, copy) re-reads the row at FULLLEN first.

; SRESET: back to the root of the mounted host, paging cleared.
SRESET: LD      HL,V_PATH
        LD      (HL),'/'
        INC     HL
        LD      (HL),0
SRST1:  LD      HL,0
        LD      (V_DPOS),HL
        XOR     A
        LD      (V_PDEP),A
        LD      (V_CUR),A
        RET

; SFRSET: paging cleared (path kept), then redraw.
SFRSET: CALL    SRST1
        JP      SFDRAW

; ---- page draw --------------------------------------------------------
SFDRAW: CALL    UCLRA
        LD      HL,V_PATH       ; title row = tail of the path
        LD      B,26
        CALL    UTAIL
        LD      HL,LINBUF
        LD      DE,TITLEY*256+2
        LD      C,OPTDIM
        SYSTEM  STRDIS
        LD      A,(V_HOST)
        CALL    CMOUNT
        JP      C,SFERR
        LD      A,(V_HOST)
        CALL    CODIR
        JP      C,SFERR
        XOR     A
        LD      (V_CNT),A
        LD      HL,(V_DPOS)
        LD      (V_DNEXT),HL    ; running directory position
SFD1:   LD      A,(V_CNT)
        CP      NROWS
        JR      NC,SFDE
        LD      HL,(V_DNEXT)
        LD      A,LISTLEN
        CALL    CDE16
        JR      C,SFDE          ; end of directory (or error)
        CALL    SISCFG
        JR      NZ,SFD2
        LD      HL,(V_DNEXT)    ; a .cfg: suppress and move on
        INC     HL
        LD      (V_DNEXT),HL
        JR      SFD1
SFD2:   LD      A,(V_CNT)       ; V_EPOS[row] = position
        ADD     A,A
        LD      E,A
        LD      D,0
        LD      HL,V_EPOS
        ADD     HL,DE
        LD      DE,(V_DNEXT)
        LD      (HL),E
        INC     HL
        LD      (HL),D
        CALL    STYPE           ; V_EDIR[row] = type
        LD      E,A
        LD      A,(V_CNT)
        LD      C,A
        LD      B,0
        LD      HL,V_EDIR
        ADD     HL,BC
        LD      (HL),E
        LD      A,E             ; row color by type
        LD      C,OPTHI
        CP      1
        JR      Z,SFD3
        LD      C,OPTFB
        CP      2
        JR      Z,SFD3
        LD      C,OPTDIM
SFD3:   PUSH    BC
        LD      HL,FNRDATA
        LD      B,NAMELEN
        CALL    CPYSAN
        POP     BC
        LD      A,(V_CNT)
        LD      B,A
        CALL    USHOWR
        LD      A,(V_CNT)
        INC     A
        LD      (V_CNT),A
        LD      HL,(V_DNEXT)
        INC     HL
        LD      (V_DNEXT),HL
        JP      SFD1
SFDE:   CALL    CCDIR           ; stateless: close what we opened
        LD      A,(V_CNT)
        OR      A
        JR      NZ,SFDF
        LD      HL,(V_DPOS)     ; empty page
        LD      A,H
        OR      L
        JR      NZ,SFPOPE       ; paged past the end: step back
        LD      A,(V_PATH+1)    ; empty at position 0:
        OR      A
        JR      NZ,SFEMPT       ; in a subdir, or
        LD      A,(V_FILT)
        OR      A
        JR      NZ,SFEMPT       ; filtered -- otherwise the root is
        JP      HOSTPG          ; genuinely empty: back to hosts
SFEMPT: LD      HL,TEMPTD
        CALL    UFOOT
        JR      SFLOOP
SFPOPE: CALL    SPOPPG
        JP      SFDRAW
SFDF:   LD      A,(V_CMODE)
        OR      A
        LD      HL,TSFOOT
        JR      Z,SFD4
        LD      HL,TSCFOOT
SFD4:   CALL    UFOOT
        LD      A,(V_CNT)
        LD      B,A
        LD      A,(V_CUR)
        CP      B
        JR      C,SFD5
        LD      A,B
        DEC     A
        LD      (V_CUR),A
SFD5:   LD      A,(V_CUR)
        LD      B,A
        CALL    UCRON

; ---- input loop -------------------------------------------------------
SFLOOP: CALL    INWAIT
        CP      EVUP
        JR      Z,SKU
        CP      EVDOWN
        JR      Z,SKD
        CP      EVLEFT
        JR      Z,SKPGB
        CP      EVRIGHT
        JR      Z,SKPGF
        CP      EVTRIG
        JP      Z,SKT
        CP      EVCE
        JP      Z,SKCE
        CP      EVD0+1
        JP      Z,SKUPD
        CP      EVD0+2
        JR      Z,SKPGB
        CP      EVD0+3
        JR      Z,SKPGF
        CP      EVD0+4
        JP      Z,SKFLT
        CP      EVD0+5
        JP      Z,SKCPY
        JR      SFLOOP

SKU:    LD      A,(V_CUR)
        OR      A
        JR      Z,SFLOOP
        LD      B,A
        CALL    UCROFF
        LD      A,(V_CUR)
        DEC     A
SKSET:  LD      (V_CUR),A
        LD      B,A
        CALL    UCRON
        JR      SFLOOP
SKD:    LD      A,(V_CNT)
        OR      A
        JR      Z,SFLOOP
        DEC     A
        LD      B,A
        LD      A,(V_CUR)
        CP      B
        JR      NC,SFLOOP
        LD      B,A
        CALL    UCROFF
        LD      A,(V_CUR)
        INC     A
        JR      SKSET

SKPGB:  LD      A,(V_PDEP)
        OR      A
        JR      Z,SFLOOP
        CALL    SPOPPG
        XOR     A
        LD      (V_CUR),A
        JP      SFDRAW
SKPGF:  LD      A,(V_CNT)
        CP      NROWS           ; only page forward off a full page
        JR      C,SFLOOP
        LD      A,(V_PDEP)
        CP      8
        JP      NC,SFLOOP
        ADD     A,A
        LD      E,A
        LD      D,0
        LD      HL,V_PSTK
        ADD     HL,DE
        LD      DE,(V_DPOS)
        LD      (HL),E
        INC     HL
        LD      (HL),D
        LD      A,(V_PDEP)
        INC     A
        LD      (V_PDEP),A
        LD      HL,(V_DNEXT)
        LD      (V_DPOS),HL
        XOR     A
        LD      (V_CUR),A
        JP      SFDRAW

; SPOPPG: pop the page stack into V_DPOS (or land on 0).
SPOPPG: LD      A,(V_PDEP)
        OR      A
        JR      Z,SPP0
        DEC     A
        LD      (V_PDEP),A
        ADD     A,A
        LD      E,A
        LD      D,0
        LD      HL,V_PSTK
        ADD     HL,DE
        LD      A,(HL)
        INC     HL
        LD      H,(HL)
        LD      L,A
        LD      (V_DPOS),HL
        RET
SPP0:   LD      HL,0
        LD      (V_DPOS),HL
        RET

; keypad 1: up one directory (the path always ends in '/').
SKUPD:  LD      A,(V_PATH+1)
        OR      A
        JP      Z,SFLOOP        ; at the root
        LD      HL,V_PATH
        CALL    USTRLN
        DEC     HL              ; the trailing '/'
SKUP1:  DEC     HL
        LD      A,(HL)
        CP      '/'
        JR      NZ,SKUP1
        INC     HL
        LD      (HL),0
        JP      SFRSET

; trigger: descend into a directory, or act on a file.
SKT:    LD      A,(V_CNT)
        OR      A
        JP      Z,SFLOOP
        LD      A,(V_CUR)
        LD      E,A
        LD      D,0
        LD      HL,V_EDIR
        ADD     HL,DE
        LD      A,(HL)
        CP      1
        JR      Z,SADV
        LD      A,(V_CMODE)
        OR      A
        JP      NZ,CPHERE       ; mid-copy: a file pick means "copy here"
        JP      BOOTSEL

; descend: full-length re-read, append (the firmware's trailing '/'
; included), restart paging.
SADV:   CALL    SFULL
        JP      C,SFDRAW
        LD      HL,V_PATH
        CALL    USTRLN
        LD      B,C
        PUSH    HL              ; the path's NUL
        LD      HL,V_ENTRY
        CALL    USTRLN
        POP     DE
        LD      A,B
        ADD     A,C
        JR      C,SADVF
        CP      190
        JR      NC,SADVF
        LD      HL,V_ENTRY
        CALL    USTRCP
        JP      SFRSET
SADVF:  LD      HL,TPLONG
        CALL    UMSG
        JP      SFDRAW

; keypad 4: edit the filter (a scratch copy -- ESC must not be
; destructive). A new pattern renumbers everything: paging restarts.
SKFLT:  LD      HL,V_FILT
        LD      DE,V_ENTRY
        LD      BC,32
        LDIR
        LD      HL,TEFILT
        LD      A,31
        CALL    EDIT
        JP      C,SFRSET
        LD      HL,V_ENTRY
        LD      DE,V_FILT
        LD      BC,32
        LDIR
        JP      SFRSET

; keypad 5: mark a copy source / perform the copy (copy.inc).
SKCPY:  LD      A,(V_CMODE)
        OR      A
        JP      NZ,CPHERE
        JP      CPMARK

; CE: back to the hosts (cancels an in-flight copy).
SKCE:   XOR     A
        LD      (V_CMODE),A
        JP      HOSTPG

SFERR:  LD      HL,EODIR
        CALL    UFAIL
        JP      HOSTPG

; ---- helpers ----------------------------------------------------------
; SFULL: re-read the selected row's name at FULLLEN into V_ENTRY.
; The directory is opened and closed here (stateless). Carry set on failure.
SFULL:  LD      A,(V_HOST)
        CALL    CODIR
        RET     C
        LD      A,(V_CUR)
        ADD     A,A
        LD      E,A
        LD      D,0
        LD      HL,V_EPOS
        ADD     HL,DE
        LD      A,(HL)
        INC     HL
        LD      H,(HL)
        LD      L,A
        LD      A,FULLLEN
        CALL    CDE16
        JR      C,SFULLF
        LD      HL,FNRDATA
        LD      DE,V_ENTRY
        LD      BC,FULLLEN
        LDIR
        XOR     A
        LD      (V_ENTRY+FULLLEN),A
        CALL    CCDIR
        OR      A
        RET
SFULLF: CALL    CCDIR
        SCF
        RET

; SISCFG: Z set when the name in the reply window ends ".cfg" (any case).
SISCFG: LD      HL,FNRDATA
        CALL    USTRLN
        LD      A,C
        CP      4
        JR      C,SNCFG
        DEC     HL
        DEC     HL
        DEC     HL
        DEC     HL
        LD      A,(HL)
        CP      '.'
        JR      NZ,SNCFG
        INC     HL
        LD      A,(HL)
        CALL    PFOLD
        CP      'C'
        JR      NZ,SNCFG
        INC     HL
        LD      A,(HL)
        CALL    PFOLD
        CP      'F'
        JR      NZ,SNCFG
        INC     HL
        LD      A,(HL)
        CALL    PFOLD
        CP      'G'
        RET
SNCFG:  LD      A,1
        OR      A
        RET

; STYPE: A = type of the name in the reply window:
; 1 = directory (trailing '/'), 2 = .bin cart, 0 = anything else.
STYPE:  LD      HL,FNRDATA
        CALL    USTRLN
        LD      A,C
        OR      A
        JR      Z,STY0
        DEC     HL
        LD      A,(HL)
        CP      '/'
        JR      NZ,STY1
        LD      A,1
        RET
STY1:   LD      A,C
        CP      4
        JR      C,STY0
        DEC     HL
        DEC     HL
        DEC     HL
        LD      A,(HL)
        CP      '.'
        JR      NZ,STY0
        INC     HL
        LD      A,(HL)
        CALL    PFOLD
        CP      'B'
        JR      NZ,STY0
        INC     HL
        LD      A,(HL)
        CALL    PFOLD
        CP      'I'
        JR      NZ,STY0
        INC     HL
        LD      A,(HL)
        CALL    PFOLD
        CP      'N'
        JR      NZ,STY0
        LD      A,2
        RET
STY0:   XOR     A
        RET

TEMPTD: DB      "<EMPTY> 4:FLT 1:UP CE",0
TSFOOT: DB      "1:UP 2/3:PG 4:FLT 5:CPY CE",0
TSCFOOT: DB     "5:COPY HERE  CE:CANCEL",0
TEFILT: DB      "FILTER (!NAME=SUBDIRS)",0
TPLONG: DB      "PATH TOO LONG",0
EODIR:  DB      "DIR ERR",0
