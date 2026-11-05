; ---------------------------------------------------------------------------
; copy.inc -- ST_COPY: copy the chosen file to another host slot.
;
; COPY_FILE is the odd one out in the whole command set: its payload goes at
; EXACT length rather than padded to 256, and its host slots are 1-BASED where
; every other command's are 0-based. Both are the server's rules; fujicmd.inc
; applies them so nothing here has to remember.
;
; The copy can take a long time over TNFS, and its ACK only arrives when the
; host-side copy finishes -- which is why the mailbox gives COPY_FILE the same
; extended budget as MOUNT_IMAGE.
; ---------------------------------------------------------------------------

SCOPY:	ENTER
	LI CVAL3
	LR 3,A
	DCI SCTITLE
	XDC
	PI UITITLE
	LI CVAL1
	LR 3,A
	DCI SCFOOT
	XDC
	PI UIFOOT

	PI CRHOSTS
	CI 0
	BZ SCOK
	LR 1,A
	LIS 9
	LR 0,A
	PI SETERR
	JMP FAIL

SCOK:	DCI FNREPLY
	XDC
	DCI VHOSTS
	XDC
	LI 0
	LR 4,A
SCCP:	LM
	XDC
	ST
	XDC
	DS 4
	BNZ SCCP

	DCI VHTOP
	CLR
	ST
	DCI VCUR
	CLR
	ST

SCDRAW:	PI SHROWS
	PI UICUR

SCIN:	PI INSCAN
	LR A,0
	CI EV_NONE
	BZ SCIN
	CI EV_DOWN
	BNZ SCI1
	JMP SHDOWN		; the host list's own movement, reused wholesale
SCI1:	CI EV_UP
	BNZ SCI2
	JMP SHUP
SCI2:	CI EV_BACK
	BNZ SCI3
	JMP SCBACK
SCI3:	CI EV_SEL
	BNZ SCIN
	JMP SCGO

SCBACK:	LI ST_FILE
	LR 0,A
	JMP GOTO

SCGO:	PI SHSLOT
	DCI VCOPYH
	LR A,0
	ST

	LI CVAL2
	LR 3,A
	DCI SCGOING
	XDC
	PI UIFOOT

	DCI VHOST
	LM
	LR 0,A
	DCI VCOPYH
	LM
	LR 1,A
	PI CCOPY
	CI 0
	BZ SCDONE
	LR 1,A
	LI 10
	LR 0,A
	PI SETERR
	JMP FAIL

SCDONE:	LI CVAL3
	LR 3,A
	DCI SCDONES
	XDC
	PI UIFOOT
SCWAIT:	PI INSCAN
	LR A,0
	CI EV_NONE
	BZ SCWAIT
	LI ST_FILE
	LR 0,A
	JMP GOTO

SCTITLE: DB "COPY TO HOST",0
SCFOOT:	DB "FIRE:HERE MODE:BACK",0
SCGOING: DB "COPYING...",0
SCDONES: DB "COPIED - ANY KEY",0
