; copy.inc -- COPY_FILE between hosts, with the .cfg sibling (block 1).
;
; As astrocade/copy.inc: keypad 5 on a file records its full path in
; V_SRC and re-skins the host page as "COPY TO HOST"; picking a slot
; drops into that host's root; 5 (or FIRE on any file) then copies into
; the directory being browsed. The copySpec is "srcfullpath|destdir +
; basename(src)", sent at EXACT length (no NUL, no padding -- a NUL
; would land in the destination name) with 1-BASED host slots.
;
; After a copy of a file with a 4-character extension the spec is sent
; again with both extensions swapped to ".cfg" (destination lowercase,
; the spelling the firmware tries first), retrying the SOURCE as ".CFG"
; for case-sensitive hosts. A miss is normal, not a failure. The TX
; stream is write-only, so each attempt rebuilds the spec.
;
; COPY_FILE gets the busy sweep and a 75 s wait (LCOMMIT); this cart's
; own deadline for it is 5 s until the firmware learns per-command
; timeouts, so a slow copy reports a timeout while the ESP32 carries on.

; 5, normal mode: mark the cursor's file as the copy source.
CPMARK: LODA,R0 V_CNT
        BCTA,EQ SFBAD
        LODA,R1 V_CUR
        LODA,R0 V_ROWM,R1
        ANDI,R0 $40
        BCTA,EQ CPM1
        LODI,R0 TNODIR>>8       ; a folder
        LODI,R1 TNODIR&$FF
        ZBSR    *ZUMSG
        ZBRR    *ZSFDRAW
CPM1:   ZBSR    *ZSFULL
        BCFA,EQ SFERR
        BSTA,UN SNAMLN          ; V_SRC = V_PATH + the full name
        BSTA,UN SPLEN
        LODZ    R3
        ADDA,R0 V_NLEN
        COMI,R0 PATHMAX+1
        BCTR,LT CPM2
        LODI,R0 TPLONG>>8
        LODI,R1 TPLONG&$FF
        ZBSR    *ZUMSG
        ZBRR    *ZSFDRAW
CPM2:   LODI,R3 $FF
CPM3:   LODA,R0 V_PATH,R3,+
        STRA,R0 V_SRC,R3
        BCFR,EQ CPM3
        LODI,R1 $FF             ; R3 = the path's NUL, where the name goes
CPM4:   LODA,R0 *FRDP,R1,+
        STRA,R0 V_SRC,R3
        ADDI,R3 1
        COMI,R0 0
        BCFR,EQ CPM4
        LODA,R0 V_HOST
        STRA,R0 V_CHOST
        LODI,R0 1
        STRA,R0 V_CMODE
        ZBSR    *ZSNDSEL
        ZBRR    *ZHOSTPG

; the copy itself, into the directory being browsed.
CPHERE: LODI,R3 $FF             ; V_SLEN = len(V_SRC)
CPH1:   LODA,R0 V_SRC,R3,+
        BCFR,EQ CPH1
        STRA,R3 V_SLEN
CPH2:   SUBI,R3 1               ; the basename: after the last '/'
        LODA,R0 V_SRC,R3
        COMI,R0 '/'
        BCFR,EQ CPH2
        ADDI,R3 1
        STRA,R3 V_BOFF
        LODA,R0 V_SLEN
        SUBZ    R3
        STRA,R0 V_BLEN
        PRINT   TCPING
        EORZ    R0              ; the real copy, original extensions
        BSTA,UN CPSEND
        BCFA,EQ CPFAIL
        LODA,R0 V_SLEN          ; a .cfg sibling only for ".xxx" names
        COMI,R0 5
        BCTA,LT CPOK
        SUBI,R0 4
        STRZ    R1
        LODA,R0 V_SRC,R1
        COMI,R0 '.'
        BCFA,EQ CPOK
        LODA,R0 V_BLEN
        COMI,R0 5               ; ".bin" alone has no basename
        BCTA,LT CPOK
        LODI,R0 1
        BSTA,UN CPSEND
        BCTA,EQ CPOK
        LODI,R0 2               ; case-sensitive host: source ".CFG"
        BSTA,UN CPSEND
CPOK:   LODI,R0 TCPOK>>8
        LODI,R1 TCPOK&$FF
        ZBSR    *ZUMSG
        BCTR,UN CPBACK
CPFAIL: STRZ    R2
        LODI,R0 ECOPY>>8
        LODI,R1 ECOPY&$FF
        ZBSR    *ZUFAIL

; back to where the copy started: the source host and directory,
; unfiltered (the filter belonged to the destination).
CPBACK: EORZ    R0
        STRA,R0 V_CMODE
        STRA,R0 V_FILT
        LODA,R0 V_CHOST
        STRA,R0 V_HOST
        LODA,R3 V_BOFF          ; V_PATH = V_SRC up to the basename
        EORZ    R0
        STRA,R0 V_PATH,R3
CPB1:   SUBI,R3 1
        LODA,R0 V_SRC,R3
        STRA,R0 V_PATH,R3
        COMI,R3 0
        BCFR,EQ CPB1
        ZBRR    *ZSFRSET

; CPSEND: one COPY_FILE. R0 = extension mode: 0 as recorded, 1 both
; ends ".cfg", 2 source ".CFG" / destination ".cfg". Results as CGO.
; Calls three down. Clobbers R0-R3.
CPSEND: STRA,R0 V_XMOD
        LODI,R3 FCCOPY
        LODI,R2 2
        ZBSR    *ZMSTART
        LODA,R1 V_CHOST         ; 1-BASED: source, then destination
        ADDI,R1 1
        ZBSR    *ZMPARB
        LODA,R1 V_HOST
        ADDI,R1 1
        ZBSR    *ZMPARB
        SETFS   V_SRC           ; the source path
        LODA,R2 V_SLEN
        BSTR,UN CPEXT
        LODI,R1 '|'
        LODA,R0 *FTXP,R1
        SETFS   V_PATH          ; the destination directory (ends '/')
        ZBSR    *ZCTXSTR
        LODA,R0 V_BOFF          ; + the basename
        ADDI,R0 V_SRC&$FF
        STRA,R0 FSTRP+1
        LODA,R2 V_BLEN
        LODA,R0 V_XMOD          ; the destination is always lowercase
        BCTR,EQ CPS1
        LODI,R0 1
        STRA,R0 V_XMOD
CPS1:   BSTR,UN CPEXT
        LODI,R1 70              ; 70 x 64 frames: ~75 s
        BCTA,UN LCOMMIT

; CPEXT: stream R2 bytes at (FSTRP), the last three replaced by "cfg"
; (V_XMOD 1) or "CFG" (2) unless V_XMOD is 0. Calls one down.
CPEXT:  LODA,R0 V_XMOD
        BCFR,EQ CPX1
        BCTA,UN CTXN
CPX1:   SUBI,R2 3
        BSTA,UN CTXN
        LODA,R0 V_XMOD
        COMI,R0 1
        BCTR,EQ CPX2
        SETFS   TCFGU
        ZBRR    *ZCTXSTR
CPX2:   SETFS   TCFGL
        ZBRR    *ZCTXSTR
