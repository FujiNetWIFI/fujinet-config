; ---------------------------------------------------------------------------
; fujicmd.inc -- one wrapper per FujiNet command CONFIG uses.
;
; Each is reached by PI and calls one level down, so each opens with ENTER and
; closes with LEAVE. Every one returns A = 0 on success, or the FN_ERR_* code,
; or $EE for a NAK. Arguments arrive in registers or RAM, never in the
; accumulator: PI and JMP both stash the target address's high byte there.
;
; Payload shapes that are NOT obvious, and were paid for on the sibling ports:
;   - OPEN_DIRECTORY and SET_DEVICE_FULLPATH need a FULL 256-byte NUL-padded
;     payload; the server reads exactly that many and fails a short one.
;   - COPY_FILE is the exception -- exact length, and its host slots are
;     1-BASED where every other command's are 0-based.
;   - SET_SSID is exactly 97 bytes: ssid[33] + password[64].
;   - READ_DIR_ENTRY with maxlen 31 makes the firmware prepend icon bytes; any
;     other length does not. The maxlen is one byte, so FULLLN is 255, the most it
;     can ask for.
;   - Never seek to directory position 0: a freshly opened directory is already
;     there, and dir_seek(0) fails outright on an empty one, which would read
;     as an error rather than as the empty listing it is.
; ---------------------------------------------------------------------------

; READ_DIR_ENTRY width. The firmware cuts a longer name to fit, and adds a
; folder's '/' only when the name is two characters short of the width.
FULLLN	EQU 255

; CMHOST -- MOUNT_HOST(r0 = host slot).
CMHOST:	ENTER
	LR A,0
	LR 4,A
	LI FD_FUJI
	LR 0,A
	LI FC_MHOST
	LR 1,A
	LIS 1
	LR 2,A
	PI FNBEG
	LR A,4
	LR 0,A
	PI FNPB
	PI FNGO
	PI FNACK
	LEAVE

; CRHOSTS -- READ_HOST_SLOTS. The reply is 8 x 32 ASCII bytes.
CRHOSTS: ENTER
	LI FD_FUJI
	LR 0,A
	LI FC_RHOST
	LR 1,A
	CLR
	LR 2,A
	PI FNBEG
	PI FNGO
	PI FNACK
	LEAVE

; CWHOSTS -- WRITE_HOST_SLOTS, from the 256 bytes at DC1.
CWHOSTS: ENTER
	LI FD_FUJI
	LR 0,A
	LI FC_WHOST
	LR 1,A
	CLR
	LR 2,A
	PI FNBEG
	PI FNPATH
	PI FNGO
	PI FNACK
	LEAVE

; CODIR -- OPEN_DIRECTORY(r0 = host) over VPATH, with an empty filter.
;   The payload is path, NUL, then NUL padding -- the server splits the two
;   fields at the first NUL, so all-NUL padding IS an empty filter.
CODIR:	ENTER
	LR A,0
	LR 4,A
	LI FD_FUJI
	LR 0,A
	LI FC_ODIR
	LR 1,A
	LIS 1
	LR 2,A
	PI FNBEG
	LR A,4
	LR 0,A
	PI FNPB
	DCI VPATH
	XDC
	PI FNPATH
	PI FNGO
	PI FNACK
	LEAVE

; CCDIR -- CLOSE_DIRECTORY.
CCDIR:	ENTER
	LI FD_FUJI
	LR 0,A
	LI FC_CDIR
	LR 1,A
	CLR
	LR 2,A
	PI FNBEG
	PI FNGO
	PI FNACK
	LEAVE

; CDPOS -- SET_DIRECTORY_POSITION(r1:r0). A position past the end NAKs, which
;   callers read as end-of-directory.
CDPOS:	ENTER
	LR A,0
	LR 4,A
	LR A,1
	LR 5,A
	LI FD_FUJI
	LR 0,A
	LI FC_SDPOS
	LR 1,A
	LIS 1
	LR 2,A
	PI FNBEG
	LR A,4
	LR 0,A
	LR A,5
	LR 1,A
	PI FNPW
	PI FNGO
	PI FNACK
	LEAVE

; CDENT -- READ_DIR_ENTRY(r0 = maxlen). The name lands in the reply window.
;   Returns $7F when the reply is the two-byte end-of-directory marker.
CDENT:	ENTER
	LR A,0
	LR 4,A
	LI FD_FUJI
	LR 0,A
	LI FC_RDIR
	LR 1,A
	LIS 2
	LR 2,A
	PI FNBEG
	LR A,4
	LR 0,A
	PI FNPB
	CLR
	LR 0,A
	PI FNPB
	PI FNGO
	PI FNACK
	CI 0
	BNZ CDENTX
	PI FNEOF
	BNZ CDENTO
	LI 07FH			; end of directory
	LEAVE
CDENTO:	CLR
CDENTX:	LEAVE

; CSDFP -- SET_DEVICE_FULLPATH(dev 0, r0 = host, read mode) over VFULL.
CSDFP:	ENTER
	LR A,0
	LR 4,A
	LI FD_FUJI
	LR 0,A
	LI FC_SDFP
	LR 1,A
	LIS 3
	LR 2,A
	PI FNBEG
	CLR
	LR 0,A
	PI FNPB			; device slot 0: this console has one thing to mount
	LR A,4
	LR 0,A
	PI FNPB
	LIS 1
	LR 0,A
	PI FNPB			; read mode
	DCI VFULL
	XDC
	PI FNPATH
	PI FNGO
	PI FNACK
	LEAVE

; CMIMG -- MOUNT_IMAGE(dev 0, read mode).
CMIMG:	ENTER
	LI FD_FUJI
	LR 0,A
	LI FC_MIMG
	LR 1,A
	LIS 2
	LR 2,A
	PI FNBEG
	CLR
	LR 0,A
	PI FNPB
	LIS 1
	LR 0,A
	PI FNPB
	PI FNGO
	PI FNACK
	LEAVE

; CADPTX -- GET_ADAPTERCONFIG_EXTENDED. ssid at +0, fn_version at +125,
;   sLocalIP at +140, all as text in the reply window.
CADPTX:	ENTER
	LI FD_FUJI
	LR 0,A
	LI FC_ADPTX
	LR 1,A
	CLR
	LR 2,A
	PI FNBEG
	PI FNGO
	PI FNACK
	LEAVE

; CWIFIS -- GET_WIFISTATUS. Reply byte 0 is 3 when the link is up.
CWIFIS:	ENTER
	LI FD_FUJI
	LR 0,A
	LI FC_WIFIS
	LR 1,A
	CLR
	LR 2,A
	PI FNBEG
	PI FNGO
	PI FNACK
	LEAVE

; CSCAN -- SCAN_NETWORKS. Reply byte 0 is the count found.
CSCAN:	ENTER
	LI FD_FUJI
	LR 0,A
	LI FC_SCAN
	LR 1,A
	CLR
	LR 2,A
	PI FNBEG
	PI FNGO
	PI FNACK
	LEAVE

; CSCANR -- GET_SCAN_RESULT(r0 = index). Reply is ssid[32] then rssi.
CSCANR:	ENTER
	LR A,0
	LR 4,A
	LI FD_FUJI
	LR 0,A
	LI FC_SCANR
	LR 1,A
	LIS 1
	LR 2,A
	PI FNBEG
	LR A,4
	LR 0,A
	PI FNPB
	PI FNGO
	PI FNACK
	LEAVE

; CSSID -- SET_SSID from VSSID and VPASS. Exactly 97 bytes: ssid[33] then
;   password[64], each NUL-padded to its full width.
CSSID:	ENTER
	LI FD_FUJI
	LR 0,A
	LI FC_SSSID
	LR 1,A
	LIS 1
	LR 2,A
	PI FNBEG
	LIS 1
	LR 0,A
	PI FNPB			; nparam >= 1 is required, and this is the flag
	DCI VSSID
	XDC
	LI 33
	LR 0,A
	PI FNFIX
	DCI VPASS
	XDC
	LI 64
	LR 0,A
	PI FNFIX
	PI FNGO
	PI FNACK
	LEAVE

; CCOPY -- COPY_FILE(r0 = source host, r1 = destination host): VSRC to the folder
;   VPATH. The payload is the copySpec "source|folder", the folder ending in '/'
;   so the firmware names the copy after the source. Sent at EXACT length, unlike
;   every other path command here, and the host slots are 1-BASED. Both are the
;   server's rules, not ours.
CCOPY:	ENTER
	LR A,0
	LR 4,A
	LR A,1
	LR 5,A
	LI FD_FUJI
	LR 0,A
	LI FC_COPY
	LR 1,A
	LIS 2
	LR 2,A
	PI FNBEG
	LR A,4
	INC
	LR 0,A
	PI FNPB
	LR A,5
	INC
	LR 0,A
	PI FNPB
	DCI VSRC
	XDC
	PI FNTXS		; exact length, no padding
	PI FNTXP
	LI '|'
	ST
	DCI VPATH
	XDC
	PI FNTXS
	PI FNGO
	PI FNACK
	LEAVE
