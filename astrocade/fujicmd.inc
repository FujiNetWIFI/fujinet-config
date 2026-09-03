; fujicmd.inc -- Fuji-device (70H) command wrappers on the fujilib transport.
;
; Payload shapes matter more than they look (learned the hard way on INTV,
; recorded in fujinet-config/intv/fujicmd.bas and honored here):
;   - The firmware's transaction_get() FAILS a short read, so OPEN_DIRECTORY
;     and SET_DEVICE_FULLPATH always send a full 256-byte NUL-padded payload.
;   - COPY_FILE is the exception: it reads the payload as a string, so it is
;     sent at EXACT length -- a padding NUL would land inside the destination
;     filename. Its host slot params are 1-BASED.
;   - SET_SSID needs nparam >= 1 (value ignored) and exactly 97 bytes:
;     ssid[33] + password[64].
;   - Directory EOF is exactly two 7FH bytes; directories get a trailing '/'
;     from the firmware; maxlen != 31 keeps icon bytes out of the reply.
;   - CONFIG_BOOT (0D9H) is deliberately never sent: CONFIG lives in the
;     cart's own flash, and the flag would only unmount device slot 0.

; ---- command ids beyond fujilib.inc's set -----------------------------
FCWHSL  EQU     0F3H            ; WRITE_HOST_SLOTS
FCSSID  EQU     0FBH            ; SET_SSID
FCSCAN  EQU     0FDH            ; SCAN_NETWORKS
FCSCRES EQU     0FCH            ; GET_SCAN_RESULT
FCWSTAT EQU     0FAH            ; GET_WIFISTATUS (3 = connected)
FCWEN   EQU     0EAH            ; GET_WIFI_ENABLED
FCGSSID EQU     0FEH            ; GET_SSID (ssid[33] + pass[64])
FCCOPY  EQU     0D8H            ; COPY_FILE ("src|dst", 1-based hosts)

; CGO: commit (8 quanta) and validate. Success: carry clear (and Z).
; Failure of any kind -- timeout, transport error, NAK -- returns carry set
; with A = the FNERR code (0FFH for timeout). Clobbers everything.
CGO:    LD      B,8
CGOB:   CALL    FNCOMMIT
        JR      C,CGOT
        OR      A
        JR      NZ,CGOF
        LD      A,(FNREPLY)
        CP      FCACK
        RET     Z
        LD      A,(FNERR)
CGOF:   SCF
        RET
CGOT:   LD      A,0FFH
        SCF
        RET

; CDO0: run the 0-param command E start to finish. Carry set on failure.
CDO0:   LD      A,FNDFUJI
        LD      L,0
        CALL    FNBEGIN
        JR      CGO

; CDO1: run command E with the single 1-byte param A. Carry set on failure.
CDO1:   PUSH    AF
        LD      A,FNDFUJI
        LD      L,1
        CALL    FNBEGIN
        POP     AF
        CALL    FNPARB
        JR      CGO

; FNPARW: append a 2-byte parameter with value HL. Clobbers A,D,E.
FNPARW: LD      A,2
        CALL    FNTXBYT
        LD      A,L
        CALL    FNTXBYT
        LD      A,H
        JP      FNTXBYT

; CTXSTR: stream the NUL-terminated string at HL, adding its length to C.
; Clobbers A,D,E,HL.
CTXSTR: LD      A,(HL)
        OR      A
        RET     Z
        CALL    FNTXBYT
        INC     HL
        INC     C
        JR      CTXSTR

; CTXFIX: stream exactly B bytes from HL; once a NUL is hit the remainder
; is streamed as zeros. Clobbers A,B,D,E,HL.
CTXFIX: LD      A,(HL)
        OR      A
        JR      Z,CTXF0
        CALL    FNTXBYT
        INC     HL
        DJNZ    CTXFIX
        RET
CTXF0:  XOR     A
        CALL    FNTXBYT
        DJNZ    CTXF0
        RET

; CPAD: pad the payload with zeros until C bytes total reach 256.
; (C = 0 means "none sent yet" and pads the full 256.) Clobbers A,B,D,E.
CPAD:   LD      B,C
CPAD1:  XOR     A
        CALL    FNTXBYT
        INC     B
        JR      NZ,CPAD1
        RET

; CMOUNT: MOUNT_HOST slot A. Carry set on failure.
CMOUNT: LD      E,FCMHOST
        JR      CDO1

; CODIR: OPEN_DIRECTORY on host A with V_PATH + V_FILT, padded to 256.
; Carry set on failure.
CODIR:  PUSH    AF
        LD      A,FNDFUJI
        LD      E,FCODIR
        LD      L,1
        CALL    FNBEGIN
        POP     AF
        CALL    FNPARB
        LD      C,0
        LD      HL,V_PATH
        CALL    CTXSTR
        XOR     A
        CALL    FNTXBYT
        INC     C
        LD      HL,V_FILT
        CALL    CTXSTR
        CALL    CPAD
        JP      CGO

; CCDIR: CLOSE_DIRECTORY. Carry set on failure.
CCDIR:  LD      E,FCCDIR
        JR      CDO0

; CDPOS: SET_DIRECTORY_POSITION HL (16-bit). Carry set on failure/NAK --
; a position past the end NAKs, which callers treat as end-of-directory.
CDPOS:  PUSH    HL
        LD      A,FNDFUJI
        LD      E,FCSDPS
        LD      L,1
        CALL    FNBEGIN
        POP     HL
        CALL    FNPARW
        JP      CGO

; CDENT: READ_DIR_ENTRY with maxlen A. Carry set on failure or on the
; 7FH,7FH end-of-directory marker; clear with the name in the reply window.
CDENT:  PUSH    AF
        LD      A,FNDFUJI
        LD      E,FCRDIR
        LD      L,2
        CALL    FNBEGIN
        POP     AF
        CALL    FNPARB
        XOR     A
        CALL    FNPARB
        CALL    CGO
        RET     C
        LD      A,(FNRDATA)
        CP      7FH
        JR      NZ,CDEOK
        LD      A,(FNRDATA+1)
        CP      7FH
        JR      NZ,CDEOK
        SCF
        RET
CDEOK:  OR      A
        RET

; CDE16: position HL, then read the entry at maxlen A. Carry set on
; failure or end of directory.
CDE16:  PUSH    AF
        CALL    CDPOS
        JR      C,CDE16F
        POP     AF
        JP      CDENT
CDE16F: POP     AF
        SCF
        RET
