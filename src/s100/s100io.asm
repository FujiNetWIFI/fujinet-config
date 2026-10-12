; Port I/O at a run-time port number, 8080 only: the 8080 has no IN r,(C),
; so each call patches its port into the IN/OUT below.

        SECTION code_user

        PUBLIC  _s100_in
        PUBLIC  _s100_out

; uint8_t s100_in(uint8_t port) __z88dk_fastcall
_s100_in:
        ld      a,l
        ld      (inp+1),a
inp:    in      a,(0FFh)
        ld      l,a
        ld      h,0
        ret

; void s100_out(uint8_t port, uint8_t v): sccz80 pushes left to right
_s100_out:
        ld      hl,4
        add     hl,sp
        ld      a,(hl)
        ld      (outp+1),a
        dec     hl
        dec     hl
        ld      a,(hl)
outp:   out     (0FFh),a
        ret
