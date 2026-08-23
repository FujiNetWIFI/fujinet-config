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

cfg_start:
    ' fj_open_directory reads SC_FILTER on every call, and cart RAM comes up
    ' with undefined contents -- clear it before anything can send garbage as
    ' a search pattern. (sf_init clears it again per browse; this is the
    ' cold-boot floor.)
    POKE (SC_FILTER), 0

    GOSUB scr_clear
    GOSUB fn_wait_mailbox
    IF fn_ok = 0 THEN
        GOSUB scr_no_mailbox
        GOTO cfg_halt
    END IF

    state = ST_CHECK_WIFI

cfg_main_loop:
    WAIT
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
