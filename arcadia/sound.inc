; sound.inc -- the 2637's single voice (block 2).
;
; Both sound registers are SHARED with the display:
;   $18FD (PITCH)  b0-6 tone, b7 the MULTICOLOR bit -- every write keeps
;                  it set (DINI's mode);
;   $18FE (VOLUME) b0-2 loudness, b3 tone, b4 noise, b5-7 HORIZONTAL
;                  SHIFT -- writes touch b0-4 only so the screen stays put.
; A HIGHER pitch value is a LOWER tone; pitch 0 is silence.
;
; Every cue blocks for its few frames (WAITVB is the clock) and
; tail-calls BEEP, so a cue costs one return-stack entry beyond BEEP's.
; Nothing may sound after the boot handover is armed: the next image
; expects a silent machine (boot.inc writes both registers to 0).

PPITCH: DWBE    PITCH
PVOLUM: DWBE    VOLUME

VTONE   EQU     $0F             ; tone, full volume
VNOISE  EQU     $1F             ; tone + noise: the error buzz

; BEEP: tone R1 for R3 frames. BEEPV: the same with volume byte R2.
; Calls one down (WAITVB). Clobbers R0,R2,R3.
BEEP:   LODI,R2 VTONE
BEEPV:  LODZ    R1
        IORI,R0 $80
        STRA,R0 *PPITCH
        LODZ    R2
        STRA,R0 *PVOLUM
BEEP1:  ZBSR    *ZWAITVB
        BDRR,R3 BEEP1
        EORZ    R0
        STRA,R0 *PVOLUM
        LODI,R0 $80
        STRA,R0 *PPITCH
        RETC,UN

; SNDMOV: the cursor moved -- one frame, so auto-repeat never drags.
SNDMOV: LODI,R1 $06
        LODI,R3 1
        BCTR,UN BEEP

; SNDPG: a page turned.
SNDPG:  LODI,R1 $38
        LODI,R3 1
        BCTR,UN BEEP

; SNDKEY: the editor took a character.
SNDKEY: LODI,R1 $0A
        LODI,R3 2
        BCTR,UN BEEP

; SNDSEL: something was chosen -- a rising pair.
SNDSEL: LODI,R1 $18
        LODI,R3 3
        BSTR,UN BEEP
        LODI,R1 $0C
        LODI,R3 4
        BCTR,UN BEEP

; SNDBCK: backing out -- the same pair, falling.
SNDBCK: LODI,R1 $0C
        LODI,R3 3
        BSTR,UN BEEP
        LODI,R1 $20
        LODI,R3 4
        ZBRR    *ZBEEP

; SNDERR: refused or failed -- a low buzz with noise.
SNDERR: LODI,R1 $60
        LODI,R2 VNOISE
        LODI,R3 12
        BCTA,UN BEEPV

; SNDOK: WiFi connected -- three rising notes.
SNDOK:  LODI,R1 $18
        LODI,R3 5
        ZBSR    *ZBEEP
        LODI,R1 $10
        LODI,R3 5
        ZBSR    *ZBEEP
        LODI,R1 $0C
        LODI,R3 10
        ZBRR    *ZBEEP

; SNDFUJI: the boot handover -- fujinet-fujitzee's FUJI fanfare.
SNDFUJI: LODI,R1 $18
        LODI,R3 5
        ZBSR    *ZBEEP
        LODI,R1 $10
        LODI,R3 5
        ZBSR    *ZBEEP
        LODI,R1 $0C
        LODI,R3 5
        ZBSR    *ZBEEP
        LODI,R1 $06
        LODI,R3 14
        ZBRR    *ZBEEP
