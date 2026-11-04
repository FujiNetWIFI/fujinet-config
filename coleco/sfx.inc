; sfx.inc -- CONFIG's sounds, the Master System CONFIG's: effects doubled
; on T1+T2 a few period units apart, a lead that decays and only then
; wavers, a swelling bass, a plucked arpeggio, noise hats and snare.
; Stream format: sound.inc.

SND_MOVE        EQU     0               ; cursor moved: a soft blip
SND_OK          EQU     1               ; confirm: the blip, rising
SND_BACK        EQU     2               ; back / cancel: the blip, falling
SND_TYPE        EQU     3               ; a key typed: a noise tick
SND_ERROR       EQU     4               ; a low, beating buzz
SND_CONNECT     EQU     5               ; joined the network: a sparkle
SND_READY       EQU     6               ; the game is loaded: a chime
SND_TITLE       EQU     7               ; the splash fanfare

; notes, as semitones above A2
N_A2    EQU     0
N_AS2   EQU     1
N_C3    EQU     3
N_D3    EQU     5
N_FS3   EQU     9
N_A3    EQU     12
N_AS3   EQU     13
N_C4    EQU     15
N_CS4   EQU     16
N_D4    EQU     17
N_E4    EQU     19
N_F4    EQU     20
N_G4    EQU     22
N_A4    EQU     24
N_AS4   EQU     25
N_B4    EQU     26
N_C5    EQU     27
N_D5    EQU     29
N_E5    EQU     31
N_F5    EQU     32
N_G5    EQU     34
N_A5    EQU     36
N_B5    EQU     38
N_C6    EQU     39
N_E6    EQU     43
N_G6    EQU     46
N_C7    EQU     51
N_E7    EQU     55

S_REST  EQU     60H
S_RAW   EQU     64H
S_ENV   EQU     70H
S_VOL   EQU     80H
S_VIB   EQU     90H
S_END   EQU     0FFH

NZ_HAT          EQU     04H             ; white noise, the two fastest rates
NZ_SNARE        EQU     05H

; ---- envelopes: attenuation per frame (0 loudest, 15 off), FEH holds ----
ENV_FLAT        EQU     0
ENV_BLIP        EQU     1
ENV_LEAD        EQU     2
ENV_BASS        EQU     3
ENV_PLUCK       EQU     4
ENV_HAT         EQU     5
ENV_SNARE       EQU     6
ENV_CHIME       EQU     7
ENV_BUZZ        EQU     8
ENV_SPARK       EQU     9
ENV_TICK        EQU     10

SND_ENVS:
        DW      envd_flat,envd_blip,envd_lead,envd_bass,envd_pluck,envd_hat
        DW      envd_snare,envd_chime,envd_buzz,envd_spark,envd_tick

envd_flat:
        DB      0,0FEH
envd_blip:
        DB      0,0,1,2,2,3,15,0FEH
envd_lead:
        DB      1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,7
        DB      8,8,8,8,9,9,9,9,10,10,10,10,11,0FEH
envd_bass:
        DB      6,5,4,5,6,6,7,7,8,0FEH
envd_pluck:
        DB      0,2,4,6,8,10,0FEH
envd_hat:
        DB      4,6,15,0FEH
envd_snare:
        DB      2,2,3,4,4,5,6,6,8,10,12,15,0FEH
envd_chime:
        DB      0,1,1,2,2,3,3,4,5,6,7,9,11,15,0FEH
envd_buzz:
        DB      1,1,1,2,2,3,3,4,4,5,6,7,8,10,12,15,0FEH
envd_spark:
        DB      1,2,3,4,5,6,7,8,9,10,12,15,0FEH
envd_tick:
        DB      0,3,15,0FEH

; ---- effects ----------------------------------------------------------------

; The blip: about 432 Hz, gone in six frames.
move_t1:
        DB      S_ENV|ENV_BLIP,S_RAW|01H,03H,7,S_END
ok_t1:  DB      S_ENV|ENV_BLIP,N_A4,3,N_E5,7,S_END
back_t1:
        DB      S_ENV|ENV_BLIP,N_A4,3,N_E4,7,S_END
type_nz:
        DB      S_ENV|ENV_TICK,NZ_HAT,3,S_END

; Down a tritone, low and wide, so the two copies beat.
error_t1:
        DB      S_ENV|ENV_BUZZ,N_FS3,6,N_C3,16,S_END

; Two rising C major arpeggios, the second a frame behind and a third up.
connect_t1:
        DB      S_ENV|ENV_SPARK,N_C5,3,N_E5,3,N_G5,3,N_C6,3,N_E6,3,N_G6,3
        DB      N_C7,8,S_END
connect_t2:
        DB      S_ENV|ENV_SPARK,S_VOL|3,S_REST,1,N_E5,3,N_G5,3,N_C6,3,N_E6,3
        DB      N_G6,3,N_C7,3,N_E7,6,S_END

; Pick-up notes into a ringing high B.
ready_t1:
        DB      S_ENV|ENV_CHIME,N_G4,2,S_REST,1,N_B4,2,S_REST,1,N_D5,2
        DB      N_G5,2,N_B5,12,S_END

; ---- the splash fanfare: Dm Bb | C A, a 12-frame beat, 192 frames ----------
title_lead:
        DB      S_ENV|ENV_LEAD,S_VIB|1
        DB      N_D5,24,N_A4,12,N_D5,12
        DB      N_F5,24,N_G5,12,N_F5,12
        DB      N_E5,12,N_C5,12,N_E5,12,N_G5,12
        DB      N_A5,48,S_END

title_bass:
        DB      S_ENV|ENV_BASS
        DB      N_D3,18,N_D3,6,N_D3,24,N_AS2,18,N_AS2,6,N_AS2,24
        DB      N_C3,18,N_C3,6,N_C3,24,N_A2,18,N_A2,6,N_A2,24
        DB      S_END

title_arp:
        DB      S_ENV|ENV_PLUCK,S_VOL|4
        DB      N_D4,6,N_F4,6,N_A4,6,N_D5,6,N_F4,6,N_A4,6,N_D5,6,N_A4,6
        DB      N_AS3,6,N_D4,6,N_F4,6,N_AS4,6,N_D4,6,N_F4,6,N_AS4,6,N_F4,6
        DB      N_C4,6,N_E4,6,N_G4,6,N_C5,6,N_E4,6,N_G4,6,N_C5,6,N_G4,6
        DB      N_A3,6,N_CS4,6,N_E4,6,N_A4,6,N_CS4,6,N_E4,6,N_A4,6,N_E4,6
        DB      S_END

title_drums:
        DB      S_VOL|1
        DB      S_ENV|ENV_HAT,NZ_HAT,6,NZ_HAT,6,NZ_HAT,6,NZ_HAT,6
        DB      S_ENV|ENV_SNARE,NZ_SNARE,12,S_ENV|ENV_HAT,NZ_HAT,6,NZ_HAT,6
        DB      S_ENV|ENV_HAT,NZ_HAT,6,NZ_HAT,6,NZ_HAT,6,NZ_HAT,6
        DB      S_ENV|ENV_SNARE,NZ_SNARE,12,S_ENV|ENV_HAT,NZ_HAT,6,NZ_HAT,6
        DB      S_ENV|ENV_HAT,NZ_HAT,6,NZ_HAT,6,NZ_HAT,6,NZ_HAT,6
        DB      S_ENV|ENV_SNARE,NZ_SNARE,12,S_ENV|ENV_HAT,NZ_HAT,6,NZ_HAT,6
        DB      S_ENV|ENV_HAT,NZ_HAT,6,NZ_HAT,6,NZ_HAT,6,NZ_HAT,6
        DB      S_ENV|ENV_SNARE,NZ_SNARE,12,S_ENV|ENV_HAT,NZ_HAT,6,NZ_HAT,6
        DB      S_END

; Each: the T0, T1, T2 and noise streams (0 = not used), the chorus, and
; the starting volume offset.
SND_DEFS:
        DW      0,move_t1,0,0
        DB      9,5
        DW      0,ok_t1,0,0
        DB      9,4
        DW      0,back_t1,0,0
        DB      9,4
        DW      0,0,0,type_nz
        DB      0,3
        DW      0,error_t1,0,0
        DB      20,2
        DW      0,connect_t1,connect_t2,0
        DB      0,1
        DW      0,ready_t1,0,0
        DB      8,1
        DW      title_lead,title_bass,title_arp,title_drums
        DB      0,0
