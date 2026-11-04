; input.inc -- controller 1, read straight from its ports.
;
; The left button picks (the Master System's button 1), the right button
; goes back (button 2), and * opens a screen's COMMAND menu (Pause). The
; keypad's digits and # are each screen's shortcuts. The VDP frame flag is
; polled, never taken as an interrupt; each new frame ticks the sound and
; the cursor blink and samples the controller.
;
; in_read, in_frames and in_wait preserve BC, DE and HL.

EV_NONE         EQU     0
EV_UP           EQU     1
EV_DOWN         EQU     2
EV_LEFT         EQU     3
EV_RIGHT        EQU     4
EV_FIRE         EQU     5               ; left button
EV_KEY0         EQU     10H             ; keypad 0-9: 10H-19H
EV_BACK         EQU     1AH             ; right button
EV_HASH         EQU     1BH
EV_MENU         EQU     1CH             ; *

PAD_UP          EQU     01H
PAD_DOWN        EQU     02H
PAD_LEFT        EQU     04H
PAD_RIGHT       EQU     08H
PAD_FIRE        EQU     10H
PAD_BACK        EQU     20H
KEY_NONE        EQU     0FFH
KEY_STAR        EQU     10
KEY_HASH        EQU     11

REPEAT_DELAY    EQU     24
REPEAT_EVERY    EQU     5

CTL_KEYPAD      EQU     80H             ; out: select keypad + right button
CTL_JOY         EQU     0C0H            ; out: select stick + left button
CTL_PORT1       EQU     0FCH            ; in: controller 1

; The keypad's 4-bit code (none = 0FH) to the key: 0-9, KEY_STAR, KEY_HASH.
key_decode:
        DB      KEY_NONE,8,4,5,KEY_NONE,7,KEY_HASH,2
        DB      KEY_NONE,KEY_STAR,0,9,3,1,6,KEY_NONE

; read_pad: B = pad bits, C = key. Clobbers A, HL.
read_pad:
        OUT     (CTL_JOY),A
        EX      (SP),HL                 ; let the lines settle
        EX      (SP),HL
        IN      A,(CTL_PORT1)
        CPL                             ; active low
        LD      C,A
        LD      B,0
        BIT     0,C
        JR      Z,rp_dn
        SET     0,B
rp_dn:  BIT     2,C
        JR      Z,rp_lf
        SET     1,B
rp_lf:  BIT     3,C
        JR      Z,rp_rt
        SET     2,B
rp_rt:  BIT     1,C
        JR      Z,rp_fi
        SET     3,B
rp_fi:  BIT     6,C
        JR      Z,rp_key
        SET     4,B
rp_key: OUT     (CTL_KEYPAD),A
        EX      (SP),HL
        EX      (SP),HL
        IN      A,(CTL_PORT1)
        BIT     6,A
        JR      NZ,rp_code
        SET     5,B
rp_code:
        AND     0FH
        LD      HL,key_decode
        ADD     A,L
        LD      L,A
        JR      NC,rp_look
        INC     H
rp_look:
        LD      C,(HL)
        RET

; sample: new presses into in_pend and in_pkey. A reading counts once it
; has been seen two frames running, which rides out switch bounce.
sample:
        CALL    read_pad
        LD      A,(in_cpad)
        CP      B
        JR      NZ,sm_new
        LD      A,(in_ckey)
        CP      C
        JR      Z,sm_take
sm_new: LD      A,B
        LD      (in_cpad),A
        LD      A,C
        LD      (in_ckey),A
        RET
sm_take:
        LD      A,(in_pad)              ; pressed = now and not before
        CPL
        AND     B
        JR      Z,sm_pad
        LD      HL,in_pend
        OR      (HL)
        LD      (HL),A
        LD      A,(frames)
        LD      (in_held),A
sm_pad: LD      A,B
        LD      (in_pad),A
        LD      A,(in_key)
        CP      C
        RET     Z
        LD      A,C
        LD      (in_key),A
        CP      KEY_NONE
        RET     Z
        LD      B,EV_MENU
        CP      KEY_STAR
        JR      Z,sm_event
        LD      B,EV_HASH
        CP      KEY_HASH
        JR      Z,sm_event
        ADD     A,EV_KEY0
        LD      B,A
sm_event:
        LD      A,B
        LD      (in_pkey),A
        RET

; in_poll: on a new frame, tick everything. Preserves BC, DE, HL.
in_poll:
        IN      A,(VDP_CTRL)
        RLA
        RET     NC
        PUSH    BC
        PUSH    DE
        PUSH    HL
        LD      HL,frames
        INC     (HL)
        CALL    snd_tick
        CALL    cur_tick
        CALL    sample
        POP     HL
        POP     DE
        POP     BC
        RET

; in_init: forget pending presses; what is held now is not new.
in_init:
        PUSH    BC
        PUSH    HL
        XOR     A
        LD      (frames),A
        LD      (in_pend),A
        LD      (in_pkey),A
        CALL    read_pad
        LD      A,B
        LD      (in_pad),A
        LD      (in_cpad),A
        LD      A,C
        LD      (in_key),A
        LD      (in_ckey),A
        POP     HL
        POP     BC
        RET

; in_frames: A = frames seen, which wraps at 256.
in_frames:
        CALL    in_poll
        LD      A,(frames)
        RET

; in_wait: let A frames go by; input is left for the next in_read.
in_wait:
        PUSH    BC
        LD      B,A
        CALL    in_frames
        LD      C,A
iw_loop:
        CALL    in_frames
        SUB     C
        CP      B
        JR      C,iw_loop
        POP     BC
        RET

; in_read: A = one event, or EV_NONE. A held direction repeats after
; REPEAT_DELAY frames, then every REPEAT_EVERY, while in_rpt is set.
in_read:
        CALL    in_poll
        PUSH    HL
        LD      HL,in_pkey
        LD      A,(HL)
        OR      A
        JR      Z,ir_pad
        LD      (HL),0
        POP     HL
        RET
ir_pad: LD      HL,in_pend
        LD      A,(HL)
        OR      A
        JR      Z,ir_rpt
        BIT     4,A
        JR      Z,ir_back
        RES     4,(HL)
        LD      A,EV_FIRE
        POP     HL
        RET
ir_back:
        BIT     5,A
        JR      Z,ir_dir
        RES     5,(HL)
        LD      A,EV_BACK
        POP     HL
        RET
ir_dir: LD      (HL),0                  ; one direction per press
        POP     HL
        JR      dir_event
ir_rpt: POP     HL
        LD      A,(in_rpt)
        OR      A
        RET     Z
        LD      A,(in_pad)
        AND     0FH
        RET     Z
        PUSH    BC
        LD      B,A
        LD      A,(in_held)
        LD      C,A
        LD      A,(frames)
        SUB     C
        CP      REPEAT_DELAY
        JR      C,ir_none
        LD      A,(frames)
        SUB     REPEAT_DELAY-REPEAT_EVERY
        LD      (in_held),A
        LD      A,B
        POP     BC
        JR      dir_event
ir_none:
        POP     BC
        XOR     A
        RET

; dir_event: A = the event for the first direction set in pad bits A.
dir_event:
        RRCA
        JR      NC,de_dn
        LD      A,EV_UP
        RET
de_dn:  RRCA
        JR      NC,de_lf
        LD      A,EV_DOWN
        RET
de_lf:  RRCA
        JR      NC,de_rt
        LD      A,EV_LEFT
        RET
de_rt:  RRCA
        LD      A,EV_RIGHT
        RET     C
        XOR     A
        RET
