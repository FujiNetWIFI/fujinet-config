	EXTERN	start
	EXTERN  callstmt
	SNSMAT	equ 0x0141		; BIOS routine to read a keyboard row
                      		; Input: A = Row number
                    		; Output: A = Bit pattern (0 = pressed)
; ROM header
	defm	"AB"
	defw	init
	defw	callstmt                ;CallSTMT handler
	defw	0                       ;Device handler
	defw	0                       ;basic
	defs	6

init:
	ld a, 6           ; Select Row 6 (contains Shift, Ctrl, Graph, etc.)
    call SNSMAT       ; Call BIOS
    bit 0, a          ; Check Bit 0 (the Shift key)
    jr z, skip_rom    ; If bit is 0 (Z flag set), key is pressed -> skip
	call	start
skip_rom:
	ret