; ---------------------------------------------------------------------------
; hosts.inc -- ST_HOST: the eight host slots.
;
; READ_HOST_SLOTS returns 8 x 32 ASCII bytes in one reply, so the whole screen
; is one round trip. Eight slots do not fit in a seven-row viewport, so the list
; scrolls: VHTOP is the first slot shown and VCUR the row within the viewport.
;
; An empty slot shows as "-- EMPTY --" and can be renamed; a named one mounts.
; ---------------------------------------------------------------------------

VHTOP	EQU 0801CH		; first host slot visible
VHSEL	EQU 0801DH		; slot the cursor is on

SHOST:	ENTER
	LI CVAL3
	LR 3,A
	DCI SHTITLE
	XDC
	PI UITITLE
	LI CVAL1
	LR 3,A
	DCI SHFOOT
	XDC
	PI UIFOOT

	PI CRHOSTS
	CI 0
	BZ SHOK
	LR 1,A
	LIS 1
	LR 0,A
	PI SETERR
	JMP FAIL

	; copy the 8 x 32 table out of the reply window: CLOSE and the next
	; transaction both repaint it, and the names are needed for the whole
	; time the screen is up
SHOK:	DCI FNREPLY
	XDC
	DCI VHOSTS
	XDC			; DC0 = reply, DC1 = VHOSTS
	LI 0
	LR 4,A			; 256 bytes, counted by DS wrapping 0 -> 0
SHCP:	LM
	XDC
	ST
	XDC
	DS 4
	BNZ SHCP

	DCI VHTOP
	CLR
	ST
	DCI VCUR
	CLR
	ST

SHDRAW:	PI SHROWS
	PI UICUR

SHIN:	PI INSCAN
	LR A,0
	CI EV_NONE
	BZ SHIN
	CI EV_DOWN
	BNZ SHI1
	JMP SHDOWN
SHI1:	CI EV_UP
	BNZ SHI2
	JMP SHUP
SHI2:	CI EV_SEL
	BNZ SHI3
	JMP SHSEL
SHI3:	CI EV_BACK
	BNZ SHI4
	JMP SHREN
SHI4:	CI EV_PGDN
	BNZ SHI5
	JMP SHINFO
SHI5:	CI EV_PGUP
	BNZ SHIN
	JMP SHINFO

SHDOWN:	; move within the viewport, else scroll
	DCI VCUR
	LM
	LR 0,A
	INC
	LR 1,A
	LI NROWS
	LR 2,A
	LR A,1
	COM
	INC
	AS 2
	BM SHDSCR		; the viewport is full: try scrolling instead
	BZ SHDSCR
	; also stop at the last slot
	DCI VHTOP
	LM
	AS 1
	CI 7
	BP SHDOK		; 7 >= top+cur+1: slots are 0..7, so the row exists
	JMP SHDRAW
SHDOK:	LR A,1
	DCI VCUR
	ST
	PI UICUR
	JMP SHIN
SHDSCR:	DCI VHTOP
	LM
	LR 0,A
	AI NROWS
	CI 8
	BP SHDSC1		; 8 >= top+NROWS: there is another slot below
	JMP SHIN
SHDSC1:	LR A,0
	INC
	DCI VHTOP
	ST
	JMP SHDRAW

SHUP:	DCI VCUR
	LM
	CI 0
	BNZ SHUOK
	DCI VHTOP
	LM
	CI 0
	BNZ SHUSCR
	JMP SHIN
SHUSCR:	LM			; DC0 has moved on; re-read
	DCI VHTOP
	LM
	AI 0FFH
	DCI VHTOP
	ST
	JMP SHDRAW
SHUOK:	AI 0FFH
	DCI VCUR
	ST
	PI UICUR
	JMP SHIN

; SHSEL -- mount the selected host and go browsing.
SHSEL:	PI SHSLOT		; r0 = the absolute slot number
	DCI VHOST
	LR A,0
	ST
	PI CMHOST
	CI 0
	BZ SHSELO
	LR 1,A
	LIS 2
	LR 0,A
	PI SETERR
	JMP FAIL
SHSELO:	DCI VPATH
	LI '/'
	ST
	CLR
	ST
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

; SHREN -- rename the selected slot through the on-screen keyboard.
SHREN:	PI SHSLOT
	DCI VHSEL
	LR A,0
	ST
	LI ED_HOST
	LR 0,A
	DCI VEDWHAT
	LR A,0
	ST
	LI 31
	LR 0,A
	DCI VEDMAX
	LR A,0
	ST
	; seed the accumulator with the current name
	PI SHNAME		; DC1 = VHOSTS + slot*32
	DCI VENTRY
	XDC			; DC0 = name, DC1 = VENTRY
	LI 32
	LR 4,A
SHRCP:	LM
	XDC
	ST
	XDC
	DS 4
	BNZ SHRCP
	DCI VENTRY+31
	CLR
	ST
	LI ST_HOST
	LR 0,A
	DCI VPEND
	LR A,0
	ST			; where the editor returns to
	LI ST_EDIT
	LR 0,A
	JMP GOTO

SHINFO:	LI ST_HOST
	LR 0,A
	DCI VPEND
	LR A,0
	ST
	LI ST_INFO
	LR 0,A
	JMP GOTO

; SHSLOT -- r0 = VHTOP + VCUR, the absolute slot under the cursor. Leaf.
SHSLOT:	DCI VHTOP
	LM
	LR 0,A
	DCI VCUR
	LM
	AS 0
	LR 0,A
	POP

; SHNAME -- DC1 = VHOSTS + r0*32, the name of slot r0. Leaf. Clobbers A, r4.
SHNAME:	DCI VHOSTS
	LR A,0
	LR 4,A
	CI 0
	BZ SHNMX
SHNM1:	LI 32
	ADC
	DS 4
	BNZ SHNM1
SHNMX:	XDC
	POP

; SHROWS -- redraw the visible slots. Reached by PI; calls one level down.
SHROWS:	ENTER
	DCI VROW
	CLR
	ST
SHR1:	DCI VROW
	LM
	CI NROWS
	BZ SHRX
	DCI VROW
	LM
	LR 0,A
	PI UICLRR

	; slot = VHTOP + row; stop drawing past slot 7
	DCI VHTOP
	LM
	LR 0,A
	DCI VROW
	LM
	AS 0
	LR 5,A			; r5 = absolute slot
	CI 7
	BM SHRNEXT		; 7 < slot: past slot 7, nothing to draw

	LR A,5
	LR 0,A
	PI SHNAME		; DC1 = the name
	; an empty slot reads as a NUL first byte
	XDC
	LM
	XDC
	CI 0
	BNZ SHRNAME
	LI CVAL1
	LR 3,A
	DCI SHEMPTY
	XDC
	BR SHRDRAW
SHRNAME: LI CVAL3
	LR 3,A
	LR A,5
	LR 0,A
	PI SHNAME
SHRDRAW: DCI VROW
	LM
	LR 0,A
	PI UIROW

SHRNEXT: DCI VROW
	LM
	INC
	DCI VROW
	ST
	BR SHR1
SHRX:	LEAVE

SHTITLE: DB "FUJINET HOSTS",0
SHFOOT:	DB "FIRE:OPEN MODE:NAME",0
SHEMPTY: DB "-- EMPTY --",0
