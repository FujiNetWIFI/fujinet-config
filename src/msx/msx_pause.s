    PUBLIC _pause
    EXTERN _system_fps

; void pause(uint8_t seconds)
; ticks are 60fps in Japan/Korea/USA/Brazil and 50fps in Europe
_pause:
    ld   a, l
    or   a
    ret  z                         ; no delay

    ld   a, (_system_fps)          ; a <- stored fps value
sec_loop:
    ld   b, a                      ; b <- a

halt_loop:
    halt                           ; wait 1/system_fps second
    djnz halt_loop                 ; system_fps times
    dec  l                         ; one second
    jr   nz, sec_loop
    ret
