; input.inc -- the stick, the keyboard, the frame clock. Main program,
; P1 = P1_MAIN.

; joy_poll: the left stick into JOY, what was pressed since last time into
; JOYEV. The keyboard scanner shares P2 with the program address, so only the
; BIOS's own code can read it ($38F getjoystick: R1 = stick; R3 up/down,
; R2 left/right as $FF/$01, F0 the button). The interrupts save and restore
; P1, so the stick's P1 survives them. Clobbers A, r0-r4, F0.
joy_poll:
    mov a,#P1_JOY
    outl p1,a
    mov r1,#0
    FCALL getjoystick
    mov a,#P1_MAIN
    outl p1,a
    clr a
    mov r4,a
    jf0 jp_fire
    jmp jp_v
jp_fire:
    mov r4,#J_FIRE
jp_v:
    mov a,r3
    jz jp_h
    jb7 jp_up
    mov a,r4
    orl a,#J_DOWN
    mov r4,a
    jmp jp_h
jp_up:
    mov a,r4
    orl a,#J_UP
    mov r4,a
jp_h:
    mov a,r2
    jz jp_done
    jb7 jp_left
    mov a,r4
    orl a,#J_RIGHT
    mov r4,a
    jmp jp_done
jp_left:
    mov a,r4
    orl a,#J_LEFT
    mov r4,a
jp_done:
    mov r0,#JOYP                ; JOYEV = JOY & ~JOYP; JOYP = JOY
    movx a,@r0
    cpl a
    anl a,r4
    mov r0,#JOYEV
    movx @r0,a
    mov a,r4
    mov r0,#JOYP
    movx @r0,a
    mov r0,#JOY
    movx @r0,a
    ret

; key_poll: KEY = a key pressed since the last call (K_NONE: none). The
; BIOS scans the matrix (readkey: A = a new key 0-47, $C0 + key while it is
; held, $FF none). It keeps the last key in bank 0's r7 and comes back in
; bank 1. Call it in the quiet stretch (frame_sync): it holds the vblank off
; for up to ~400 cycles while it debounces, and runs in register bank 1,
; which a band interrupt would trample. Clobbers A, r0-r7.
key_poll:
    mov r0,#KLAST
    movx a,@r0
    mov r7,a
    FCALL readkey
    sel rb0
    mov r6,a
    mov a,r7
    mov r0,#KLAST
    movx @r0,a
    mov a,r6
    jb7 kp_none
    jb6 kp_none
    jmp kp_put
kp_none:
    mov a,#K_NONE
kp_put:
    mov r0,#KEY
    movx @r0,a
    ret

; frame_wait: until the next vblank. Clobbers A, r1, r2.
frame_wait:
    mov r1,#FRAMES
    mov a,@r1
    mov r2,a
fw_l:
    mov a,@r1
    xrl a,r2
    jz fw_l
    ret

; frame_sync: until the frame's last band has run -- the stretch with no
; band due before the next frame's, where the keyboard scan belongs.
; Clobbers A, r1, r2.
frame_sync:
    mov r1,#QUIET
    mov a,@r1
    mov r2,a
fs_l:
    mov a,@r1
    xrl a,r2
    jz fs_l
    ret

; in_poll: one frame's input, taken after its last band, and the sound
; queued (snd.inc): KEY (K_NONE: none)
; and JOYEV, the stick's directions repeating while held (REP_FIRST frames,
; then every REP_NEXT). Clobbers A, r0-r7, F0.
REP_FIRST equ 18
REP_NEXT  equ 4
in_poll:
    call frame_sync
    call snd_go
    call key_poll
    call joy_poll
    mov r0,#JOY
    movx a,@r0
    anl a,#J_UP|J_DOWN|J_LEFT|J_RIGHT
    jz ip_x                     ; nothing held
    mov r5,a
    mov r0,#JOYEV
    movx a,@r0
    anl a,#J_UP|J_DOWN|J_LEFT|J_RIGHT
    jnz ip_arm                  ; just pressed: the delay, then repeats
    mov r0,#REP
    movx a,@r0
    dec a
    movx @r0,a
    jnz ip_x
    mov a,#REP_NEXT
    movx @r0,a
    mov r0,#JOYEV
    movx a,@r0
    orl a,r5
    movx @r0,a
ip_x:
    ret
ip_arm:
    mov r0,#REP
    mov a,#REP_FIRST
    movx @r0,a
    ret
