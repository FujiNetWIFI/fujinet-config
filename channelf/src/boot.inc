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

SBMIMG:	PI CMIMG
	CI 0
	BZ SBWAIT
	LR 1,A
	LIS 6
	LR 0,A
	PI SETERR
	JMP FAIL

; IDLE and XFER are the states before READY, not errors: only FAILED stops the
; wait.
SBWAIT:	DCI FNBSTAT
	LM
	CI FNB_RDY
	BZ SBGO
	CI FNB_FAIL
	BNZ SBWAIT
	DCI FNBERR
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

SBTITLE: DB "BOOTING",0
SBMNT:	DB "MOUNTING...",0
SBGOING: DB "DO NOT POWER OFF",0
