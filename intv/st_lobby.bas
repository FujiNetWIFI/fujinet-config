' st_lobby.bas -- keypad 0 on the HOST SLOTS screen: mount and boot the
' FujiNet Game Lobby ROM from ec.tnfs.io/intv/lobby.rom, so a user can
' reach the lobby without navigating the host/file browser. Reuses ST_BOOT
' (st_boot.bas) unchanged -- this just plays the same role st_hosts.bas's
' own BTN-mount-and-browse path does, except it lands on ST_BOOT directly
' instead of ST_SELECT_FILE, since the lobby path is fixed rather than
' user-chosen.
'
' Placed in the $D000 segment (INCLUDEd after st_boot.bas in config.bas):
' the default $5000-$6FFF segment is already essentially full per
' config.bas's own comment, and this has no reason to compete for that
' space over st_wifi/st_hosts/st_file, which do.

lit_lobby_host: DATA 101,99,46,116,110,102,115,46,105,111
lit_lobby_path: DATA 47,105,110,116,118,47,108,111,98,98,121,46,114,111,109

    CONST LEN_LOBBY_HOST = 10
    CONST LEN_LOBBY_PATH = 15

    DIM lb_i, lb_j, lb_c, lb_found, lb_slot

' ---------------------------------------------------------------------------
' hosts_launch_lobby: called from do_hosts on keypad 0. SC_HOSTS is already
' populated (do_hosts calls fj_read_host_slots on entry). Case-insensitively
' finds ec.tnfs.io among the 8 slots, or claims the last one if absent --
' same policy fujinet-lobby/intv/st_boot.bas uses for its own host slots.
' ---------------------------------------------------------------------------
hosts_launch_lobby: PROCEDURE
    lb_found = 0
    FOR lb_slot = 0 TO NUM_HOST_SLOTS - 1
        GOSUB lb_slot_matches
        IF lb_c = 1 THEN
            lb_found = 1
            EXIT FOR
        END IF
    NEXT lb_slot

    IF lb_found = 0 THEN
        lb_slot = NUM_HOST_SLOTS - 1
        FOR lb_j = 0 TO LEN_LOBBY_HOST - 1
            POKE (SC_HOSTS + lb_slot * HOST_NAME_LEN + lb_j), PEEK(VARPTR lit_lobby_host(0) + lb_j) AND 255
        NEXT lb_j
        POKE (SC_HOSTS + lb_slot * HOST_NAME_LEN + LEN_LOBBY_HOST), 0
        GOSUB fj_write_host_slots
    END IF

    host_slot = lb_slot
    fc_hs = host_slot
    GOSUB fj_mount_host
    IF fn_ok = 0 THEN
        ' Returns with state still ST_HOSTS, so the screen is live in FGBG and
        ' row 11 has just lost its purple to the PRINT.
        PRINT AT screenpos(0,11) COLOR COL_ERROR,"MOUNT FAILED        "
        s_row = 11 : #s_bg = BG_PURPLE : GOSUB scr_fgbg_row
        RETURN
    END IF

    FOR lb_j = 0 TO LEN_LOBBY_PATH - 1
        POKE (SC_BOOTPATH + lb_j), PEEK(VARPTR lit_lobby_path(0) + lb_j) AND 255
    NEXT lb_j
    POKE (SC_BOOTPATH + LEN_LOBBY_PATH), 0

    hosts_shown = 0
    state = ST_BOOT
END

' lb_slot_matches: case-insensitive compare of SC_HOSTS slot `lb_slot`
' against lit_lobby_host. Sets lb_c to 1/0.
lb_slot_matches: PROCEDURE
    lb_c = 1
    IF (PEEK(SC_HOSTS + lb_slot * HOST_NAME_LEN) AND 255) = 0 THEN
        lb_c = 0
        RETURN
    END IF
    FOR lb_j = 0 TO LEN_LOBBY_HOST - 1
        lb_i = PEEK(SC_HOSTS + lb_slot * HOST_NAME_LEN + lb_j) AND 255
        IF lb_i >= 97 AND lb_i <= 122 THEN lb_i = lb_i - 32
        IF lb_i <> (PEEK(VARPTR lit_lobby_host(0) + lb_j) AND 255) THEN lb_c = 0
    NEXT lb_j
    IF (PEEK(SC_HOSTS + lb_slot * HOST_NAME_LEN + LEN_LOBBY_HOST) AND 255) <> 0 THEN lb_c = 0
END
