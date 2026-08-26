' fgbg.bas -- the one helper the foreground/background-mode HOST SLOTS screen
' needs. FGBG gives every cell its own background colour out of all 16 (see the
' BG_* constants and the bit-order warning in constants.bas), which is the only
' way to draw this screen: the colour stack holds just four entries, resets to
' position 0 each frame and only ever advances forward, and a tan selection bar
' sitting in the middle of the dark green list would force dark green to occupy
' two of those four slots -- leaving no room for blue, red and purple as well.
'
' FGBG's price is that only GROM cards 0-63 are reachable, because word bits 9
' and 10 are taken for the background. Card = ascii-32, so that is ASCII 32-95:
' uppercase only. Hostnames are folded to uppercase on the way to the screen
' (DNS is case-insensitive, and SC_HOSTS itself is never touched).
'
' Placed in the $D000 segment (INCLUDEd after st_boot.bas in config.bas) for
' the same reason st_lobby.bas is: the default $5000-$6FFF segment is already
' essentially full per config.bas's own comment, and screen.bas/st_hosts.bas
' have more claim on it than this does.

' ---------------------------------------------------------------------------
' scr_fgbg_row: fix up row s_row for FGBG mode. For all 20 columns: fold
' lowercase to uppercase, blank any card FGBG cannot show, and stamp the
' background #s_bg in -- each cell keeping its own glyph and foreground colour.
' Call AFTER the row's text has been drawn (PRINT and scr_puts both write plain
' card*8+color words, with no background bits at all).
'
' Cards are ascii-32, so 'a'-'z' are cards 65-90 and 'A'-'Z' cards 33-58: the
' fold is a flat -32 on the card number. The /8 to recover the card is the same
' move scr_hilite_digits makes. Reuses screen.bas's s_i/s_c/#s_val scratch,
' which is safe because this only ever runs after scr_puts/scr_recolor, never
' inside one.
'
' *** Call this EXACTLY ONCE per row, after every character on it is final. ***
' It is not idempotent and cannot be made so: before painting, cards 64-94 --
' the lowercase range the fold exists to catch -- occupy word bits 9 and 10,
' which are precisely FGBG's background bits 0 and 1. There is no way to tell
' "unpainted lowercase card" from "painted card carrying a background", so a
' second pass reads part of the background back as card number and garbles the
' row. BG_DARKGREEN ($2000) happens to survive a repeat; BG_GREEN and BG_YELLOW
' do not. Redraw the row's text and paint again rather than painting twice.
' ---------------------------------------------------------------------------
scr_fgbg_row: PROCEDURE
    FOR s_i = 0 TO SCREEN_COLS - 1
        #s_val = #BACKTAB(s_row * SCREEN_COLS + s_i)
        s_c = (#s_val / 8) AND 255                       ' card number
        IF s_c >= 65 AND s_c <= 90 THEN s_c = s_c - 32   ' 'a'-'z' -> 'A'-'Z'
        IF s_c > 63 THEN s_c = 0                         ' unreachable in FGBG
        #BACKTAB(s_row * SCREEN_COLS + s_i) = s_c * 8 + (#s_val AND 7) + #s_bg
    NEXT s_i
END
