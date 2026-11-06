; ---------------------------------------------------------------------------
; browse.inc -- ST_FILE: the directory browser.
;
; Stateless by construction, as on the Astrocade and INTV: every page draw is
; OPEN_DIRECTORY, at most one SET_DIRECTORY_POSITION, one READ_DIR_ENTRY per
; row, and CLOSE_DIRECTORY. Nothing about the rows survives the draw: the names
; are drawn straight out of the reply window as they arrive.
;
; Rules the sibling ports paid for and this one keeps:
;   - every row is read at FULLLN, the widest the command allows, and cut to the
;     screen here. A narrower read makes the firmware drop a folder's '/' once
;     the name nears the width, and the folder then lists, and boots, as a file.
;     The cut name is not a filename; anything that has to OPEN -- descending,
;     booting, copying -- re-reads the row into VNAME and takes its type from
;     that.
;   - never read past the $7F,$7F end-of-directory marker. A fujinet-pc SD host
;     answers a position past the end with ".." forever, and after that every
;     SET_DIRECTORY_POSITION NAKs for the rest of the session.
;
; In copy mode (VCMODE) the same browser picks the destination folder: it
; descends as usual, and a file or the copy key copies into the folder on screen.
; ---------------------------------------------------------------------------

SFILE:
SFDRAW:	LI CVAL3
	LR 3,A
	DCI VPATH
	XDC
	PI TAILFIT
	PI UITITLE
	DCI VCMODE
	LM
	CI 0
	BNZ SFFCP
	LI CVAL1
	LR 3,A
	DCI SFFOOT
	BR SFFDRW
SFFCP:	LI CVAL2
	LR 3,A
	DCI SFFCOPY
SFFDRW:	XDC
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

	LI FULLLN
	LR 0,A
	PI CDENT
	CI 0
	BNZ SFEND		; error or the end-of-directory marker

	PI SFCRNCH
	LI CVAL1
	LR 3,A
	DCI VNROW
	LM
	LR 0,A
	DCI VDISP
	XDC
	PI UIROW

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

; SFPGDN -- the next page starts NROWS positions on. The position is 16 bits,
;   and the carry out of the low byte's add is the only thing that says to
;   bump the high byte: the sum can wrap to a nonzero value.
SFPGDN:	DCI VMORE
	LM
	CI 0
	BNZ SFPGD1
	JMP SFIN		; the page did not fill: there is no next page
SFPGD1:	DCI VPOSL
	LM
	LR 0,A
	LM
	LR 1,A
	LR A,0
	AI NROWS
	LR 0,A			; LR leaves the carry alone
	BNC SFPD1
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

; SFPGUP -- the page before starts NROWS positions back, or at the top when
;   there is less than a page above. The carry of the low byte's subtract says
;   whether it borrowed; BP and BM test the sign, which a low byte of 136 or more
;   gets wrong.
SFPGUP:	DCI VPOSL
	LM
	LR 0,A
	LM
	LR 1,A
	LR A,0
	AI -NROWS		; carry: the low byte was NROWS or more
	LR 0,A
	BC SFPUS
	LR A,1
	CI 0
	BZ SFPUZ		; nothing to borrow from: the very start
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

; SFSEL -- re-read the highlighted row whole, then descend or boot. In copy
;   mode a file means "copy into this folder".
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
	BNZ SFSD1
	DCI VCMODE
	LM
	CI 0
	BZ SFSD1
	JMP CPHERE
SFSD1:	PI PATHCHK
	LR A,0
	CI 0
	BZ SFSD2
	JMP SFLONG
SFSD2:	DCI VISDIR
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

; SFLONG -- the path would not fit in PATHMAX: say so, wait for a key, redraw.
SFLONG:	LI CVAL2
	LR 3,A
	DCI SFTOOL
	XDC
	PI NOTEKEY
	JMP SFDRAW

; SFCOPY -- the copy key. In copy mode it copies into the folder on screen;
;   otherwise it takes the highlighted file as the copy source and goes to pick
;   the destination host.
SFCOPY:	DCI VCMODE
	LM
	CI 0
	BZ SFCMRK
	JMP CPHERE
SFCMRK:	DCI VNROW
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
SFCFILE: PI PATHCHK
	LR A,0
	CI 0
	BZ SFCFIT
	JMP SFLONG
SFCFIT:	JMP CPMARK

; ---- helpers ---------------------------------------------------------------

; SFFULLN -- re-open the directory, re-read the row under the cursor whole into
;   VNAME, record whether it is a directory, and close. Returns A = 0 or an
;   error. Reached by PI; calls one level down.
SFFULLN: ENTER
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
	LR A,5			; LR leaves W alone: C is still the add's
	LR 1,A
	BNC SFFN1
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

	; the type comes from the whole name: a directory ends with '/'. A name
	; too long to carry one is cut short, and PATHMAX would refuse it anyway.
	PI SFCRNCH
	DCI VISDIR
	LR A,0
	ST

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
	PI CCDIR
	CLR
	LEAVE
SFFNC:	LR 6,A
	PI CCDIR
	LR A,6
SFFNX:	LEAVE

; SFCRNCH -- VDISP = the name in the reply window cut to NAMEW cells, and r0 = 1
;   if it is a directory (its last character is '/'). A longer name keeps
;   NAMEW-1 characters and ends in the marker that says what it is: '/' for a
;   directory, '~' for a file. Leaf. Clobbers A, r0-r3, DC0, DC1.
SFCRNCH: DCI FNREPLY
	CLR
	LR 1,A			; r1 = the last character
	LR 2,A			; r2 = the length
SFCR1:	LM
	CI 0
	BZ SFCR2
	LR 1,A
	LR A,2
	INC
	LR 2,A
	BR SFCR1
SFCR2:	LR A,1
	XI '/'
	BNZ SFCRF
	LIS 1
	LR 0,A
	BR SFCR3
SFCRF:	CLR
	LR 0,A
SFCR3:	DCI VDISP
	XDC			; DC1 = the destination
	DCI FNREPLY		; DC0 = the name
	LR A,2
	AI -(NAMEW+1)		; carry: longer than the row
	BC SFCRW
SFCRC:	LM			; fits: copy it all, NUL included
	XDC
	ST
	XDC
	CI 0
	BNZ SFCRC
	POP
SFCRW:	LI NAMEW-1
	LR 3,A
SFCRW1:	LM
	XDC
	ST
	XDC
	DS 3
	BNZ SFCRW1
	XDC			; DC0 = the destination, past the copied part
	LR A,0
	CI 0
	BNZ SFCRD
	LI '~'
	BR SFCRM
SFCRD:	LI '/'
SFCRM:	ST
	CLR
	ST
	POP

; TAILFIT -- DC1 = the string at DC1 as it fits NAMEW cells: itself, or '~' and
;   its last NAMEW-1 characters copied into VDISP. Leaf.
;   Clobbers A, r1, DC0, DC1, H.
TAILFIT: XDC			; DC0 = the string
	LR H,DC			; H = its start
	CLR
	LR 1,A
TFLEN:	LM
	CI 0
	BZ TFLEN2
	LR A,1
	INC
	LR 1,A
	BR TFLEN
TFLEN2:	LR A,1
	AI -(NAMEW+1)		; carry: longer than NAMEW
	BC TFCUT
	LR DC,H
	XDC			; DC1 = the string, as given
	POP
TFCUT:	LR A,1
	AI -(NAMEW-1)		; characters to skip
	LR 1,A
	LR DC,H
TFSKIP:	LM
	DS 1
	BNZ TFSKIP		; DC0 = the last NAMEW-1 characters
	XDC			; DC1 = the tail
	DCI VDISP
	LI '~'
	ST			; DC0 = VDISP+1
	XDC			; DC0 = the tail, DC1 = VDISP+1
TFCOPY:	LM
	XDC
	ST
	XDC
	CI 0
	BNZ TFCOPY
	DCI VDISP
	XDC
	POP

; PATHCHK -- r0 = 0 if VPATH followed by VNAME fits in PATHMAX characters, else 1.
;   The add can carry (VNAME may be 254 long), and a carry is "too long" before
;   any compare. Leaf. Clobbers A, r0-r2, DC0.
PATHCHK: DCI VPATH
	CLR
	LR 1,A
PCK1:	LM
	CI 0
	BZ PCK2
	LR A,1
	INC
	LR 1,A
	BR PCK1
PCK2:	DCI VNAME
	CLR
	LR 2,A
PCK3:	LM
	CI 0
	BZ PCK4
	LR A,2
	INC
	LR 2,A
	BR PCK3
PCK4:	LR A,1
	AS 2
	BC PCKBAD
	CI PATHMAX		; PATHMAX - total: carry when the total is not over
	BNC PCKBAD
	CLR
	LR 0,A
	POP
PCKBAD:	LIS 1
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
;   "/a/b/" becomes "/a/"; "/" stays "/". It keeps the places just past the last
;   two '/' as pointers in H and Q: ADC adds its accumulator as a signed number,
;   so an index past 127 would step backwards. Leaf. Clobbers A, DC0, DC1, H, Q.
PATHUP:	DCI VPATH+1
	LR Q,DC			; Q = past the second-last '/'
	LR H,DC			; H = past the last
	DCI VPATH
PUP1:	LM
	CI 0
	BZ PUP2
	CI '/'
	BNZ PUP1
	XDC			; Q = H, H = DC0, and DC0 stays where it is
	LR DC,H
	LR Q,DC
	XDC
	LR H,DC
	BR PUP1
PUP2:	LR DC,Q
	CLR
	ST
	POP

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
SFFCOPY: DB "FIRE:OPEN TWIST:COPY",0
SFTOOL:	DB "PATH TOO LONG - ANY KEY",0
