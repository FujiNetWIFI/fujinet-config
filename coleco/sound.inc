; sound.inc -- the SN76489 on port FFH: a small sequencer for effects and
; the splash fanfare, the Master System CONFIG's, which shares the chip
; and its clock.
;
; Ticked once a frame from in_poll, so it runs only while CONFIG waits on
; the user or on in_wait; a mailbox transaction freezes it. Hence
; snd_settle before anything long.
;
; A sound is up to four voice streams (T0, T1, T2, noise). Playing one
; replaces only the voices it has streams for. A "chorus" sound puts T2 on
; T1's line a few period units flat and a step quieter.
;
; Stream bytes (sfx.inc):
;   00-5F n, len   note n semitones above A2, len frames; on the noise
;                  voice n is the noise control instead
;   60 len         rest
;   64|hi lo len   a raw 10-bit tone period
;   70|e           envelope e (SND_ENVS)
;   80|v           volume offset added to the envelope
;   90|on          delayed vibrato (after 10 frames, toward flat)
;   FF             end; the voice falls silent

PSG_PORT        EQU     0FFH

V_PTR           EQU     0               ; 2: next stream byte; 0 = idle
V_ENV0          EQU     2               ; 2: envelope, from the top
V_ENV           EQU     4               ; 2: where this note has got to
V_PERIOD        EQU     6               ; 2: tone period or noise control
V_LEFT          EQU     8               ; frames left of this note
V_REST          EQU     9
V_VOL           EQU     10
V_VIB           EQU     11
V_AGE           EQU     12              ; frames since the note began
V_CHORUS        EQU     13              ; T1: T2 doubles it, this much flat
VOICE_SIZE      EQU     14

; A2 up to G#3; each octave up halves the period.
note_period:
        DW      1017,960,906,855,807,762,719,679,641,605,571,539

; Vibrato only bends flat.
vib_offset:
        DB      0,1,3,5,5,3,1,0

; psg_tone: tone channel C to period HL (at most 1023). Clobbers A, B, HL.
psg_tone:
        LD      A,H
        CP      4
        JR      C,pt_ok
        LD      HL,1023
pt_ok:  LD      A,C
        RRCA
        RRCA
        RRCA
        LD      B,A
        LD      A,L
        AND     0FH
        OR      B
        OR      80H
        OUT     (PSG_PORT),A
        LD      A,L                     ; period >> 4
        RRCA
        RRCA
        RRCA
        RRCA
        AND     0FH
        LD      B,A
        LD      A,H
        RLCA
        RLCA
        RLCA
        RLCA
        AND     30H
        OR      B
        OUT     (PSG_PORT),A
        RET

; psg_att: channel C to attenuation A. Clobbers A, B.
psg_att:
        LD      B,A
        LD      A,C
        RRCA
        RRCA
        RRCA
        OR      90H
        OR      B
        OUT     (PSG_PORT),A
        RET

; snd_init: silence, and forget everything.
snd_init:
        PUSH    BC
        LD      HL,voices
        LD      C,0
si_loop:
        XOR     A
        LD      (HL),A
        INC     HL
        LD      (HL),A
        LD      DE,VOICE_SIZE-1
        ADD     HL,DE
        LD      A,15
        CALL    psg_att
        INC     C
        LD      A,C
        CP      4
        JR      C,si_loop
        XOR     A
        LD      (snd_slaved),A
        POP     BC
        RET

; snd_play: start sound A.
snd_play:
        PUSH    IX
        LD      L,A                     ; HL = SND_DEFS + 10 x A
        LD      H,0
        ADD     HL,HL
        LD      D,H
        LD      E,L
        ADD     HL,HL
        ADD     HL,HL
        ADD     HL,DE
        LD      DE,SND_DEFS
        ADD     HL,DE
        LD      (snd_def),HL
        LD      DE,9
        ADD     HL,DE
        LD      A,(HL)
        LD      (snd_vol),A
        LD      HL,(snd_def)
        LD      IX,voices
        LD      B,4
sp_voice:
        LD      E,(HL)
        INC     HL
        LD      D,(HL)
        INC     HL
        LD      A,D
        OR      E
        JR      Z,sp_next
        LD      (IX+V_PTR),E
        LD      (IX+V_PTR+1),D
        LD      DE,(SND_ENVS)
        LD      (IX+V_ENV0),E
        LD      (IX+V_ENV0+1),D
        LD      (IX+V_LEFT),0
        LD      (IX+V_VIB),0
        LD      (IX+V_CHORUS),0
        LD      A,(snd_vol)
        LD      (IX+V_VOL),A
sp_next:
        LD      DE,VOICE_SIZE
        ADD     IX,DE
        DJNZ    sp_voice
        LD      B,(HL)                  ; B = the chorus
        LD      IX,(snd_def)
        LD      A,(IX+4)                ; a T2 stream runs on its own
        OR      (IX+5)
        LD      C,A
        JR      Z,sp_t1
        XOR     A
        LD      (snd_slaved),A
sp_t1:  LD      A,(IX+2)                ; a T1 stream sets up the chorus
        OR      (IX+3)
        JR      Z,sp_done
        LD      A,(snd_slaved)
        OR      A
        JR      Z,sp_lead
        LD      A,B
        OR      C
        JR      NZ,sp_lead
        PUSH    BC                      ; T2 loses its leader
        LD      C,2
        LD      A,15
        CALL    psg_att
        POP     BC
sp_lead:
        LD      A,B
        LD      (voices+VOICE_SIZE+V_CHORUS),A
        OR      A
        JR      Z,sp_solo
        LD      A,1
        LD      (snd_slaved),A
        XOR     A
        LD      (voices+2*VOICE_SIZE+V_PTR),A
        LD      (voices+2*VOICE_SIZE+V_PTR+1),A
        JR      sp_done
sp_solo:
        LD      (snd_slaved),A
sp_done:
        POP     IX
        RET

; snd_busy: NZ while any voice is playing. Preserves BC, DE, HL.
snd_busy:
        PUSH    HL
        PUSH    BC
        LD      HL,voices
        LD      B,4
sb_loop:
        LD      A,(HL)
        INC     HL
        OR      (HL)
        JR      NZ,sb_done
        LD      A,L
        ADD     A,VOICE_SIZE-1
        LD      L,A
        JR      NC,sb_next
        INC     H
sb_next:
        DJNZ    sb_loop
        XOR     A
sb_done:
        POP     BC
        POP     HL
        RET

; snd_settle: before a long transaction, let a short sound finish (20
; frames at most), then silence.
snd_settle:
        CALL    in_frames
        LD      C,A
ss_loop:
        CALL    snd_busy
        JR      Z,ss_done
        CALL    in_frames
        SUB     C
        CP      20
        JR      C,ss_loop
ss_done:
        JP      snd_init

; snd_tick: once a frame. Preserves IX.
snd_tick:
        PUSH    IX
        LD      IX,voices
        LD      C,0
st_loop:
        PUSH    BC
        CALL    voice_tick
        POP     BC
        LD      DE,VOICE_SIZE
        ADD     IX,DE
        INC     C
        LD      A,C
        CP      4
        JR      C,st_loop
        POP     IX
        RET

; voice_tick: voice IX, channel C.
voice_tick:
        LD      A,(IX+V_PTR)
        OR      (IX+V_PTR+1)
        RET     Z
        LD      A,(IX+V_LEFT)
        OR      A
        JR      NZ,vt_play
        CALL    next_note
        JR      C,vt_play
        LD      A,15                    ; the stream ended
        CALL    psg_att
        LD      A,C
        CP      1
        RET     NZ
        LD      A,(snd_slaved)
        OR      A
        RET     Z
        XOR     A
        LD      (snd_slaved),A
        LD      C,2
        LD      A,15
        JP      psg_att
vt_play:
        LD      A,(IX+V_REST)
        OR      A
        LD      A,15
        JR      NZ,vt_att
        LD      L,(IX+V_ENV)            ; attenuation = envelope + volume
        LD      H,(IX+V_ENV+1)
        LD      A,(HL)
        ADD     A,(IX+V_VOL)
        CP      16
        JR      C,vt_env
        LD      A,15
vt_env: PUSH    AF
        INC     HL                      ; step the envelope unless it holds
        LD      A,(HL)
        CP      0FEH
        JR      Z,vt_held
        LD      (IX+V_ENV),L
        LD      (IX+V_ENV+1),H
vt_held:
        LD      A,C
        CP      3
        JR      NC,vt_noise
        LD      L,(IX+V_PERIOD)
        LD      H,(IX+V_PERIOD+1)
        LD      A,(IX+V_VIB)
        OR      A
        JR      Z,vt_tone
        LD      A,(IX+V_AGE)
        SUB     10
        JR      C,vt_tone
        AND     7
        LD      E,A
        LD      D,0
        PUSH    HL
        LD      HL,vib_offset
        ADD     HL,DE
        LD      E,(HL)
        POP     HL
        ADD     HL,DE
vt_tone:
        PUSH    HL
        CALL    psg_tone
        POP     HL
        LD      A,C
        CP      1
        JR      NZ,vt_noise
        LD      A,(snd_slaved)
        OR      A
        JR      Z,vt_noise
        LD      E,(IX+V_CHORUS)
        LD      D,0
        ADD     HL,DE
        LD      C,2
        CALL    psg_tone
        LD      C,1
vt_noise:
        POP     AF
vt_att: PUSH    AF
        CALL    psg_att
        POP     AF
        LD      B,A
        LD      A,C
        CP      1
        JR      NZ,vt_age
        LD      A,(snd_slaved)
        OR      A
        JR      Z,vt_age
        LD      A,B                     ; T2 a step quieter
        CP      15
        JR      NC,vt_t2
        INC     A
vt_t2:  LD      C,2
        CALL    psg_att
        LD      C,1
vt_age: LD      A,(IX+V_AGE)
        INC     A
        JR      Z,vt_left
        LD      (IX+V_AGE),A
vt_left:
        DEC     (IX+V_LEFT)
        RET

; next_note: read voice IX's stream to its next note or rest. Carry set
; if there is one, clear at the end (the voice is idle). Preserves C.
next_note:
        LD      L,(IX+V_PTR)
        LD      H,(IX+V_PTR+1)
nn_byte:
        LD      A,(HL)
        INC     HL
        CP      0FFH
        JR      NZ,nn_cmd
        XOR     A                       ; the end: carry clear
        LD      (IX+V_PTR),A
        LD      (IX+V_PTR+1),A
        RET
nn_cmd: CP      60H
        JR      C,nn_note
        JR      Z,nn_rest
        CP      68H
        JR      C,nn_raw
        CP      80H
        JR      C,nn_envl
        CP      90H
        JR      C,nn_vol
        AND     1
        LD      (IX+V_VIB),A
        JR      nn_byte
nn_envl:
        AND     0FH
        PUSH    HL
        ADD     A,A
        LD      E,A
        LD      D,0
        LD      HL,SND_ENVS
        ADD     HL,DE
        LD      E,(HL)
        INC     HL
        LD      D,(HL)
        LD      (IX+V_ENV0),E
        LD      (IX+V_ENV0+1),D
        POP     HL
        JR      nn_byte
nn_vol: AND     0FH
        LD      (IX+V_VOL),A
        JR      nn_byte
nn_rest:
        LD      (IX+V_REST),1
        JR      nn_len
nn_raw: AND     3
        LD      (IX+V_PERIOD+1),A
        LD      A,(HL)
        INC     HL
        LD      (IX+V_PERIOD),A
        LD      (IX+V_REST),0
        JR      nn_len
nn_note:
        LD      (IX+V_REST),0
        LD      B,A
        LD      A,C
        CP      3
        LD      A,B
        JR      NZ,nn_tone
        LD      (IX+V_PERIOD),A         ; noise: the control value
        LD      (IX+V_PERIOD+1),0
        JR      nn_len
nn_tone:
        LD      B,0                     ; B = octave, A = note in it
nn_oct: CP      12
        JR      C,nn_look
        SUB     12
        INC     B
        JR      nn_oct
nn_look:
        PUSH    HL
        ADD     A,A
        LD      E,A
        LD      D,0
        LD      HL,note_period
        ADD     HL,DE
        LD      E,(HL)
        INC     HL
        LD      D,(HL)
        EX      DE,HL
        INC     B
nn_shift:
        DEC     B
        JR      Z,nn_period
        SRL     H
        RR      L
        JR      nn_shift
nn_period:
        LD      (IX+V_PERIOD),L
        LD      (IX+V_PERIOD+1),H
        POP     HL
nn_len: LD      A,(HL)
        INC     HL
        LD      (IX+V_LEFT),A
        LD      (IX+V_PTR),L
        LD      (IX+V_PTR+1),H
        LD      A,(IX+V_ENV0)
        LD      (IX+V_ENV),A
        LD      A,(IX+V_ENV0+1)
        LD      (IX+V_ENV+1),A
        LD      (IX+V_AGE),0
        LD      A,C
        CP      3
        JR      NZ,nn_ok
        LD      A,(IX+V_REST)
        OR      A
        JR      NZ,nn_ok
        LD      A,(IX+V_PERIOD)         ; a new noise note resets the noise
        OR      0E0H
        OUT     (PSG_PORT),A
nn_ok:  SCF
        RET
