; ---------------------------------------------------------------------------
; wifi.inc -- ST_WIFI, ST_SCAN and ST_CONN.
;
; GET_WIFISTATUS reply byte 0 is 3 when the link is up. If it already is, this
; screen steps straight through to the host slots -- the common case is a cart
; that was configured once and never touched again.
; ---------------------------------------------------------------------------

SWIFI:
	LI CVAL3
	LR 3,A
	DCI SWTITLE
	XDC
	PI UITITLE
	PI UICLRL
	LI CVAL1
	LR 3,A
	DCI SWCHECK
	XDC
	PI UIFOOT

	PI CWIFIS
	CI 0
	BNZ SWOFF
	DCI FNREPLY
	LM
	CI 3
	BNZ SWOFF
	LI ST_HOST
	LR 0,A
	JMP GOTO

SWOFF:	LI CVAL2
	LR 3,A
	CLR
	LR 0,A
	DCI SWNOLINK
	XDC
	PI UIROW
	LI CVAL1
	LR 3,A
	DCI SWFOOT
	XDC
	PI UIFOOT

SWIN:	PI INSCAN
	LR A,0
	CI EV_NONE
	BZ SWIN
	CI EV_SEL
	BNZ SWIN1
	LI ST_SCAN
	LR 0,A
	JMP GOTO
SWIN1:	CI EV_BACK
	BNZ SWIN
	LI ST_HOST
	LR 0,A
	JMP GOTO		; carry on offline; the host list still works

; ---------------------------------------------------------------------------
; SSCAN -- pick a network. The list scrolls, and the last row is always
; "OTHER (ENTER SSID)" for a hidden network.
; ---------------------------------------------------------------------------
SSCAN:
	LI CVAL3
	LR 3,A
	DCI SSTITLE
	XDC
	PI UITITLE
	PI UICLRL
	LI CVAL1
	LR 3,A
	DCI SSSCANG
	XDC
	PI UIFOOT

	PI CSCAN
	CI 0
	BZ SSOK
	LR 1,A
	LIS 11
	LR 0,A
	PI SETERR
	JMP FAIL
SSOK:	DCI FNREPLY
	LM
	DCI VSCANN
	ST
	DCI VSCANB
	CLR
	ST
	DCI VCUR
	CLR
	ST
	LI CVAL1
	LR 3,A
	DCI SSFOOT
	XDC
	PI UIFOOT

SSDRAW:	PI SSROWS
	PI UICUR

SSIN:	PI INSCAN
	LR A,0
	CI EV_NONE
	BZ SSIN
	CI EV_DOWN
	BNZ SSI1
	JMP SSDOWN
SSI1:	CI EV_UP
	BNZ SSI2
	JMP SSUP
SSI2:	CI EV_BACK
	BNZ SSI3
	LI ST_WIFI
	LR 0,A
	JMP GOTO
SSI3:	CI EV_SEL
	BNZ SSIN
	JMP SSSEL

; The list is VSCANN networks plus one "OTHER" row.
SSDOWN:	DCI VCUR
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
	BM SSDSCR
	BZ SSDSCR
	PI SSTOTAL		; r0 = VSCANN + 1
	DCI VSCANB
	LM
	AS 1
	LR 2,A
	LR A,0
	COM
	INC
	AS 2			; (base+cur+1) - total
	BM SSDOK
	JMP SSIN
SSDOK:	LR A,1
	DCI VCUR
	ST
	PI UICUR
	JMP SSIN
SSDSCR:	PI SSTOTAL
	LR 2,A
	DCI VSCANB
	LM
	AI NROWS
	LR 1,A
	LR A,2
	COM
	INC
	AS 1			; (base+NROWS) - total
	BM SSDSC1
	JMP SSIN
SSDSC1:	DCI VSCANB
	LM
	INC
	DCI VSCANB
	ST
	JMP SSDRAW

SSUP:	DCI VCUR
	LM
	CI 0
	BNZ SSUOK
	DCI VSCANB
	LM
	CI 0
	BNZ SSUSCR
	JMP SSIN
SSUSCR:	DCI VSCANB
	LM
	AI 0FFH
	DCI VSCANB
	ST
	JMP SSDRAW
SSUOK:	AI 0FFH
	DCI VCUR
	ST
	PI UICUR
	JMP SSIN

SSSEL:	DCI VSCANB
	LM
	LR 0,A
	DCI VCUR
	LM
	AS 0
	LR 4,A			; r4 = absolute index
	DCI VSCANN
	LM
	COM
	INC
	AS 4			; index - count; negate the COUNT, not the index
	BM SSPICK		; a real network
	; the OTHER row: type the SSID by hand
	DCI VENTRY
	CLR
	ST
	LI ED_SSID
	LR 0,A
	DCI VEDWHAT
	LR A,0
	ST
	LI 32
	LR 0,A
	DCI VEDMAX
	LR A,0
	ST
	LI ST_EDIT
	LR 0,A
	JMP GOTO

SSPICK:	LR A,4
	LR 0,A
	PI CSCANR
	CI 0
	BZ SSPOK
	LR 1,A
	LIS 12
	LR 0,A
	PI SETERR
	JMP FAIL
SSPOK:	; the reply is ssid[32] then rssi; keep the ssid
	DCI FNREPLY
	XDC
	DCI VSSID
	XDC
	LI 32
	LR 4,A
SSPCP:	LM
	XDC
	ST
	XDC
	DS 4
	BNZ SSPCP
	DCI VSSID+32
	CLR
	ST
	; now the password
	DCI VENTRY
	CLR
	ST
	LI ED_PASS
	LR 0,A
	DCI VEDWHAT
	LR A,0
	ST
	LI 63
	LR 0,A
	DCI VEDMAX
	LR A,0
	ST
	LI ST_EDIT
	LR 0,A
	JMP GOTO

; SSTOTAL -- r0 = VSCANN + 1, the OTHER row included. Leaf.
SSTOTAL: DCI VSCANN
	LM
	INC
	LR 0,A
	POP

; SSROWS -- redraw the visible networks. Reached by PI; calls one level down.
SSROWS:	ENTER
	DCI VROW
	CLR
	ST
SSR1:	DCI VROW
	LM
	CI NROWS
	BZ SSRX
	DCI VROW
	LM
	LR 0,A
	PI UICLRR

	DCI VSCANB
	LM
	LR 0,A
	DCI VROW
	LM
	AS 0
	LR 5,A			; absolute index
	DCI VSCANN
	LM
	COM
	INC
	AS 5			; index - count; negate the COUNT, not the index
	BM SSRNET
	BZ SSROTHER
	BR SSRNEXT		; past the OTHER row: nothing here

SSROTHER: LI CVAL1
	LR 3,A
	DCI VROW
	LM
	LR 0,A
	DCI SSOTHER
	XDC
	PI UIROW
	BR SSRNEXT

SSRNET:	LR A,5
	LR 0,A
	PI CSCANR
	CI 0
	BNZ SSRNEXT
	LI CVAL3
	LR 3,A
	DCI VROW
	LM
	LR 0,A
	DCI FNREPLY
	XDC
	PI UIROW

SSRNEXT: DCI VROW
	LM
	INC
	DCI VROW
	ST
	BR SSR1
SSRX:	LEAVE

; ---------------------------------------------------------------------------
; SCONN -- send the credentials and wait for the link.
; ---------------------------------------------------------------------------
SCONN:
	LI CVAL3
	LR 3,A
	DCI SNTITLE
	XDC
	PI UITITLE
	PI UICLRL
	LI CVAL2
	LR 3,A
	DCI SNWAIT
	XDC
	PI UIFOOT

	PI CSSID
	CI 0
	BZ SNPOLL
	LR 1,A
	LI 13
	LR 0,A
	PI SETERR
	JMP FAIL

	; Poll until the link comes up, bounded: 30 tries with a delay between
	; them is comfortably longer than an association takes. The delay is cut
	; into short slices with an input scan after each, so a brief press still
	; cancels.
SNPOLL:	LI 30
	DCI VPOLLN
	ST			; RAM, not r9: FNGO stores the sequence there
SNP1:	LI 50
	DCI VPOLLS
	ST
SNPS:	LIS 4
	LR 3,A
	PI DELAYMS
	PI INSCAN
	LR A,0
	CI EV_NONE
	BZ SNPS1
	LI ST_WIFI
	LR 0,A
	JMP GOTO		; any input cancels
SNPS1:	DCI VPOLLS
	LM
	AI 0FFH
	DCI VPOLLS
	ST
	CI 0
	BNZ SNPS
	PI CWIFIS
	CI 0
	BNZ SNP2
	DCI FNREPLY
	LM
	CI 3
	BNZ SNP2
	LI ST_HOST
	LR 0,A
	JMP GOTO
SNP2:	DCI VPOLLN
	LM
	AI 0FFH
	DCI VPOLLN
	ST
	CI 0
	BNZ SNP1

	LI CVAL2
	LR 3,A
	CLR
	LR 0,A
	DCI SNFAIL
	XDC
	PI UIROW
	LI CVAL1
	LR 3,A
	DCI SNFOOT
	XDC
	PI UIFOOT
SNIN:	PI INSCAN
	LR A,0
	CI EV_NONE
	BZ SNIN
	LI ST_WIFI
	LR 0,A
	JMP GOTO

SWTITLE: DB "FUJINET CONFIG",0
SWCHECK: DB "CHECKING LINK...",0
SWNOLINK: DB "NO NETWORK LINK",0
SWFOOT:	DB "FIRE:SETUP MODE:SKIP",0
SSTITLE: DB "SELECT NETWORK",0
SSSCANG: DB "SCANNING...",0
SSFOOT:	DB "FIRE:PICK MODE:BACK",0
SSOTHER: DB "OTHER (ENTER SSID)",0
SNTITLE: DB "CONNECTING",0
SNWAIT:	DB "ANY INPUT CANCELS",0
SNFAIL:	DB "COULD NOT CONNECT",0
SNFOOT:	DB "ANY KEY: BACK",0
