; boot.inc -- SET_DEVICE_FULLPATH + MOUNT_IMAGE with a progress bar,
; then the swap.
;
; MOUNT_IMAGE makes the ESP32 pull the file and push it to the cartridge
; over the DBC side-channel WHILE the transaction is still outstanding, so
; the commit loop here is FNCOMMIT's proven shape (SEQ from ACKSEQ, never
; a local counter) with one addition: it watches FN_R_BOOT_PCT and paints
; the bar. Only passive status-window reads happen while the transaction
; is in flight -- no hotspots.
;
; On success the cart publishes BOOT_READY; we arm BOOTLOCK, copy the
; 6-byte stub to the top of screen RAM and jump: the cart flips to the
; staged image between the stub's swap read and the JP 0, and the OS
; cold-starts into it. A staged image carrying the "FUJI" claim keeps the
; mailbox after the swap; a plain commercial dump runs as a plain ROM
; with the mailbox dead. Both are firmware policy -- nothing to do here.
; CONFIG_BOOT (0D9H) is never sent.

BARY    EQU     40              ; progress bar: y, raw start byte, width
BARB    EQU     8
BARW    EQU     25              ; 25 bytes = 100 px = 4 px per percent

; boot the browser's selection: V_SRC = V_PATH + full entry name.
BOOTSEL:CALL    SFULL
        JP      C,SFDRAW
        CALL    SBLDSRC
        JP      C,SFDRAW
        LD      A,(V_HOST)

; BOOTGO: boot the file at V_SRC from host slot A.
BOOTGO: LD      (V_TMP+6),A
        LD      HL,TBOOT
        CALL    UTITLE
        LD      HL,V_SRC
        LD      B,26
        CALL    UTAIL
        LD      B,0
        LD      C,OPTDIM
        CALL    USHOWR
        LD      HL,TNOPOW
        CALL    UFOOT
        LD      A,FNDFUJI
        LD      E,FCSDFP
        LD      L,3
        CALL    FNBEGIN
        LD      A,DEVSLOT
        CALL    FNPARB
        LD      A,(V_TMP+6)
        CALL    FNPARB
        LD      A,FMREAD
        CALL    FNPARB
        LD      C,0
        LD      HL,V_SRC
        CALL    CTXSTR
        CALL    CPAD
        CALL    CGO
        JR      NC,BGMNT
        LD      HL,EPATH
BGFAIL: CALL    UFAIL
        JP      HOSTPG
BGMNT:  CALL    BARBG
        LD      A,FNDFUJI
        LD      E,FCMIMG
        LD      L,2
        CALL    FNBEGIN
        LD      A,DEVSLOT
        CALL    FNPARB
        LD      A,FMREAD
        CALL    FNPARB
        CALL    BCOMMIT
        JR      NC,BGRDY
        PUSH    AF              ; keep BCOMMIT's code; the FNBSTAT read below
        LD      A,(FNBSTAT)     ; prefer the boot-state verdict over the
        CP      FBFAIL          ; transport error -- it names the cause
        JR      NC,BGBERR2
        POP     AF              ; restore the code for UFAIL to show
        LD      HL,EMOUNT
        JR      BGFAIL
BGBERR2: POP     AF             ; discard it; BGBERR loads the boot error name
BGBERR: CALL    BERRNAM
        LD      A,(FNBERR)
        JR      BGFAIL
BGRDY:  LD      BC,3000         ; ~30 s for the stage to settle
BW1:    LD      A,(FNBSTAT)
        CP      FBREADY
        JR      Z,BREADY
        CP      FBFAIL
        JR      NC,BGBERR
        PUSH    BC
        CALL    DELAY10
        POP     BC
        DEC     BC
        LD      A,B
        OR      C
        JR      NZ,BW1
        LD      HL,ESTAGE
        LD      A,0FFH
        JR      BGFAIL
BREADY: LD      A,100
        CALL    BARUPD
        LD      C,FRBOOTL
        LD      A,FNBLMAG
        CALL    FNREGWR
        LD      HL,BSTUB
        LD      DE,STUB
        LD      BC,BSTUBL
        LDIR
        JP      STUB
BSTUB:  LD      A,(FNSWAP)      ; the cart swaps between this read and
        JP      0               ; the next fetch; nothing touches cart
BSTUBL  EQU     $-BSTUB         ; space in between -- we run from RAM

; BCOMMIT: commit the staged MOUNT_IMAGE, painting FN_R_BOOT_PCT into the
; bar while the transaction is outstanding. ~120 s budget. Carry set on
; timeout / transport error / NAK, exactly like CGO.
BCOMMIT:LD      A,0FFH
        LD      (V_TMP),A       ; last percent drawn
        LD      A,(FNACKSQ)
        INC     A
        JR      NZ,BC1
        INC     A               ; 255 wraps past the reserved 0
BC1:    LD      H,A
        LD      C,FRSEQ
        CALL    FNREGWR
        LD      B,60            ; 60 x ~2 s
BCO:    LD      DE,0
BCI:    LD      A,(FNACKSQ)
        CP      H
        JR      Z,BCOK
        LD      A,(FNBPCT)
        LD      L,A
        LD      A,(V_TMP)
        CP      L
        JR      Z,BCSKIP
        LD      A,L
        LD      (V_TMP),A
        PUSH    HL
        PUSH    DE
        PUSH    BC
        CALL    BARUPD
        POP     BC
        POP     DE
        POP     HL
BCSKIP: DEC     DE
        LD      A,D
        OR      E
        JR      NZ,BCI
        DJNZ    BCO
        SCF
        RET
BCOK:   LD      A,(FNERR)
        OR      A
        JR      NZ,BCF
        LD      A,(FNREPLY)
        CP      FCACK
        RET     Z
        LD      A,(FNERR)
BCF:    SCF
        RET

; BARBG: paint the empty bar (8 lines of color-1 pixels).
BARBG:  LD      HL,NORMEM+BARY*BYTEPL+BARB
        LD      B,8
BB1:    PUSH    BC
        PUSH    HL
        LD      BC,BARW
        LD      A,55H
        CALL    MSET
        POP     HL
        LD      DE,BYTEPL
        ADD     HL,DE
        POP     BC
        DJNZ    BB1
        RET

; BARUPD: A = percent 0-100: fill pct/4 bytes per line, percent text under.
BARUPD: PUSH    AF
        SRL     A
        SRL     A
        OR      A
        JR      Z,BAR2
        LD      C,A
        LD      HL,NORMEM+BARY*BYTEPL+BARB
        LD      B,8
BU1:    PUSH    BC
        PUSH    HL
        LD      B,0
        LD      A,0FFH
        CALL    MSET
        POP     HL
        LD      DE,BYTEPL
        ADD     HL,DE
        POP     BC
        DJNZ    BU1
BAR2:   POP     AF
        CALL    UPCT
        LD      HL,HEXBUF
        LD      DE,(BARY+12)*256+72
        LD      C,OPTFB
        SYSTEM  STRDIS
        RET

; BERRNAM: HL = the message for the FN_R_BOOT_ERR code.
BERRNAM:LD      A,(FNBERR)
        LD      HL,EBTOOB
        CP      1
        RET     Z
        LD      HL,EBTRUNC
        CP      2
        RET     Z
        LD      HL,EBNOMAP
        CP      3
        RET     Z
        LD      HL,ESTAGE
        RET

TBOOT:  DB      "BOOTING",0
TNOPOW: DB      "DO NOT POWER OFF",0
EPATH:  DB      "PATH ERR",0
EMOUNT: DB      "MOUNT ERR",0
ESTAGE: DB      "STAGE ERR",0
EBTOOB: DB      "ROM TOO BIG",0
EBTRUNC: DB     "TRUNCATED XFER",0
EBNOMAP: DB     "NO MAPPING",0
