        .export     _cfg_debug

; This allows us to set a BreakPoint anywhere in code for Altirra to stop on.
; simply call this function. The breakpoint is configured in the Altirra secton of Makefile.atari
.proc _cfg_debug
        rts
.endproc