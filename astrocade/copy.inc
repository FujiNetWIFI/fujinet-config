; copy.inc -- COPY_FILE between hosts, with the .cfg sibling.
;
; Ports intv/st_copy.bas: keypad 5 on a file records its full path in
; V_SRC and re-skins the host page as "COPY TO HOST"; picking a slot
; drops into that host's root; 5 again (or a trigger on any file) copies
; into the directory being browsed. The copySpec is "srcfullpath|destdir+
; basename(src)" -- basename only, so a "!name" recursive-filter hit like
; "roms/a/game.bin" never asks the destination for directories it doesn't
; have -- sent at EXACT length (no NUL, no padding) with 1-BASED host
; slots.
;
; After a copy of a ROM with a 4-character extension the copySpec is sent
; again with both extensions swapped to ".cfg" (destination lowercase --
; the spelling the firmware tries first), retrying the SOURCE as ".CFG"
; for case-sensitive hosts. A miss is normal, not a failure. Unlike INTV,
; which patches its TX buffer in place, the TX stream here is write-only,
; so each attempt rebuilds the spec (CPSEND).

; keypad 5, normal mode: mark the selected file as the copy source.
CPMARK: LD      A,(V_CNT)
        OR      A
        JP      Z,SFLOOP
        LD      A,(V_CUR)
        LD      E,A
        LD      D,0
        LD      HL,V_EDIR
        ADD     HL,DE
        LD      A,(HL)
        CP      1
        JR      NZ,CPM1
        LD      HL,TNODIR
        CALL    UMSG
        JP      SFDRAW
CPM1:   CALL    SFULL
        JP      C,SFDRAW
        CALL    SBLDSRC
        JP      C,SFDRAW
        LD      A,(V_HOST)
        LD      (V_CHOST),A
        LD      A,1
        LD      (V_CMODE),A
        JP      HOSTPG

; SBLDSRC: V_SRC = V_PATH + V_ENTRY (also the boot path builder).
; Carry set -- message already shown -- when it wouldn't fit.
SBLDSRC:LD      HL,V_PATH
        CALL    USTRLN
        LD      B,C
        LD      HL,V_ENTRY
        CALL    USTRLN
        LD      A,B
        ADD     A,C
        JR      C,SBLONG
        CP      224
        JR      NC,SBLONG
        LD      HL,V_PATH
        LD      DE,V_SRC
        CALL    USTRCP
        DEC     DE
        LD      HL,V_ENTRY
        CALL    USTRCP
        OR      A
        RET
SBLONG: LD      HL,TPLONG
        CALL    UMSG
        SCF
        RET

; the copy itself, into the directory being browsed.
CPHERE: LD      HL,V_SRC
        CALL    USTRLN
        LD      A,C
        LD      (V_TMP),A       ; source length
CPH1:   DEC     HL              ; find the basename (V_SRC begins '/')
        LD      A,(HL)
        CP      '/'
        JR      NZ,CPH1
        INC     HL
        LD      (V_TMP+2),HL    ; basename pointer
        CALL    USTRLN
        LD      A,C
        LD      (V_TMP+4),A     ; basename length
        LD      HL,V_PATH
        CALL    USTRLN          ; C = dest dir length
        LD      A,(V_TMP)       ; spec length = src + 1 + destdir + base
        ADD     A,C
        JR      C,CPLONG
        INC     A
        JR      C,CPLONG
        LD      B,A
        LD      A,(V_TMP+4)
        ADD     A,B
        JR      C,CPLONG
        CP      250
        JR      NC,CPLONG
        LD      HL,TCPING
        CALL    UFOOT
        XOR     A               ; the real copy, original extensions
        CALL    CPSEND
        JR      C,CPFAIL
        LD      A,(V_TMP)       ; .cfg sibling only for ".xxx" sources
        CP      5
        JR      C,CPOK
        LD      E,A
        LD      D,0
        LD      HL,V_SRC
        ADD     HL,DE
        DEC     HL
        DEC     HL
        DEC     HL
        DEC     HL
        LD      A,(HL)
        CP      '.'
        JR      NZ,CPOK
        LD      A,(V_TMP+4)
        CP      5               ; ".bin" alone has no basename
        JR      C,CPOK
        LD      A,1
        CALL    CPSEND
        JR      NC,CPOK
        LD      A,2             ; case-sensitive host: source as .CFG
        CALL    CPSEND
CPOK:   LD      HL,TCPOK
        CALL    UMSG
        JR      CPBACK
CPLONG: LD      HL,TPLONG
        CALL    UMSG
        JP      SFDRAW
CPFAIL: LD      HL,ECOPY
        CALL    UFAIL

; back to where the copy started: source host, source directory,
; unfiltered (the filter belonged to the destination browse).
CPBACK: XOR     A
        LD      (V_CMODE),A
        LD      (V_FILT),A
        LD      A,(V_CHOST)
        LD      (V_HOST),A
        LD      HL,(V_TMP+2)
        LD      DE,V_SRC
        OR      A
        SBC     HL,DE           ; HL = basename offset
        LD      A,H
        OR      A
        JR      NZ,CPROOT
        LD      A,L
        CP      191
        JR      NC,CPROOT       ; deeper than V_PATH holds: land on root
        LD      C,A
        LD      B,0
        LD      HL,V_SRC
        LD      DE,V_PATH
        LDIR
        XOR     A
        LD      (DE),A
        JP      SFRSET
CPROOT: LD      HL,V_PATH
        LD      (HL),'/'
        INC     HL
        LD      (HL),0
        JP      SFRSET

; CPSEND: send one COPY_FILE. A = extension mode: 0 = as recorded,
; 1 = both ends ".cfg", 2 = source ".CFG", destination ".cfg".
; Carry set on failure.
CPSEND: LD      (V_TMP+5),A
        LD      A,FNDFUJI
        LD      E,FCCOPY
        LD      L,2
        CALL    FNBEGIN
        LD      A,(V_CHOST)     ; 1-BASED slots, source then destination
        INC     A
        CALL    FNPARB
        LD      A,(V_HOST)
        INC     A
        CALL    FNPARB
        LD      A,(V_TMP)       ; source path
        LD      B,A
        LD      HL,V_SRC
        LD      A,(V_TMP+5)
        OR      A
        JR      Z,CPS1
        LD      A,B
        SUB     3
        LD      B,A
        CALL    CTXN
        LD      A,(V_TMP+5)
        LD      HL,TCFGL
        CP      2
        JR      NZ,CPS2
        LD      HL,TCFGU
CPS2:   CALL    FNTXSTR
        JR      CPS3
CPS1:   CALL    CTXN
CPS3:   LD      A,'|'
        CALL    FNTXBYT
        LD      HL,V_PATH       ; destination directory (ends '/')
        CALL    FNTXSTR
        LD      HL,(V_TMP+2)    ; basename
        LD      A,(V_TMP+5)
        OR      A
        JR      Z,CPS4
        LD      A,(V_TMP+4)
        SUB     3
        LD      B,A
        CALL    CTXN
        LD      HL,TCFGL        ; destination always lowercase
        CALL    FNTXSTR
        JR      CPS5
CPS4:   CALL    FNTXSTR
CPS5:   JP      CGO

; CTXN: stream exactly B raw bytes from HL. Clobbers A,B,D,E,HL.
CTXN:   LD      A,(HL)
        CALL    FNTXBYT
        INC     HL
        DJNZ    CTXN
        RET

TNODIR: DB      "CANNOT COPY A FOLDER",0
TCPING: DB      "COPYING...",0
TCPOK:  DB      "COPIED",0
ECOPY:  DB      "COPY FAILED",0
TCFGL:  DB      "cfg",0
TCFGU:  DB      "CFG",0
