; ---------------------------------------------------------------------------
; ui.inc -- the screen furniture every state shares.
;
; 23 columns x 9 rows over the ~95x58 safe area: a title row, seven list rows,
; and a footer. Column 0 of each list row is a cursor gutter, so a name gets 22.
;
; Redrawing the gutter on its own is what makes moving the cursor free: the row
; text is never refetched, so a keypress costs no network round trip. The
; ColecoVision inverts the selected row instead, which needs VRAM readback --
; and this console cannot read VRAM at all.
;
; DCLRR writes r0, r2 and r4-r8 but leaves r1 and r3 alone, so the colour in r3
; survives a clear and does not have to be parked anywhere.
; ---------------------------------------------------------------------------

NROWS	EQU 7			; list rows on screen
NAMELN	EQU 21			; usable width after the gutter
UICOLS	EQU 23

; UIROWY -- r2 = the pixel row for list row r0 (0..NROWS-1).
;   Screen row is list row + 1, because row 0 is the title.
;   Leaf. Clobbers A, r2, r5, r6.
UIROWY:	LR A,0
	INC
	LR 6,A
	LI DCELLH
	LR 5,A
	CLR
	LR 2,A
UIRY1:	LR A,2
	AS 5
	LR 2,A
	DS 6
	BNZ UIRY1
	LR A,2
	AI DORGY
	LR 2,A
	POP

; UICLRR -- clear list row r0 across the full width.
;   Tail-jumps into DCLRR, whose POP returns to OUR caller: JMP leaves PC1
;   alone, and DCLRR reloads the accumulator JMP clobbers on its first
;   instruction.
;   Clobbers A, r0, r1, r2, r4-r8.
UICLRR:	LR A,0
	INC
	LR 6,A
	LI DCELLH
	LR 5,A
	CLR
	LR 2,A
UICR1:	LR A,2
	AS 5
	LR 2,A
	DS 6
	BNZ UICR1
	LR A,2
	AI DORGY
	LR 2,A
	LI UICOLS
	LR 0,A
	LI DORGX
	LR 1,A
	JMP DCLRR

; UIROW -- draw the string at DC1 on list row r0 in colour r3, one cell in from
;   the gutter. Does NOT clear first; callers that need it call UICLRR.
;
;   The row arithmetic is INLINED rather than calling UIROWY, so this stays one
;   level deep: it is reached by PI, and a PI of its own would overwrite the
;   PC1 holding its return address. The closing JMP into DSTR is a tail call --
;   JMP leaves PC1 alone, so DSTR's POP returns to OUR caller.
;   Clobbers A, r0, r1, r2, r4-r8, DC0, DC1.
UIROW:	LR A,0
	INC
	LR 6,A
	LI DCELLH
	LR 5,A
	CLR
	LR 2,A
UIRW1:	LR A,2
	AS 5
	LR 2,A
	DS 6
	BNZ UIRW1
	LR A,2
	AI DORGY
	LR 2,A
	LI DORGX+DCELLW
	LR 1,A
	XDC
	JMP DSTR

; UITITLE / UIFOOT -- clear a fixed row and draw the string at DC1 in r3.
;   These genuinely need two calls (clear, then draw), so they save PC1 through
;   the K stack. Everything else here is arranged to avoid that.
;   Clobbers A, r0, r1, r2, r4-r8, DC0, DC1.
UITITLE: ENTER
	LI UICOLS
	LR 0,A
	LI DORGX
	LR 1,A
	LI DORGY
	LR 2,A
	PI DCLRR
	LI DORGX
	LR 1,A
	LI DORGY
	LR 2,A
	XDC
	PI DSTR
	LEAVE

UIFOOT:	ENTER
	LI UICOLS
	LR 0,A
	LI DORGX
	LR 1,A
	LI DORGY+8*DCELLH
	LR 2,A
	PI DCLRR
	LI DORGX
	LR 1,A
	LI DORGY+8*DCELLH
	LR 2,A
	XDC
	PI DSTR
	LEAVE

; UICUR -- repaint the cursor gutter: a chevron on row VCUR, blank elsewhere.
;   The loop counter and the pixel row live in RAM because DCLRR eats r0, r2
;   and r4-r8 -- an early version kept the counter in r7 and never terminated.
;   Reached by PI; calls one level down, so it needs ENTER/LEAVE.
UICUR:	ENTER
	DCI VROW
	CLR
	ST
UICUR1:	DCI VROW
	LM
	CI NROWS
	BZ UICURX

	DCI VROW
	LM
	LR 0,A
	PI UIROWY
	LR A,2
	DCI VPIXY
	ST

	LIS 1
	LR 0,A
	LI DORGX
	LR 1,A
	DCI VPIXY
	LM
	LR 2,A
	PI DCLRR

	DCI VCUR
	LM
	LR 0,A
	DCI VROW
	LM
	XS 0
	BNZ UICURN
	LI CVAL2
	LR 3,A
	LI DORGX
	LR 1,A
	DCI VPIXY
	LM
	LR 2,A
	DCI UICSTR
	PI DSTR
UICURN:	DCI VROW
	LM
	INC
	DCI VROW
	ST
	BR UICUR1
UICURX:	LEAVE

UICSTR:	DB ">",0

; UICLRL -- blank every list row. Reached by PI; calls one level down.
UICLRL:	ENTER
	DCI VROW
	CLR
	ST
UICLL1:	DCI VROW
	LM
	CI NROWS
	BZ UICLLX
	DCI VROW
	LM
	LR 0,A
	PI UICLRR
	DCI VROW
	LM
	INC
	DCI VROW
	ST
	BR UICLL1
UICLLX:	LEAVE
