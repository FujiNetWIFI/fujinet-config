; splash.inc -- the power-on splash (block 1).
;
; SPLASH draws a list of raw-cell runs (strings.inc, TSPLSH) at R0:R1,
; plays the FUJI fanfare, then holds the screen until a key or ~1.5 s.
; The next screen clears it. Calls three down (UPAUSE). Clobbers R0-R3,
; V_T0.
SPLASH: STRA,R0 DSTRP
        STRA,R1 DSTRP+1
        LODI,R3 $FF
SPLRUN:   LODA,R0 *DSTRP,R3,+     ; a run's cell-1; 0 ends the list
        BCTR,EQ SPLEND
        STRZ    R2
SPLCEL:   LODA,R0 *DSTRP,R3,+
        BCTR,EQ SPLRUN
        STRA,R0 SCREEN,R2,+
        BCTR,UN SPLCEL
SPLEND:   BSTA,UN SNDFUJI
        LODI,R3 90
        BCTA,UN UPAUSE
