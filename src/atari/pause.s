        .export     _pause
        .export     _wait_scan1
        .import     _fuji_wait_jiffies

        .include    "atari.inc"

; void _pause(uint8_t delay)
.proc _pause
        ldx     #$00
        jsr     _fuji_wait_jiffies
        jmp     _wait_scan1
.endproc

.proc _wait_scan1
:       lda VCOUNT
        bne :-
        rts

:       lda VCOUNT
        beq :-
        rts
.endproc