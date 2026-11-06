; ---------------------------------------------------------------------------
; boot.inc -- ST_BOOT: mount the selection and swap it in.
;
; The stub MUST run from the arena. The swap replaces every byte of the ROM
; window including the code that triggered it, and this console has no RAM of
; its own -- the cart's arena is the only memory a stub can execute from, which
; is why the arena survives a swap even though the mailbox decode does not.
; ---------------------------------------------------------------------------

SBOOT:
	LI CVAL3
	LR 3,A
	DCI SBTITLE
	XDC
	PI UITITLE
	PI UICLRL
	LI CVAL2
	LR 3,A
	DCI SBMNT
	XDC
	PI UIFOOT

	PI PATHFULL		; VFULL = VPATH + VNAME
	LI CVAL1
	LR 3,A
	DCI VFULL
	XDC
	PI TAILFIT
	CLR
	LR 0,A
	PI UIROW

	DCI VHOST
	LM
	LR 0,A
	PI CSDFP
	CI 0
	BZ SBMIMG
	LR 1,A
	LIS 5
	LR 0,A
	PI SETERR
	JMP FAIL

SBMIMG:	CLR
	LR 0,A
	PI SBBAR		; the empty bar
	PI SBCOMMIT
	CI 0
	BZ SBWAIT
	LR 1,A
	LIS 6
	LR 0,A
	PI SETERR
	JMP FAIL

; IDLE and XFER are the states before READY, not errors: only FAILED stops the
; wait. An ACKed mount that never leaves IDLE was served as a plain disk with
; nothing pushed, so the wait is bounded (~30 s).
SBWAIT:	LI SBWAITN
	DCI VBTMO
	ST
SBW1:	DCI FNBSTAT
	LM
	CI FNB_RDY
	BZ SBGO
	CI FNB_FAIL
	BZ SBBERR
	LI 32
	LR 3,A
	PI DELAYMS
	DCI VBTMO
	LM
	AI 0FFH
	DCI VBTMO
	ST
	CI 0
	BNZ SBW1
	LIS FNE_TMO
	LR 1,A
	LI 9
	LR 0,A
	PI SETERR
	JMP FAIL

SBBERR:	DCI FNBERR
	LM
	LR 1,A
	LIS 7
	LR 0,A
	PI SETERR
	JMP FAIL

SBGO:	LI CVAL2
	LR 3,A
	DCI SBGOING
	XDC
	PI UIFOOT

	DCI FNREG+FR_BLOK
	LI FN_BLKM
	ST			; arm the swap

	DCI STUB
	XDC
	DCI SBSTUB
	LI SBSTUBL
	LR 0,A
SBCPY:	LM
	XDC
	ST
	XDC
	DS 0
	BNZ SBCPY
	JMP STUB

; Seven bytes, assembled here and run from the arena: store anything to FNSWAP,
; then cold-start the BIOS so the new image's own $0800 header runs.
SBSTUB:	DCI FNSWAP
	ST
	JMP 00000H
SBSTUBL	EQU $-SBSTUB

; ---------------------------------------------------------------------------
; SBCOMMIT -- MOUNT_IMAGE with a progress bar. Returns A = 0, or an error code.
;
; The firmware pushes the whole image BEFORE it ACKs the mount, so FNGO would
; freeze the screen until the end. Commit by hand instead: store the sequence,
; then only read the status page while ACKSEQ is pending. The mailbox must not
; be written before the ACK; a write would queue the next transaction. Gives up
; after SBSTALL polls with no change in the percentage.
; ---------------------------------------------------------------------------
SBCOMMIT: ENTER
	LI FD_FUJI
	LR 0,A
	LI FC_MIMG
	LR 1,A
	LIS 2
	LR 2,A
	PI FNBEG
	CLR
	LR 0,A
	PI FNPB			; device slot 0
	LIS 1
	LR 0,A
	PI FNPB			; read mode

	DCI FNACKSQ
	LM
	INC
	BNZ SBC1
	LIS 1			; 0 is reserved: wrap 255 -> 1
SBC1:	DCI VBSEQ
	ST
	DCI FNREG+FR_SEQ
	ST			; launches the transaction
	DCI VBPCT
	LI 0FFH
	ST
	DCI VBTMO
	LI SBSTALL
	ST

SBCLP:	DCI VBSEQ
	LM
	LR 0,A
	DCI FNACKSQ
	LM
	XS 0
	BZ SBCDN
	DCI FNBPCT
	LM
	LR 0,A
	DCI FNBPCT
	LM
	XS 0
	BNZ SBCDLY		; changing as we read it: take the next poll
	DCI VBPCT
	LM
	XS 0
	BZ SBCDLY		; no change
	LR A,0
	DCI VBPCT
	ST
	PI SBBAR
	DCI VBTMO
	LI SBSTALL
	ST
SBCDLY:	LI 32
	LR 3,A
	PI DELAYMS
	DCI VBTMO
	LM
	AI 0FFH
	DCI VBTMO
	ST
	CI 0
	BNZ SBCLP
	LIS FNE_TMO
	BR SBCX

SBCDN:	PI FNACK
	CI 0
	BNZ SBCX
	LI 100
	LR 0,A
	PI SBBAR
	CLR
SBCX:	LEAVE

; ---------------------------------------------------------------------------
; SBBAR -- draw the bar for r0 percent: SBBARW cells on list row 3, one cell in
;   from the gutter, 5% a cell. Filled cells in CVAL3, the rest in CVAL1.
; ---------------------------------------------------------------------------
SBBAR:	ENTER
	LR A,0
	LR 7,A
	CLR
	LR 6,A			; r6 = cells filled
SBB1:	LR A,7
	AI 0FBH			; -5; carry while at least 5 remain
	BNC SBB2
	LR 7,A
	LR A,6
	INC
	LR 6,A
	BR SBB1
SBB2:	LR A,6
	AI 256-SBBARW-1
	BNC SBB3		; more than SBBARW (a garbled percentage): cap
	LI SBBARW
	LR 6,A
SBB3:	LR A,6
	DCI VBCEL
	ST
	LIS 3
	LR 0,A
	PI UIROWY
	LR A,2
	DCI VPIXY
	ST

	DCI VBCEL
	LM
	CI 0
	BZ SBB4
	LR 0,A
	LI CVAL3
	LR 3,A
	LI DORGX+DCELLW
	LR 1,A
	DCI VPIXY
	LM
	LR 2,A
	PI DFILLR
SBB4:	DCI VBCEL
	LM
	COM
	INC
	AI SBBARW
	LR 0,A			; cells left empty
	CI 0
	BZ SBBX
	DCI VBCEL
	LM
	SL 1
	SL 1
	AI DORGX+DCELLW
	LR 1,A
	LI CVAL1
	LR 3,A
	DCI VPIXY
	LM
	LR 2,A
	PI DFILLR
SBBX:	LEAVE

SBBARW	EQU 20			; bar width in cells
SBSTALL	EQU 255			; polls (~0.35 s each) with no progress: ~90 s
SBWAITN	EQU 85			; polls waiting for READY after the ACK: ~30 s

SBTITLE: DB "BOOTING",0
SBMNT:	DB "MOUNTING...",0
SBGOING: DB "DO NOT POWER OFF",0
