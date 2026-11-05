; ---------------------------------------------------------------------------
; browse.inc -- ST_FILE: the directory browser.
;
; Stateless by construction, as on the Astrocade and INTV: every page draw is
; OPEN_DIRECTORY, at most one SET_DIRECTORY_POSITION, one READ_DIR_ENTRY per
; row, and CLOSE_DIRECTORY. Only per-row metadata survives -- the entry type --
; because the names are drawn straight out of the reply window as they arrive.
;
; Two rules the sibling ports paid for and this one keeps:
;   - the drawn name is CRUNCHED to NAMELN and is not a filename. Anything that
;     has to OPEN -- descending, booting, copying -- re-reads the row at FULLLN.
;   - never read past the $7F,$7F end-of-directory marker. A fujinet-pc SD host
;     answers a position past the end with ".." forever, and after that every
;     SET_DIRECTORY_POSITION NAKs for the rest of the session.
; ---------------------------------------------------------------------------

SFILE:	ENTER
SFDRAW:	LI CVAL3
	LR 3,A
	DCI VPATH
	XDC
	PI UITITLE
	LI CVAL1
	LR 3,A
	DCI SFFOOT
	XDC
	PI UIFOOT

	DCI VHOST
	LM
	LR 0,A
	PI CODIR
	CI 0
	BZ SFROWS
	LR 1,A
	LIS 3
	LR 0,A
	PI SETERR
	JMP FAIL

SFROWS:	DCI VNROW
	CLR
	ST
	DCI VMORE
	CLR
	ST

	; Seek ONCE per page, and only when the page does not start at the top: a
	; freshly opened directory is already at position 0, and dir_seek(0) fails
	; outright on an empty directory, which would read as an error rather than
	; as the empty listing it is.
	DCI VPOSL
	LM
	LR 4,A
	LM
	LR 5,A
	LR A,4
	OI 0
	BNZ SFSEEK
	LR A,5
	OI 0
	BZ SFLOOP
SFSEEK:	LR A,4
	LR 0,A
	LR A,5
	LR 1,A
	PI CDPOS
	CI 0
	BZ SFLOOP
	JMP SFEND		; past the end: an empty page, not an error

SFLOOP:	DCI VNROW
	LM
	LR 0,A
	PI UICLRR

	LI NAMELN
	LR 0,A
	PI CDENT
	CI 0
	BNZ SFEND		; error or the end-of-directory marker

	LI CVAL1
	LR 3,A
	DCI VNROW
	LM
	LR 0,A
	DCI FNREPLY
	XDC
	PI UIROW

	; a directory arrives with a trailing '/' from the firmware; there is no
	; separate flag
	PI SFISDIR
	DCI VNROW
	LM
	LR 1,A
	DCI VDIR
	LR A,1
	ADC
	LR A,0
	ST

	DCI VNROW
	LM
	INC
	DCI VNROW
	ST
	CI NROWS
	BZ SFFULL
	JMP SFLOOP
SFFULL:	DCI VMORE
	LIS 1
	ST

SFEND:	PI CCDIR

	; blank whatever the page did not fill
	DCI VNROW
	LM
	DCI VROW
	ST
SFBLNK:	DCI VROW
	LM
	CI NROWS
	BZ SFCLMP
	DCI VROW
	LM
	LR 0,A
	PI UICLRR
	DCI VROW
	LM
	INC
	DCI VROW
	ST
	BR SFBLNK

	; keep the cursor inside the page
SFCLMP:	DCI VNROW
	LM
	LR 0,A
	CI 0
	BNZ SFCL1
	DCI VCUR
	CLR
	ST
	BR SFCUR
SFCL1:	DCI VCUR
	LM
	LR 1,A
	LR A,0
	COM
	INC
	AS 1			; cursor - rows
	BM SFCUR
	LR A,0
	AI 0FFH
	DCI VCUR
	ST

SFCUR:	PI UICUR

SFIN:	PI INSCAN
	LR A,0
	CI EV_NONE
	BZ SFIN
	CI EV_DOWN
	BNZ SFI1
	JMP SFDOWN
SFI1:	CI EV_UP
	BNZ SFI2
	JMP SFUP
SFI2:	CI EV_SEL
	BNZ SFI3
	JMP SFSEL
SFI3:	CI EV_BACK
	BNZ SFI4
	JMP SFBACK
SFI4:	CI EV_PGDN
	BNZ SFI5
	JMP SFPGDN
SFI5:	CI EV_PGUP
	BNZ SFI6
	JMP SFPGUP
SFI6:	CI EV_COPY
	BNZ SFI7
	JMP SFCOPY
SFI7:	CI EV_INFO
	BNZ SFIN
	LI ST_FILE
	LR 0,A
	DCI VPEND
	LR A,0
	ST
	LI ST_INFO
	LR 0,A
	JMP GOTO

SFDOWN:	DCI VCUR
	LM
	LR 0,A
	INC
	LR 1,A
	DCI VNROW
	LM
	LR 2,A
	LR A,1
	COM
	INC
	AS 2
	BM SFIN
	BZ SFIN
	LR A,1
	DCI VCUR
	ST
	PI UICUR
	JMP SFIN

SFUP:	DCI VCUR
	LM
	CI 0
	BZ SFIN
	AI 0FFH
	DCI VCUR
	ST
	PI UICUR
	JMP SFIN

SFPGDN:	DCI VMORE
	LM
	CI 0
	BNZ SFPGD1
	JMP SFIN		; the page did not fill: there is no next page
SFPGD1:
	DCI VPOSL
	LM
	AI NROWS
	LR 0,A
	LM
	LR 1,A
	LR A,0
	BNZ SFPD1
	LR A,1
	INC
	LR 1,A
SFPD1:	DCI VPOSL
	LR A,0
	ST
	LR A,1
	ST
	DCI VCUR
	CLR
	ST
	JMP SFDRAW

SFPGUP:	DCI VPOSL
	LM
	LR 0,A
	LM
	LR 1,A
	LR A,1
	CI 0
	BNZ SFPU1
	LR A,0
	CI NROWS
	BP SFPUZ		; below one page: go to the very start
SFPU1:	LR A,0
	CI NROWS
	BP SFPUB		; borrowing from the high byte
	LR A,0
	AI -NROWS
	LR 0,A
	BR SFPUS
SFPUB:	LR A,0
	AI -NROWS
	LR 0,A
	LR A,1
	AI 0FFH
	LR 1,A
SFPUS:	DCI VPOSL
	LR A,0
	ST
	LR A,1
	ST
	BR SFPUD
SFPUZ:	DCI VPOSL
	CLR
	ST
	CLR
	ST
SFPUD:	DCI VCUR
	CLR
	ST
	JMP SFDRAW

; SFBACK -- up one directory, or back to the host list at the root.
SFBACK:	DCI VPATH+1
	LM
	CI 0
	BNZ SFBUP		; not the root: drop a component
	LI ST_HOST
	LR 0,A
	JMP GOTO
SFBUP:	PI PATHUP
	DCI VPOSL
	CLR
	ST
	CLR
	ST
	DCI VCUR
	CLR
	ST
	JMP SFDRAW

; SFSEL -- re-read the highlighted row at full length, then descend or boot.
SFSEL:	DCI VNROW
	LM
	CI 0
	BNZ SFSHAVE
	JMP SFIN
SFSHAVE: PI SFFULLN
	CI 0
	BZ SFSOK
	LR 1,A
	LIS 4
	LR 0,A
	PI SETERR
	JMP FAIL
SFSOK:	DCI VISDIR
	LM
	CI 0
	BNZ SFSDIR
	LI ST_BOOT
	LR 0,A
	JMP GOTO
SFSDIR:	PI PATHAPP
	DCI VPOSL
	CLR
	ST
	CLR
	ST
	DCI VCUR
	CLR
	ST
	JMP SFDRAW

; SFCOPY -- take the highlighted file as the copy source.
SFCOPY:	DCI VNROW
	LM
	CI 0
	BNZ SFCHAVE
	JMP SFIN
SFCHAVE: PI SFFULLN
	CI 0
	BZ SFCOK
	LR 1,A
	LIS 4
	LR 0,A
	PI SETERR
	JMP FAIL
SFCOK:	DCI VISDIR
	LM
	CI 0
	BZ SFCFILE
	JMP SFIN		; directories are not copyable
SFCFILE: PI PATHFULL		; VFULL = VPATH + VNAME
	DCI VFULL
	XDC
	DCI VSRC
	XDC
SFCCP:	LM
	XDC
	ST
	XDC
	CI 0
	BNZ SFCCP
	LI ST_COPY
	LR 0,A
	JMP GOTO

; ---- helpers ---------------------------------------------------------------

; SFFULLN -- re-open the directory, re-read the row under the cursor at FULLLN
;   into VNAME, record whether it is a directory, and close. Returns A = 0 or an
;   error. Reached by PI; calls one level down.
SFFULLN: ENTER
	DCI VCUR
	LM
	LR 1,A
	DCI VDIR
	LR A,1
	ADC
	LM
	DCI VISDIR
	ST

	DCI VHOST
	LM
	LR 0,A
	PI CODIR
	CI 0
	BNZ SFFNX

	DCI VPOSL
	LM
	LR 4,A
	LM
	LR 5,A
	DCI VCUR
	LM
	AS 4
	LR 0,A
	LR A,5
	LR 1,A
	LR A,0
	BNZ SFFN1
	LR A,1
	INC
	LR 1,A
SFFN1:	LR A,0
	OI 0
	BNZ SFFNSK
	LR A,1
	OI 0
	BZ SFFNRD		; position 0 needs no seek
SFFNSK:	PI CDPOS
	CI 0
	BNZ SFFNC
SFFNRD:	LI FULLLN
	LR 0,A
	PI CDENT
	CI 0
	BNZ SFFNC

	; copy the name out before CLOSE repaints the reply window
	DCI VNAME
	XDC
	DCI FNREPLY
SFFNCP:	LM
	XDC
	ST
	XDC
	CI 0
	BNZ SFFNCP
	XDC
	PI CCDIR
	CLR
	LEAVE
SFFNC:	LR 6,A
	PI CCDIR
	LR A,6
SFFNX:	LEAVE

; SFISDIR -- r0 = 1 if the name in the reply window ends with '/'. Leaf.
SFISDIR: DCI FNREPLY
	CLR
	LR 1,A
SFID1:	LM
	CI 0
	BZ SFID2
	LR 1,A
	BR SFID1
SFID2:	LR A,1
	CI '/'
	BZ SFIDY
	CLR
	LR 0,A
	POP
SFIDY:	LIS 1
	LR 0,A
	POP

; PATHAPP -- append VNAME to VPATH. Leaf.
PATHAPP: DCI VPATH
PAPP1:	LM
	CI 0
	BNZ PAPP1
	LI 0FFH
	ADC			; LM left DC0 past the NUL; step back onto it
	XDC
	DCI VNAME
PAPP2:	LM
	XDC
	ST
	XDC
	CI 0
	BNZ PAPP2
	POP

; PATHUP -- drop the last component of VPATH, keeping the trailing '/'.
;   "/a/b/" becomes "/a/"; "/" stays "/". Leaf.
PATHUP:	DCI VPATH
	CLR
	LR 0,A
	LR 1,A
PUP1:	LM
	CI 0
	BZ PUP2
	CI '/'
	BNZ PUPN
	LR A,0
	LR 1,A
PUPN:	LR A,0
	INC
	LR 0,A
	BR PUP1
PUP2:	LR A,1
	CI 0
	BZ PUPX
	DCI VPATH
	CLR
	LR 0,A
	CLR
	LR 2,A
PUP3:	LR A,0
	XS 1
	BZ PUP4
	LM
	CI '/'
	BNZ PUPN2
	LR A,0
	LR 2,A
PUPN2:	LR A,0
	INC
	LR 0,A
	BR PUP3
PUP4:	DCI VPATH
	LR A,2
	INC
	ADC
	CLR
	ST
PUPX:	POP

; PATHFULL -- VFULL = VPATH followed by VNAME. Leaf.
PATHFULL: DCI VFULL
	XDC
	DCI VPATH
PF1:	LM
	CI 0
	BZ PF2
	XDC
	ST
	XDC
	BR PF1
PF2:	DCI VNAME
PF3:	LM
	XDC
	ST
	XDC
	CI 0
	BNZ PF3
	POP

SFFOOT:	DB "FIRE:OPEN MODE:UP",0
