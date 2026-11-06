; ---------------------------------------------------------------------------
; config.asm -- FujiNet CONFIG for the Fairchild Channel F.
;
; Standalone, like ../intv/ and ../coleco/: it mirrors src/'s state machine
; without sharing its code, because the shared code assumes a keyboard, four
; device slots and kilobytes of buffers.
;
; SCOPE. Astrocade parity: WiFi (scan, custom SSID, on-screen keyboard, connect),
; the 8 host slots (list, mount, rename), directory browsing (paging, subfolder
; navigation), copying a file to a folder on another host slot, adapter info,
; and boot.
; Deliberately out of scope, as on every console port: the device-slot screen,
; mount/eject, read/write toggles, new-disk and appkeys -- none of them is
; needed to get from power-on to a booted cartridge, which is this program's
; whole job. DEVICE_SLOT is hardwired to 0; the Channel F has exactly one thing
; to mount into.
;
; STRUCTURE. The F8 keeps one return address, so this is a flat state machine
; that JMPs between states, with calls at most two deep through the BIOS
; K-stack (see f8call.inc). A state is reached by JMP and never returns, so it
; takes no ENTER: nothing would pop the entry, and the nine-deep stack would
; overflow after a handful of screen changes. CONFIG_BOOT (0xD9) is deliberately
; never sent: CONFIG lives in cart flash, and the flag would only unmount
; device slot 0.
;
; RAM. 30K in the cartridge's arena at $8000. Unlike every sibling port there
; is no ledger and no rationing -- the Arcadia client lives in 84 bytes and the
; O2 client in 39, because those cart edges cannot accept a write cycle. This
; one can, so the four strings that dominate their RAM budgets are just
; buffers here.
; ---------------------------------------------------------------------------
	CPU F3850

; --- state ids ---
ST_WIFI	EQU 0			; wait for a link, or offer the picker
ST_SCAN	EQU 1			; network picker
ST_EDIT	EQU 2			; on-screen keyboard
ST_CONN	EQU 3			; connecting
ST_HOST	EQU 4			; the 8 host slots
ST_FILE	EQU 5			; directory browser
ST_INFO	EQU 6			; adapter info
ST_BOOT	EQU 7			; mount and swap

; what the editor is editing, so it knows where to put the result
ED_SSID	EQU 0
ED_PASS	EQU 1
ED_HOST	EQU 2

; --- client RAM, in the arena at $8000 ---
VHEX	EQU 08000H		; 3 bytes: two hex digits and a NUL
VERR	EQU 08004H
VCLASS	EQU 08005H
VSTATE	EQU 08006H
VCUR	EQU 08007H		; cursor row, 0..VNROW-1
VNROW	EQU 08008H		; rows filled on this page
VPOSL	EQU 08009H		; first directory position on this page, LE
VPOSH	EQU 0800AH
VMORE	EQU 0800BH		; the page filled: there may be more
VISDIR	EQU 0800CH
VROW	EQU 0800DH		; loop counter for the drawing loops
VPIXY	EQU 0800EH		; pixel row of the row being drawn
VHOST	EQU 0800FH		; host slot being browsed
VIPCTL	EQU 08010H		; (input.inc)
VIPPAN	EQU 08011H
VEDWHAT	EQU 08012H		; ED_*: what the editor is filling in
VEDX	EQU 08013H		; keyboard cursor
VEDY	EQU 08014H
VEDLEN	EQU 08015H		; characters entered so far
VEDMAX	EQU 08016H		; field width
VEDCASE	EQU 08017H		; 0 lower, 1 upper
VSCANN	EQU 08018H		; networks the scan found
VSCANB	EQU 08019H		; first network shown on this page
VCHOST	EQU 0801AH		; host slot the copy source is on
VPEND	EQU 0801BH		; pending state, for screens that return
VCMODE	EQU 0801EH		; 1 while picking a copy destination
VSRCD	EQU 0801FH		; length of VSRC's folder part, '/' included
VBSEQ	EQU 08036H		; boot: the MOUNT_IMAGE sequence
VBPCT	EQU 08037H		; boot: last percentage drawn
VBTMO	EQU 08038H		; boot: polls left before giving up
VBCEL	EQU 08039H		; boot: filled bar cells
VPOLLN	EQU 08032H		; link-poll countdown (r9 belongs to FNGO)
VPOLLS	EQU 08035H		; input samples left in this poll
VEDDR	EQU 08033H		; keyboard row being drawn
VEDDC	EQU 08034H		; keyboard column being drawn
VDISP	EQU 08040H		; a name or path cut to the screen, NAMEW + NUL
VPATH	EQU 08100H		; current path, NUL-terminated, at most PATHMAX
VNAME	EQU 08200H		; the selected entry at FULLLN: up to 255 with its NUL
VSSID	EQU 08300H		; 33
VPASS	EQU 08340H		; 64
VENTRY	EQU 08380H		; editor accumulator, at most 64
VFULL	EQU 08400H		; VPATH + VNAME, the boot path
VSRC	EQU 08500H		; copy source path
VHOSTS	EQU 08600H		; 8 x 32, the host slot table
STUB	EQU 08800H		; the swap stub

; The longest folder path, or folder plus file, CONFIG will build. COPY_FILE
; carries "source|folder" in the mailbox's 320-byte TX buffer after four bytes
; of parameters, so two paths and the bar must fit in 316.
PATHMAX	EQU 150

	ORG 0800H
	DB 55H			; cart signature -- the BIOS compares this at $0800
	DB 00H			; $0801 is skipped; the BIOS jumps to $0802

ENTRY:	DI

	; The BIOS has already zeroed all 64 scratchpad registers and set r59
	; (the K-stack pointer) to 40, so nested calls work from here.

	LI CVAL0
	LR 3,A
	PI BCLRSCR
	LI PAL2A
	LR 3,A
	LI PAL2B
	LR 4,A
	PI DPAL

	; path = "/"
	DCI VPATH
	LI '/'
	ST
	CLR
	ST

	DCI VHOST
	CLR
	ST
	DCI VSTATE
	CLR
	ST			; ST_WIFI

	PI FNCHK
	BZ MAIN

	LI CVAL2
	LR 3,A
	LI DORGX
	LR 1,A
	LI DORGY+3*DCELLH
	LR 2,A
	DCI SNOCART
	PI DSTR
HLTNC:	BR HLTNC

; ---------------------------------------------------------------------------
; MAIN -- dispatch on VSTATE. Each state runs to completion, sets VSTATE, and
; JMPs back here; none of them returns, so nothing accumulates on the K stack.
; ---------------------------------------------------------------------------
MAIN:	DCI VSTATE
	LM
	CI ST_WIFI
	BNZ MD1
	JMP SWIFI
MD1:	CI ST_SCAN
	BNZ MD2
	JMP SSCAN
MD2:	CI ST_EDIT
	BNZ MD3
	JMP SEDIT
MD3:	CI ST_CONN
	BNZ MD4
	JMP SCONN
MD4:	CI ST_HOST
	BNZ MD5
	JMP SHOST
MD5:	CI ST_FILE
	BNZ MD6
	JMP SFILE
MD6:	CI ST_INFO
	BNZ MD7
	JMP SINFO
MD7:	CI ST_BOOT
	BNZ MAIN
	JMP SBOOT

; ---------------------------------------------------------------------------
; GOTO -- set VSTATE from r0 and re-dispatch. Reached by JMP, never called.
; ---------------------------------------------------------------------------
GOTO:	DCI VSTATE
	LR A,0
	ST
	JMP MAIN

; ---------------------------------------------------------------------------
; FAIL -- show a class and a code on the footer, then stop. Reached by JMP,
; with the class already in VCLASS and the code in VERR.
; ---------------------------------------------------------------------------
FAIL:	LI CVAL2
	LR 3,A
	DCI SERR
	XDC
	PI UIFOOT
	DCI VCLASS
	LM
	LR 0,A
	DCI VHEX
	PI DHEXS
	DCI VHEX
	PI DSTR
	DCI VERR
	LM
	LR 0,A
	DCI VHEX
	PI DHEXS
	DCI VHEX
	PI DSTR
HLTF:	BR HLTF

; SETERR -- park r0 as the class and r1 as the code, then the caller JMPs FAIL.
;   Neither travels in the accumulator: the PI that reaches this routine
;   clobbers A before the first instruction runs. Splitting the park from the
;   jump keeps FAIL reachable from anywhere without a K-stack entry.
SETERR:	DCI VERR
	LR A,1
	ST
	DCI VCLASS
	LR A,0
	ST
	POP

	INCLUDE "f8call.inc"
	INCLUDE "fujidisp.inc"
	INCLUDE "ui.inc"
	INCLUDE "fujilib.inc"
	INCLUDE "fujicmd.inc"
	INCLUDE "input.inc"
	INCLUDE "wifi.inc"
	INCLUDE "edit.inc"
	INCLUDE "hosts.inc"
	INCLUDE "browse.inc"
	INCLUDE "copy.inc"
	INCLUDE "info.inc"
	INCLUDE "boot.inc"
	INCLUDE "font.inc"

SNOCART: DB "NO FUJINET CART",0
SERR:	DB "ERR ",0

	ORG 0800H+FN_ROM_CLAIM
	DB "FUJI"

	END
