; splash.inc -- the boot splash: the FujiNet wordmark fading in under the
; fanfare. Fire goes on at any point; half a second after the fanfare
; ends it goes on by itself, fading out.

SPLASH_ROW      EQU     6               ; the logo's ten rows
LOGO_TILE       EQU     80H             ; logo.inc's names start here
SPLASH_QUIET    EQU     30
PRESS_ROW       EQU     21

; centre: the string HL centred on row D.
centre: CALL    str_len
        LD      A,DISP_COLS
        SUB     B
        SRL     A
        LD      E,A
        JP      disp_str

splash_show:
        LD      A,80H                   ; drawn in the dark
        LD      B,1
        CALL    vdp_reg
        XOR     A
        CALL    col_apply
        LD      HL,LOGO_PATTERNS
        LD      DE,PATT_TAB+LOGO_TILE*8
        LD      BC,96*8
        CALL    vdp_write
        LD      HL,LOGO_NAMES
        LD      DE,NAME_TAB+SPLASH_ROW*DISP_COLS
        LD      BC,10*DISP_COLS
        CALL    vdp_write
        LD      HL,str_for_coleco
        LD      D,18
        CALL    centre
        LD      HL,str_press_fire
        LD      D,PRESS_ROW
        CALL    centre
        LD      A,0C0H
        LD      B,1
        CALL    vdp_reg
        LD      A,SND_TITLE
        CALL    snd_play
        LD      DE,PRESS_ROW*256+9      ; two left of "PRESS FIRE"
        LD      BC,T_TRI*256+' '
        CALL    cur_show
        XOR     A
        LD      (tmp_a),A               ; tmp_a: frames since the last level
        LD      (tmp_b),A               ; tmp_b: frames since the fanfare
        CALL    in_frames
        LD      (tmp_w),A
spl_loop:
        CALL    in_read
        CP      EV_FIRE
        JR      Z,spl_done
        CALL    in_frames
        LD      HL,tmp_w
        CP      (HL)
        JR      Z,spl_loop
        LD      (HL),A
        LD      HL,tmp_a
        INC     (HL)
        LD      A,(HL)
        CP      FADE_STEP
        JR      C,spl_quiet
        LD      (HL),0
        LD      A,(fade_level)
        CP      FADE_FULL
        JR      NC,spl_quiet
        INC     A
        CALL    col_apply
spl_quiet:
        CALL    snd_busy
        JR      NZ,spl_loop
        LD      HL,tmp_b
        INC     (HL)
        LD      A,(HL)
        CP      SPLASH_QUIET+1
        JR      C,spl_loop
spl_done:
        CALL    cur_hide
        CALL    fade_out
        JP      snd_init
