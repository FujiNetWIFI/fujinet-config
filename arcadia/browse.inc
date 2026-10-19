; browse.inc -- the directory browser (block 1).
;
; Stateless by construction, like the Astrocade and INTV ports: every
; page draw is MOUNT_HOST + OPEN_DIRECTORY(path+filter) + one
; SET_DIRECTORY_POSITION / READ_DIR_ENTRY pair per row +
; CLOSE_DIRECTORY, drawn straight from the reply window. Only per-row
; metadata survives (V_ROWM: offset from the page's top position and
; the directory flag). *.cfg entries are suppressed;
; directories arrive with a trailing '/' from the firmware and get the
; accent colour.
;
; Names are crunched to LISTLEN, so anything that must open (descend,
; boot, copy) re-reads the row at FULLLEN. A re-read leaves
; the directory open: CLOSE_DIRECTORY zeroes the window, and the next
; OPEN_DIRECTORY closes the old one anyway.
;
; Paging keeps an 8-entry ring of page-start positions (exact for the
; last 8 pages; past that, back steps 8 positions). The entry count for
; the scrollbar is found once per directory by an exponential probe and
; a bisection over SET_DIRECTORY_POSITION / READ_DIR_ENTRY.

; SRESET: the root of the mounted host. SFRST0: paging, cursor and entry
; count cleared (path kept). Leaf.
SRESET: LODI,R0 '/'
        STRA,R0 V_PATH
        EORZ    R0
        STRA,R0 V_PATH+1
SFRST0: EORZ    R0
        STRA,R0 V_DPOS
        STRA,R0 V_DPOS+1
        STRA,R0 V_SCUR
        STRA,R0 V_PNUM
        LODI,R0 $FF
        STRA,R0 V_TOT
        STRA,R0 V_TOT+1
        RETC,UN

; SFRSET: a new directory or filter: reset paging and redraw.
SFRSET: BSTR,UN SFRST0

; ---- page draw --------------------------------------------------------------
SFDRAW: LODI,R0 14
        STRA,R0 V_HLW
        BSTA,UN DCLS
        DCOL    AACC            ; title row: the tail of the path
        SETSTR  V_PATH
        DAT     0,0
        LODI,R1 16
        ZBSR    *ZDTAIL
        BSTA,UN CMOUNT
        BCFA,EQ SFERR
        BSTA,UN CODIR
        BCFA,EQ SFERR
        LODA,R0 V_TOT           ; count the entries once per directory
        COMI,R0 $FF
        BCFR,EQ SFD0
        BSTA,UN PROBE
SFD0:   EORZ    R0
        STRA,R0 V_CNT
        STRA,R0 V_EOF
        LODA,R0 V_DPOS
        STRA,R0 V_DNEXT
        LODA,R0 V_DPOS+1
        STRA,R0 V_DNEXT+1
SFD1:   LODA,R0 V_CNT
        COMI,R0 8
        BCTA,EQ SFDE
        LODA,R0 V_DNEXT+1       ; offset from the top (< 64 or stop)
        SUBA,R0 V_DPOS+1
        COMI,R0 64
        BCFA,LT SFDE
        STRA,R0 V_T2
        LODA,R0 V_DNEXT
        STRA,R0 V_POS
        LODA,R0 V_DNEXT+1
        STRA,R0 V_POS+1
        LODI,R1 LISTLEN
        BSTA,UN CDE16
        BCTR,EQ SFD2
        LODI,R0 1
        STRA,R0 V_EOF
        BCTA,UN SFDE
SFD2:   BSTA,UN SNAMLN
        BSTA,UN SISCFG
        BCTA,EQ SFDNX           ; a .cfg: suppressed
        DCOL    ANORM
        BSTA,UN SISDIR
        BCFR,EQ SFD3
        LODA,R0 V_T2
        IORI,R0 $40
        STRA,R0 V_T2
        DCOL    AACC
SFD3:   LODA,R0 V_CNT           ; the row, cols 0-13
        STRA,R0 V_CUR
        ZBSR    *ZROWCEL
        SETSTR  FNRDATA
        LODI,R1 14
        ZBSR    *ZDPRTN
        LODA,R1 V_CNT
        LODA,R0 V_T2
        STRA,R0 V_ROWM,R1
        ADDI,R1 1
        STRA,R1 V_CNT
SFDNX:  LODA,R0 V_DNEXT+1
        ADDI,R0 1
        STRA,R0 V_DNEXT+1
        BCFA,EQ SFD1
        LODA,R0 V_DNEXT
        ADDI,R0 1
        STRA,R0 V_DNEXT
        BCTA,UN SFD1

SFDE:   BSTA,UN CCDIR
        LODA,R0 V_SCUR          ; the cursor row to come back to
        STRA,R0 V_CUR
        LODA,R0 V_CNT
        BCFR,EQ SFDF
        LODA,R0 V_DPOS          ; empty page: paged past the end?
        IORA,R0 V_DPOS+1
        BCFA,EQ SKPGB0
        PRINT   TEMPTD
        BCTR,UN SFLEG
SFDF:   LODA,R0 V_CUR           ; keep the cursor on a real row
        COMA,R0 V_CNT
        BCTR,LT SFDF1
        LODA,R0 V_CNT
        SUBI,R0 1
        STRA,R0 V_CUR
SFDF1:  BSTA,UN LHI
SFLEG:  BSTA,UN SBAR
        LODA,R0 V_CMODE
        BCTR,EQ SFL1
        DCOL    AACC            ; copy mode: the file being copied
        SETSTR  V_SRC
        DAT     9,0
        LODI,R1 16
        ZBSR    *ZDTAIL
        PRINT   LBCOPY
        BCTR,UN SFLOOP
SFL1:   LODA,R0 V_FILT          ; a filter is shown on row 9
        BCTR,EQ SFL2
        PRINT   TFLT
        DCOL    AACC
        SETSTR  V_FILT
        BSTA,UN DPRINT
SFL2:   PRINT   LBRWS

; ---- input loop -------------------------------------------------------------
SFLOOP: ZBSR    *ZLKEY
        LODA,R1 V_CUR
        STRA,R1 V_SCUR
        COMI,R0 KFIRE
        BCTA,EQ SKT
        COMI,R0 KENT
        BCTA,EQ SKT
        COMI,R0 KRIGHT
        BCTA,EQ SKPGF
        COMI,R0 KLEFT
        BCTA,EQ SKPGB
        COMI,R0 1
        BCTA,EQ SKUPD
        COMI,R0 5
        BCTA,EQ SKCPY
        COMI,R0 7
        BCTA,EQ SKFLT
        COMI,R0 KCLR
        BCTA,EQ SKCE
        BCTR,UN SFLOOP

; FIRE: descend into a directory, or act on a file.
SKT:    LODA,R0 V_CNT
        BCTA,EQ SFBAD
        LODA,R1 V_CUR
        LODA,R0 V_ROWM,R1
        ANDI,R0 $40
        BCFR,EQ SADV
        LODA,R0 V_CMODE
        BCTA,EQ BOOTSEL
        BCTA,UN CPHERE          ; mid-copy: a file pick means "copy here"

; descend: full-length re-read, append (the firmware's '/' included).
SADV:   ZBSR    *ZSFULL
        BCFA,EQ SFERR
        BSTA,UN SNAMLN
        BSTA,UN SPLEN
        LODZ    R3
        ADDA,R0 V_NLEN
        COMI,R0 PATHMAX+1
        BCTA,LT SAD0
        LODI,R0 TPLONG>>8
        LODI,R1 TPLONG&$FF
        ZBSR    *ZUMSG
        ZBRR    *ZSFDRAW
SAD0:   LODI,R1 0
SAD1:   LODA,R0 *FRDP,R1
        STRA,R0 V_PATH,R3       ; STRA keeps the load's CC
        BCTR,EQ SAD2
        ADDI,R1 1
        ADDI,R3 1
        BCTR,UN SAD1
SAD2:   ZBSR    *ZSNDSEL
        ZBRR    *ZSFRSET

; 1: up one directory (the path always ends in '/').
SKUPD:  LODA,R0 V_PATH+1
        BCTA,EQ SFBAD           ; at the root
        BSTA,UN SPLEN
        SUBI,R3 1               ; the trailing '/'
SKU1:   SUBI,R3 1
        LODA,R0 V_PATH,R3
        COMI,R0 '/'
        BCFR,EQ SKU1
        EORZ    R0
        STRA,R0 V_PATH+1,R3
        ZBSR    *ZSNDBCK
        ZBRR    *ZSFRSET

; right / 6: the next page, if there is one.
SKPGF:  LODA,R0 V_CNT
        COMI,R0 8
        BCFA,EQ SFBAD
        LODA,R0 V_EOF
        BCFA,EQ SFBAD
        LODA,R0 V_DNEXT         ; DNEXT < TOT ?
        COMA,R0 V_TOT
        BCTR,LT SKPF1
        BCFA,EQ SFBAD
        LODA,R0 V_DNEXT+1
        COMA,R0 V_TOT+1
        BCFA,LT SFBAD
SKPF1:  LODA,R0 V_PHEAD         ; push DPOS on the ring
        ANDI,R0 7
        RRL,R0
        STRZ    R1
        LODA,R0 V_DPOS
        STRA,R0 V_PSTK,R1
        LODA,R0 V_DPOS+1
        STRA,R0 V_PSTK+1,R1
        LODA,R0 V_PHEAD
        ADDI,R0 1
        STRA,R0 V_PHEAD
        LODA,R0 V_PNUM
        COMI,R0 8
        BCFR,LT SKPF2
        ADDI,R0 1
        STRA,R0 V_PNUM
SKPF2:  LODA,R0 V_DNEXT
        STRA,R0 V_DPOS
        LODA,R0 V_DNEXT+1
        STRA,R0 V_DPOS+1
SKPGO:  EORZ    R0
        STRA,R0 V_SCUR
        BSTA,UN SNDPG
        ZBRR    *ZSFDRAW

; left / 4: the previous page (also the empty-page recovery, SKPGB0).
SKPGB:  LODA,R0 V_DPOS
        IORA,R0 V_DPOS+1
        BCTA,EQ SFBAD           ; already on the first page
SKPGB0: LODA,R0 V_PNUM
        BCTR,EQ SKPB1
        SUBI,R0 1
        STRA,R0 V_PNUM
        LODA,R0 V_PHEAD
        SUBI,R0 1
        STRA,R0 V_PHEAD
        ANDI,R0 7
        RRL,R0
        STRZ    R1
        LODA,R0 V_PSTK,R1
        STRA,R0 V_DPOS
        LODA,R0 V_PSTK+1,R1
        STRA,R0 V_DPOS+1
        BCTR,UN SKPGO
SKPB1:  PPSL    $09             ; off the ring: DPOS -= 8, floor 0
        LODA,R0 V_DPOS+1
        SUBI,R0 8
        STRA,R0 V_DPOS+1
        LODA,R0 V_DPOS
        SUBI,R0 0
        CPSL    $08
        STRA,R0 V_DPOS
        TPSL    $01
        BCTA,EQ SKPGO           ; no borrow out of the top
        EORZ    R0
        STRA,R0 V_DPOS
        STRA,R0 V_DPOS+1
        BCTA,UN SKPGO

; 5: mark a copy source / copy here (copy.inc).
SKCPY:  LODA,R0 V_CMODE
        BCTA,EQ CPMARK
        BCTA,UN CPHERE

; 7: edit the filter on a scratch copy (ESC must not be destructive).
SKFLT:  LODI,R3 $FF
SKF1:   LODA,R0 V_FILT,R3,+
        STRA,R0 V_FSCR,R3
        BCFR,EQ SKF1
        LODI,R0 V_FSCR>>8
        STRA,R0 V_EBUF
        LODI,R0 V_FSCR&$FF
        STRA,R0 V_EBUF+1
        LODI,R0 FILTMAX
        STRA,R0 V_EMAX
        LODI,R0 TEFILT>>8
        LODI,R1 TEFILT&$FF
        BSTA,UN EDIT
        BCTA,EQ SKF2
        ZBRR    *ZSFDRAW
SKF2:   LODI,R3 $FF
SKF3:   LODA,R0 V_FSCR,R3,+
        STRA,R0 V_FILT,R3
        BCFR,EQ SKF3
        ZBRR    *ZSFRSET

; Clear: back to the hosts (cancels a copy in progress).
SKCE:   EORZ    R0
        STRA,R0 V_CMODE
        ZBSR    *ZSNDBCK
        ZBRR    *ZHOSTPG

SFBAD:  ZBSR    *ZSNDERR
        BCTA,UN SFLOOP

SFERR:  STRZ    R2
        LODI,R0 EODIR>>8
        LODI,R1 EODIR&$FF
        ZBSR    *ZUFAIL
        ZBRR    *ZHOSTPG

; ---- helpers ----------------------------------------------------------------
; SFULL: re-read the cursor row at FULLLEN into the window (the
; directory is left open). Results as CDENT. Calls four down.
SFULL:  BSTA,UN CODIR
        BCFR,EQ SFUL9
        LODA,R1 V_CUR
        LODA,R0 V_ROWM,R1
        ANDI,R0 $3F
        ADDA,R0 V_DPOS+1
        STRA,R0 V_POS+1
        LODA,R0 V_DPOS
        TPSL    $01             ; LODA left the add's carry alone
        BCFR,EQ SFUL1
        ADDI,R0 1
SFUL1:  STRA,R0 V_POS
        LODI,R1 FULLLEN
        BSTA,UN CDE16
SFUL9:  RETC,UN

; SISCFG: CC EQ when the window name (V_NLEN long) ends ".cfg", any
; case. Calls one down. Clobbers R0,R1,R3.
SISCFG: LODA,R3 V_NLEN
        COMI,R3 4
        BCTR,LT SNCFG
        SUBI,R3 4
        LODA,R0 *FRDP,R3
        COMI,R0 '.'
        BCFR,EQ SNCFG
        LODI,R1 0
SCF1:   ADDI,R3 1
        LODA,R0 *FRDP,R3
        BSTA,UN PFOLD
        COMA,R0 TCFG3,R1
        BCFR,EQ SNCFG
        ADDI,R1 1
        COMI,R1 3
        BCTR,LT SCF1
        EORZ    R0
        RETC,UN
SNCFG:  LODI,R0 1
        RETC,UN
TCFG3:  DB      "CFG"

; SISDIR: CC EQ when the window name ends in '/'. Leaf. Clobbers R0,R3.
SISDIR: LODA,R3 V_NLEN
        BCTR,EQ SNDIR
        SUBI,R3 1
        LODA,R0 *FRDP,R3
        COMI,R0 '/'
        RETC,UN
SNDIR:  LODI,R0 1
        RETC,UN

; ---- the entry count ----------------------------------------------------------
; PROBE: V_TOT = the number of entries in the open directory: entry 0,
; then 8, 16, 32 ... until one is missing, then a bisection. A missing
; entry is a NAK on the position or a $7F,$7F read; capped at $7FFF.
; Scratch: V_T2:3 LO (exists), V_T4:5 HI (missing). Calls five down.
PROBE:  EORZ    R0
        STRA,R0 V_T2
        STRA,R0 V_T3
        STRA,R0 V_T4
        STRA,R0 V_T5
        BSTA,UN PRBHI           ; is there an entry 0 at all?
        BCFA,EQ PRB9            ; no: HI = 0 = the count
        LODI,R0 8
        STRA,R0 V_T5
PRB1:   BSTA,UN PRBHI           ; HI exists: LO = HI, HI *= 2
        BCFR,EQ PRB2
        LODA,R0 V_T4
        STRA,R0 V_T2
        LODA,R0 V_T5
        STRA,R0 V_T3
        LODA,R0 V_T4
        COMI,R0 $40
        BCTA,EQ PRBCAP
        CPSL    $01
        PPSL    $08
        LODA,R0 V_T5
        RRL,R0
        STRA,R0 V_T5
        LODA,R0 V_T4
        RRL,R0
        STRA,R0 V_T4
        CPSL    $09
        BCTR,UN PRB1
PRB2:   PPSL    $09             ; bisect while HI - LO > 1
        LODA,R0 V_T5
        SUBA,R0 V_T3
        STRZ    R1
        LODA,R0 V_T4
        SUBA,R0 V_T2
        CPSL    $09
        BCFR,EQ PRB3            ; high byte of the gap nonzero
        COMI,R1 1
        BCFA,GT PRB3
        BCTA,UN PRB9
PRB3:   CPSL    $01             ; V_POS = (LO + HI) / 2
        PPSL    $08
        LODA,R0 V_T3
        ADDA,R0 V_T5
        STRA,R0 V_POS+1
        LODA,R0 V_T2
        ADDA,R0 V_T4
        RRR,R0
        STRA,R0 V_POS
        LODA,R0 V_POS+1
        RRR,R0
        STRA,R0 V_POS+1
        CPSL    $09
        BSTA,UN PRBPOS
        BCFR,EQ PRB4
        LODA,R0 V_POS           ; exists: LO = mid
        STRA,R0 V_T2
        LODA,R0 V_POS+1
        STRA,R0 V_T3
        BCTA,UN PRB2
PRB4:   LODA,R0 V_POS           ; missing: HI = mid
        STRA,R0 V_T4
        LODA,R0 V_POS+1
        STRA,R0 V_T5
        BCTA,UN PRB2
PRBCAP: LODI,R0 $7F
        STRA,R0 V_T4
        LODI,R0 $FF
        STRA,R0 V_T5
PRB9:   LODA,R0 V_T4
        STRA,R0 V_TOT
        LODA,R0 V_T5
        STRA,R0 V_TOT+1
        RETC,UN

; PRBHI: does entry HI exist? PRBPOS: does entry V_POS exist? CC EQ yes.
PRBHI:  LODA,R0 V_T4
        STRA,R0 V_POS
        LODA,R0 V_T5
        STRA,R0 V_POS+1
PRBPOS: LODI,R1 2
        BCTA,UN CDE16
