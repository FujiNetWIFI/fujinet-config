; beep.inc -- beep: queue sound A (SND_*) for the main loop (snd.inc snd_go),
; which plays it from its table; the vblank counts it down and stops it.
; SNDCNT: bit 7 a sound queued (its id below), else the frames it has left.
; The soft ones (SND_SOFT on) never cut another sound. Main program.
; Clobbers A, r0, r1.
beep:
    mov r0,a
    mov r1,#SNDCNT
    add a,#256-SND_SOFT
    jnc bp_q
    mov a,@r1
    jnz bp_x
bp_q:
    mov a,r0
    orl a,#80h
    mov @r1,a
bp_x:
    ret
