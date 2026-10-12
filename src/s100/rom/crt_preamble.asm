; CONFIG.BIN is entered from the card's boot page with no stack set up:
; this runs first and puts one above BSS.
        EXTERN  __BSS_END_tail
        ld      sp,__BSS_END_tail + 0600h
