; ---------------------------------------------------------------------------
; f8call.inc -- nested calls on a CPU that has room for exactly one.
;
; PI copies PC0 into PC1 and POP restores it, so a routine reached by PI has
; exactly one return address and cannot call anything else. The console BIOS
; solves this for itself with a stack of K registers at r40-r58, r59 as the
; pointer, and this is the same mechanism its own clrscrn uses.
;
;   FOO:	ENTER		; K = my return; push it
;		PI BAR		; now free to call one level down
;		LEAVE		; pop K and return through it
;
; ENTER must be the FIRST thing in the routine, before any PI, because PC1 is
; where the return address lives until it is saved.
;
; Depth is 9 (r40-r58, two bytes each). The BIOS sets r59 = 40 during its own
; reset, so this works from the moment a cartridge is entered.
;
; COSTS, both of which have bitten: push_k and pop_k CLOBBER r7 and A. Nothing
; may hold a value in r7 across ENTER or LEAVE, and no argument or result may
; travel in A across a call at all -- PI and JMP both stash the target
; address's high byte there.
; ---------------------------------------------------------------------------

PUSHK	EQU 00107H		; BIOS: push K onto the r40 stack
POPK	EQU 0011EH		; BIOS: pop it back

ENTER	MACRO
	LR K,P
	PI PUSHK
	ENDM

; LEAVE carries the accumulator across POPK by hand. POPK is reached with PI,
; and PI stashes the target address's high byte in A -- so without this every
; routine that returns a status would hand back the high byte of $011E instead.
; pop_k clobbers r7 and A and nothing else, so r10 is safe to borrow.
LEAVE	MACRO
	LR 10,A
	PI POPK
	LR A,10
	PK
	ENDM
