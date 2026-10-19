; txlib.inc -- the page-0 half of the transport (block 1): TX payload
; streaming from RAM strings, the Fuji commands whose payloads come from
; RAM, the long frame-clocked commit, and the pointer words page-0 code
; reaches the mailbox through.
;
; FSTRP names its string with INDIRECT loads (LODA,R0 *FSTRP,R3,+), and
; an indirect operand must sit on the same page as the code -- FSTRP is
; page-0 RAM, so everything that dereferences it lives here. The string
; itself can be anywhere: RAM, block-2 ROM, or the reply window (the
; window holds still until the next commit, so streaming a reply back
; into TX is legal).

LISTLEN EQU     36              ; READ_DIR_ENTRY maxlen for listings
FULLLEN EQU     120             ; ... for names that must open

; ---- pointer words: the page-1 bytes page-0 code touches ---------------
FRDP:   DWBE    FNRDATA
FTXP:   DWBE    FNDATA
FACKP:  DWBE    FNACKSQ
FERRP:  DWBE    FNERR
FMAGFP: DWBE    FNMAGF

SETFS   MACRO   adr             ; FSTRP = adr (clobbers R0)
        LODI,R0 (adr)>>8
        STRA,R0 FSTRP
        LODI,R0 (adr)&$FF
        STRA,R0 FSTRP+1
        ENDM

; CTXSTR: stream the 0-terminated string at (FSTRP) -- not its NUL --
; and add its length to V_TXN. Leaf. Clobbers R0,R1,R3.
CTXSTR: LODI,R3 $FF
CTS1:   LODA,R0 *FSTRP,R3,+
        BCTR,EQ CTS2
        STRZ    R1
        LODA,R0 *FTXP,R1        ; TX append: the INDEX is the byte
        BCTR,UN CTS1
CTS2:   LODZ    R3
        ADDA,R0 V_TXN
        STRA,R0 V_TXN
        RETC,UN

; CTXFIX: stream exactly R2 (>0) bytes from (FSTRP); past its NUL the
; rest are zeros. Leaf. Clobbers R0-R3.
CTXFIX: LODI,R3 $FF
CTF1:   LODA,R0 *FSTRP,R3,+
        BCTR,EQ CTF2
        STRZ    R1
        LODA,R0 *FTXP,R1
        BDRR,R2 CTF1
        RETC,UN
CTF2:   LODA,R0 *FTXP           ; append 0
        BDRR,R2 CTF2
        RETC,UN

; CTXN: stream exactly R2 (>0) raw bytes from (FSTRP). Leaf.
; Clobbers R0-R3.
CTXN:   LODI,R3 $FF
CTN1:   LODA,R0 *FSTRP,R3,+
        STRZ    R1
        LODA,R0 *FTXP,R1
        BDRR,R2 CTN1
        RETC,UN

; CPAD: zeros until V_TXN payload bytes reach 256 -- OPEN_DIRECTORY and
; SET_DEVICE_FULLPATH fail a short payload on the ESP32. Leaf.
; Clobbers R0,R3.
CPAD:   LODA,R3 V_TXN
CPD1:   LODA,R0 *FTXP
        BIRR,R3 CPD1
        RETC,UN

; CMOUNT: MOUNT_HOST V_HOST. CCDIR: CLOSE_DIRECTORY. Results as CGO.
CMOUNT: LODA,R1 V_HOST
        LODI,R3 FCMHOST
        BCTA,UN CDO1
CCDIR:  LODI,R3 FCCDIR
        ZBRR    *ZCDO0

; CODIR: OPEN_DIRECTORY on V_HOST: "path\0filter\0", padded to 256.
; Results as CGO. Calls three down.
CODIR:  LODI,R3 FCODIR
        LODI,R2 1
        ZBSR    *ZMSTART
        LODA,R1 V_HOST
        ZBSR    *ZMPARB
        LODI,R0 1               ; the NUL between the two strings
        STRA,R0 V_TXN
        SETFS   V_PATH
        ZBSR    *ZCTXSTR
        LODA,R0 *FTXP
        SETFS   V_FILT
        ZBSR    *ZCTXSTR
        BSTA,UN CPAD
        ZBRR    *ZCGO

; CDPOS: SET_DIRECTORY_POSITION V_POS (u16). A position past the end
; NAKs, which callers read as end-of-directory. Calls three down.
CDPOS:  LODI,R3 FCSDPS
        LODI,R2 1
        ZBSR    *ZMSTART
        LODI,R2 2               ; a 2-byte parameter, little-endian
        LODA,R0 *FTXP,R2
        LODA,R2 V_POS+1
        LODA,R0 *FTXP,R2
        LODA,R2 V_POS
        LODA,R0 *FTXP,R2
        ZBRR    *ZCGO

; CDE16: the entry at V_POS, maxlen R1. Results as CDENT.
CDE16:  STRA,R1 V_MAXL
        BSTR,UN CDPOS
        BCFR,EQ CDE16X
        LODA,R1 V_MAXL
        BCTA,UN CDENT
CDE16X: RETC,UN

; LCOMMIT: commit, then wait up to R1 x 64 frames, one ACKSEQ poll per
; frame, ticking the busy sweep -- for the commands that take seconds
; (scan, copy). Results as CGO. Calls two down. Clobbers R0-R3, V_T0,
; V_LTIM/V_LSEQ/V_LTIC.
LCOMMIT: STRA,R1 V_LTIM
        BSTA,UN MSEQ
        STRA,R0 V_LSEQ
        LODI,R0 64
        STRA,R0 V_LTIC
LCM1:   LODA,R0 *FACKP
        COMA,R0 V_LSEQ
        BCTR,EQ LCMOK
        ZBSR    *ZWAITVB
        BSTA,UN SWTICK
        LODA,R0 V_LTIC
        SUBI,R0 1
        STRA,R0 V_LTIC
        BCFR,EQ LCM1
        LODI,R0 64
        STRA,R0 V_LTIC
        LODA,R0 V_LTIM
        SUBI,R0 1
        STRA,R0 V_LTIM
        BCFR,EQ LCM1
        LODI,R0 FETIMO
        RETC,UN
LCMOK:  LODA,R0 *FERRP
        BCTA,UN CGOCHK

; ---- window string helpers ------------------------------------------------
; SNAMLN: V_NLEN = R3 = length of the string at the reply window's start.
; Leaf. Clobbers R0,R3.
SNAMLN: LODI,R3 $FF
SNL1:   LODA,R0 *FRDP,R3,+
        BCFR,EQ SNL1
        STRA,R3 V_NLEN
        RETC,UN

; SPLEN: R3 = length of V_PATH. Leaf. Clobbers R0,R3.
SPLEN:  LODI,R3 $FF
SPL1:   LODA,R0 V_PATH,R3,+
        BCFR,EQ SPL1
        RETC,UN

; PFOLD: fold a-z in R0 to A-Z. Leaf.
PFOLD:  COMI,R0 'a'
        BCTR,LT PF9
        COMI,R0 'z'+1
        BCFR,LT PF9
        SUBI,R0 $20
PF9:    RETC,UN
