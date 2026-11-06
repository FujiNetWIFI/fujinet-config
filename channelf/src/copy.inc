; ---------------------------------------------------------------------------
; copy.inc -- copy a file to a folder on another host slot.
;
; As on the Astrocade and Arcadia: the copy key on a file records its full path
; in VSRC and turns the host list into "COPY TO HOST"; picking a slot opens that
; host's root in the browser, which now picks a folder (VCMODE); a file or the
; copy key there copies into the folder on screen, and the browser goes back to
; the folder the file came from.
;
; COPY_FILE is the odd one out in the whole command set: its payload goes at
; EXACT length rather than padded to 256, and its host slots are 1-BASED where
; every other command's are 0-based. Both are the server's rules; fujicmd.inc
; applies them so nothing here has to remember. The payload is "source|folder";
; the folder ends in '/', so the firmware names the copy after the source.
;
; The copy can take a long time over TNFS, and its ACK only arrives when the
; host-side copy finishes -- which is why the mailbox gives COPY_FILE the same
; extended budget as MOUNT_IMAGE.
; ---------------------------------------------------------------------------

; CPMARK -- the highlighted file, whole name in VNAME and short enough, becomes
;   the copy source, and the host list picks where it goes.
CPMARK:	PI PATHFULL		; VFULL = VPATH + VNAME
	DCI VFULL
	XDC
	DCI VSRC
	XDC
CPMCP:	LM
	XDC
	ST
	XDC
	CI 0
	BNZ CPMCP

	; VSRCD = the length of VPATH: the folder part of VSRC
	DCI VPATH
	CLR
	LR 1,A
CPMLEN:	LM
	CI 0
	BZ CPMLD
	LR A,1
	INC
	LR 1,A
	BR CPMLEN
CPMLD:	DCI VSRCD
	LR A,1
	ST

	DCI VHOST
	LM
	DCI VCHOST
	ST
	DCI VCMODE
	LIS 1
	ST
	LI ST_HOST
	LR 0,A
	JMP GOTO

; CPHERE -- copy VSRC into the folder on screen (VPATH on host VHOST). Reached
;   by JMP from the browser; goes back to the source folder on success and to
;   this browser on a failure.
CPHERE:	PI CPSAME
	LR A,0
	CI 0
	BZ CPHGO
	LI CVAL2
	LR 3,A
	DCI CPTSAME
	XDC
	PI NOTEKEY
	JMP SFDRAW

CPHGO:	LI CVAL2
	LR 3,A
	DCI CPTGOING
	XDC
	PI UIFOOT

	DCI VCHOST
	LM
	LR 0,A
	DCI VHOST
	LM
	LR 1,A
	PI CCOPY
	CI 0
	BZ CPHOK
	LI CVAL2
	LR 3,A
	DCI CPTFAIL
	XDC
	PI NOTEKEY
	JMP SFDRAW

CPHOK:	LI CVAL3
	LR 3,A
	DCI CPTDONE
	XDC
	PI NOTEKEY

; CPBACK -- leave copy mode and return to the folder the source came from, on
;   its host. Also the way out of the host list.
CPBACK:	DCI VCMODE
	CLR
	ST
	DCI VCHOST
	LM
	DCI VHOST
	ST
	DCI VSRCD
	LM
	LR 4,A			; r4 = how many bytes of folder to copy
	DCI VSRC
	XDC
	DCI VPATH		; DC0 = VPATH, DC1 = VSRC
CPBCP:	XDC
	LM
	XDC
	ST
	DS 4
	BNZ CPBCP
	CLR
	ST			; the NUL after the folder part
	DCI VPOSL
	CLR
	ST
	CLR
	ST
	DCI VCUR
	CLR
	ST
	LI ST_FILE
	LR 0,A
	JMP GOTO

; CPSAME -- r0 = 1 if the folder on screen is the one VSRC came from, on the
;   same host: copying a file onto itself would truncate it before it is read.
;   Leaf. Clobbers A, r0-r4, DC0, DC1.
CPSAME:	DCI VHOST
	LM
	LR 1,A
	DCI VCHOST
	LM
	XS 1
	BNZ CPSNO
	DCI VSRCD
	LM
	LR 2,A
	DCI VSRC
	XDC
	DCI VPATH		; DC0 = VPATH, DC1 = VSRC
CPSL:	LM			; VPATH[i]
	LR 4,A
	XDC
	LM			; VSRC[i]
	XDC
	XS 4
	BNZ CPSNO
	DS 2
	BNZ CPSL
	LM			; the folder on screen must end here
	CI 0
	BNZ CPSNO
	LIS 1
	LR 0,A
	POP
CPSNO:	CLR
	LR 0,A
	POP

CPTGOING: DB "COPYING...",0
CPTDONE: DB "COPIED - ANY KEY",0
CPTFAIL: DB "COPY FAILED - ANY KEY",0
CPTSAME: DB "SAME FOLDER - ANY KEY",0
