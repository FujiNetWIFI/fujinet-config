        .export     _pause
        .export     _wait_scan1

        .include    "atari.inc"
        .include    "cfg-zp.inc"

; void _pause(uint8_t delay)
;
; wait for clock to cycle to given count 0-255
.proc _pause
        sta     tmp6

        ; simple use of RTCLOK+2 to track jiffies
        lda     #$00
        sta     RTCLOK+2

:       lda     RTCLOK+2
        cmp     tmp6
        bne     :-

        jsr     _wait_scan1
        rts
.endproc

.proc _wait_scan1
:       lda VCOUNT
        bne :-
        rts

:       lda VCOUNT
        beq :-
        rts
.endproc