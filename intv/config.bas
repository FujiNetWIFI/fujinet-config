' config.bas -- FujiNet CONFIG for Intellivision. Entry point and top-level
' state dispatcher, mirroring fujinet-config's src/main.c state machine:
'   CHECK_WIFI -> CONNECT_WIFI -> SET_WIFI -> HOSTS -> SELECT_FILE -> BOOT
' plus an INFO screen. See the plan doc for the full design rationale.
'
' Execution is sequential in IntyBASIC, and INCLUDE pastes files in
' verbatim -- falling into a PROCEDURE or DATA body by straight-line
' execution corrupts the return stack. So every INCLUDE happens here, ahead
' of a GOTO that jumps clear of all of them before real code starts.
    GOTO cfg_start

    INCLUDE "constants.bas"
    INCLUDE "fujinet.bas"
    INCLUDE "fujicmd.bas"
    INCLUDE "screen.bas"
    INCLUDE "scroll.bas"
    INCLUDE "input.bas"
    INCLUDE "st_wifi.bas"
    INCLUDE "st_hosts.bas"
    INCLUDE "st_file.bas"
    INCLUDE "st_info.bas"

    ' The default $5000-$6FFF segment (8K words) is essentially full as of
    ' st_wifi.bas/st_hosts.bas/st_file.bas -- st_boot.bas pushed total size
    ' just past it. Rather than let the compiler auto-continue into a third
    ' $7000 page (observed to produce a cart that fails EXEC's boot
    ' detection under jzIntv -- PC lands in unprogrammed GROM after 2
    ' instructions), give it an explicit segment per the IntyBASIC manual's
    ' own guidance ("Beyond that, add segments manually: ASM ORG $D000").
    ASM ORG $D000
    INCLUDE "st_boot.bas"
    INCLUDE "st_lobby.bas"
    INCLUDE "st_copy.bas"
    INCLUDE "fgbg.bas"

    ' $D000-$DFFF filled up in turn once csbar.bas landed, and the compiler
    ' quietly auto-continued into $E000 -- which is NOT one of the two areas
    ' the manual says a cart may use ("$D000-$DFFF and $F000-$FFFF"), and is
    ' the same silent-spill behaviour the $7000 note above is about. Take the
    ' second sanctioned area explicitly instead. Everything from here on,
    ' including cfg_start and the main loop, assembles into $F000.
    ASM ORG $F000
    INCLUDE "csbar.bas"

cfg_start:
    ' fj_open_directory reads SC_FILTER on every call, and cart RAM comes up
    ' with undefined contents -- clear it before anything can send garbage as
    ' a search pattern. (sf_init clears it again per browse; this is the
    ' cold-boot floor.)
    POKE (SC_FILTER), 0

    GOSUB cb_define_glyphs   ' file-browser folder/cartridge cards into GRAM
    GOSUB scr_clear
    GOSUB fn_wait_mailbox
    IF fn_ok = 0 THEN
        GOSUB scr_no_mailbox
        GOTO cfg_halt
    END IF

    state = ST_CHECK_WIFI

cfg_main_loop:
    WAIT

    ' ST_HOSTS and ST_INFO are drawn in foreground/background mode (fgbg.bas
    ' explains why); every other screen stays in the color stack mode IntyBASIC
    ' starts in, so filenames and the character grid keep their lowercase.
    ' Switching here rather than in each screen covers every entry and exit
    ' path in one place -- and there are several, since ST_INFO,
    ' ST_SELECT_FILE, ST_BOOT and st_copy.bas all fall back to ST_HOSTS.
    '
    ' This is a profile id and NOT a mode flag, deliberately: ST_HOSTS ->
    ' ST_INFO is FGBG on both sides and differs only in border colour. As a
    ' bare "is FGBG" boolean the value would not change across that move, the
    ' switch would never fire, and the info screen would inherit the host
    ' screen's blue border.
    '
    ' The CLS is safe on every transition because each screen redraws itself
    ' from scratch (st_wifi/st_file/st_boot all scr_clear, st_info and
    ' st_hosts have shown-flags that are already 0 by the time they are left).
    ' Without it the outgoing screen's BACKTAB is reinterpreted under the new
    ' mode for a frame, which reads as garbage.
    vid_want = 0
    IF state = ST_HOSTS THEN vid_want = 1
    IF state = ST_INFO THEN vid_want = 2
    IF state = ST_SELECT_FILE THEN vid_want = 3
    IF state = ST_BOOT THEN vid_want = 4
    IF vid_want <> vid_now THEN
        vid_now = vid_want
        CLS
        IF vid_now = 1 THEN
            MODE 1 : BORDER CS_BLUE
        ELSEIF vid_now = 2 THEN
            MODE 1 : BORDER CS_GREEN
        ELSEIF vid_now = 3 THEN
            ' Color stack, not FGBG -- filenames have to keep their lowercase.
            ' p0 dark green bars, p1 content, p2 selection bar, p3 content again;
            ' csbar.bas explains why the last one repeats.
            MODE 0,CS_DARKGREEN,CS_BLUE,CS_CYAN,CS_BLUE : BORDER CS_DARKGREEN
        ELSEIF vid_now = 4 THEN
            ' One background for the whole boot screen, so it needs no advance
            ' bits at all. All four entries are the same colour deliberately:
            ' a stray bit 13 on some cell then costs nothing instead of
            ' producing a surprise band.
            MODE 0,CS_DARKGREEN,CS_DARKGREEN,CS_DARKGREEN,CS_DARKGREEN : BORDER CS_DARKGREEN
        ELSE
            MODE 0,0,0,0,0 : BORDER CS_BLACK
        END IF
        ' MODE lands on the next frame, and borrows the color variable to carry
        ' its arguments until then -- so no PRINT COLOR until after this WAIT.
        WAIT
    END IF

    ' 0-based: ST_CHECK_WIFI=0 .. ST_BOOT=6 (see constants.bas), matching
    ' this label order exactly -- ON...GOSUB indexes by state directly.
    ON state GOSUB do_check_wifi, do_connect_wifi, do_set_wifi, \
                   do_hosts, do_select_file, do_info, do_boot
    GOTO cfg_main_loop

cfg_halt:
    WAIT
    GOTO cfg_halt

' ---------------------------------------------------------------------------
' scr_no_mailbox: shown once at boot if the RP2040 mailbox magic never
' appeared within fn_wait_mailbox's 3-second window.
' ---------------------------------------------------------------------------
scr_no_mailbox: PROCEDURE
    PRINT AT screenpos(0,0) COLOR COL_NORMAL,"FUJINET CONFIG  INTV"
    PRINT AT screenpos(2,3) COLOR COL_ERROR,"NO CARTRIDGE"
    PRINT AT screenpos(2,4) COLOR COL_ERROR,"MAILBOX"
END
