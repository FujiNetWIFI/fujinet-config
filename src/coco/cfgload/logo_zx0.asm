; ZX0 decompressor for M6809, used to unpack the splash logo bitmap.
; Adapted from https://github.com/dougmasten/zx0-6x09 (zx0_6809_standard.asm)
;
; Copyright (c) 2021 Doug Masten
; ZX0 compression (c) 2021 Einar Saukas, https://github.com/einar-saukas/ZX0
;
; This software is provided 'as-is', without any express or implied
; warranty. In no event will the authors be held liable for any damages
; arising from the use of this software.
;
; Permission is granted to anyone to use this software for any purpose,
; including commercial applications, and to alter it and redistribute it
; freely, subject to the following restrictions:
;
; 1. The origin of this software must not be misrepresented; you must not
;    claim that you wrote the original software. If you use this software
;    in a product, an acknowledgment in the product documentation would be
;    appreciated but is not required.
; 2. Altered source versions must be plainly marked as such, and must not be
;    misrepresented as being the original software.
; 3. This notice may not be removed or altered from any source distribution.

	SECTION code

_zx0_decompress		EXPORT

;------------------------------------------------------------------------------
; Function    : zx0_decompress
; Entry       : Reg X = start of compressed data
;             : Reg U = start of decompression buffer
; Exit        : Reg X = end of compressed data + 1
;             : Reg U = end of decompression buffer + 1
; Destroys    : Regs D, Y
; Description : Decompress ZX0 data (version 1)
;------------------------------------------------------------------------------
; One-time-use build: skips re-initializing zx0_bit/zx0_offset on entry,
; since this program only ever decompresses the logo once before jumping
; to the downloaded payload.
ZX0_ONE_TIME_USE	equ	1

_zx0_decompress
			lda	#$80
			sta	zx0_bit		; init bit stream

; 0 - literal (copy next N bytes from compressed data)
zx0_literals		bsr	zx0_elias	; obtain length
			tfr	d,y		;  "      "
			bsr	zx0_copy_bytes	; copy literals
			bcs	zx0_new_offset	; branch if next block is new-offset

; 0 - copy from last offset (repeat N bytes from last offset)
			bsr	zx0_elias	; obtain length
zx0_copy		equ	*
			pshs	x		; save reg X
			tfr	d,y		; setup length
zx0_offset		equ	*+2
			leax	>$ffff,u	; calculate offset address
			bsr	zx0_copy_bytes	; copy match
			puls	x		; restore reg X
			bcc	zx0_literals	; branch if next block is literals

; 1 - copy from new offset (repeat N bytes from new offset)
zx0_new_offset		bsr	zx0_elias	; obtain offset MSB
			negb			; adjust for negative offset (set carry for RORA below)
			beq	zx0_eof		; eof? (length = 256) if so exit
			tfr	b,a		; transfer to MSB position
			ldb	,x+		; obtain LSB offset
			rora			; last offset bit becomes first length bit
			rorb			;  "     "     "    "      "     "      "
			std	zx0_offset	; preserve new offset
			ldd	#1		; set elias = 1
			bsr	zx0_elias_bt	; get length but skip first bit
			addd	#1		; length = length + 1
			bra	zx0_copy	; copy new offset match


; interlaced elias gamma coding
zx0_elias		ldd	#1		; set elias = 1
			bra	zx0_elias_start	; goto start of elias gamma coding

zx0_elias_loop		lsl	zx0_bit		; get next bit
			rolb			; rotate elias value
			rola			;   "     "     "
zx0_elias_start		lsl	zx0_bit		; get next bit
			bne	zx0_elias_bt	; branch if bit stream is not empty
			pshs	a		; save reg A
			lda	,x+		; load another 8-bits
			rola			; get next bit
			sta	zx0_bit		; save bit stream
			puls	a		; restore reg A
zx0_elias_bt		bcc	zx0_elias_loop	; loop until done
zx0_eof			rts			; return


; copy Y bytes from X to U and get next bit
zx0_copy_bytes		ldb	,x+		; copy byte
			stb	,u+		;  "    "
			leay	-1,y		; decrement loop counter
			bne	zx0_copy_bytes	; loop until done
			lsl	zx0_bit		; get next bit
			rts

zx0_bit			fcb	$80

	ENDSECTION
