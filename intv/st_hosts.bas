' st_hosts.bas -- ST_HOSTS: list the 8 host slots, select one to mount and
' browse (-> ST_SELECT_FILE), edit a slot's name via the character grid, or
' jump to the info screen.
'
' Keys: disc/1-8 move the cursor, BTN mounts+browses the selected slot,
' ENTER edits its name, 9 goes to the info screen, 0 mounts and boots the
' FujiNet Game Lobby ROM from ec.tnfs.io (st_lobby.bas). (Not CLEAR for
' edit -- CLEAR is free for a future "clear slot" action, and there's no
' need for a "back" key here since this is the program's home screen.)

    DIM hosts_shown, hd_i, hd_row, hd_col, hd_color

do_hosts: PROCEDURE
    IF hosts_shown = 0 THEN
        GOSUB fj_read_host_slots
        sel_row = 0
        GOSUB scr_clear
        PRINT AT screenpos(0,0) COLOR COL_NORMAL,"HOST SLOTS"
        GOSUB hosts_draw_list
        PRINT AT screenpos(0,10) COLOR COL_HILIGHT,"0=PLAY GAME LOBBY"
        PRINT AT screenpos(0,11) COLOR COL_DIM,"BTN=OPEN ENT=EDT 9=I"
        hosts_shown = 1
    END IF

    GOSUB in_poll

    IF in_disc = DISC_UP AND sel_row > 0 THEN
        sel_row = sel_row - 1
        GOSUB hosts_draw_list
    END IF
    IF in_disc = DISC_DOWN AND sel_row < NUM_HOST_SLOTS - 1 THEN
        sel_row = sel_row + 1
        GOSUB hosts_draw_list
    END IF
    IF in_key >= KEYPAD_1 AND in_key <= KEYPAD_8 THEN
        sel_row = in_key - 1
        GOSUB hosts_draw_list
    END IF

    IF in_key = KEYPAD_0 THEN
        GOSUB hosts_launch_lobby
        RETURN
    END IF

    IF in_key = KEYPAD_9 THEN
        hosts_shown = 0
        state = ST_INFO
        RETURN
    END IF

    IF in_key = KEYPAD_ENTER THEN
        GOSUB hosts_edit_selected
        RETURN
    END IF

    IF in_btn <> 0 THEN
        host_slot = sel_row
        fc_hs = host_slot
        GOSUB fj_mount_host
        IF fn_ok = 0 THEN
            PRINT AT screenpos(0,11) COLOR COL_ERROR,"MOUNT FAILED        "
        ELSE
            POKE (SC_PATH), 47    ' '/' -- start browsing at the host's root
            POKE (SC_PATH + 1), 0
            sf_sub = SF_INIT
            #dp_start = 0
            pstk_depth = 0
            hosts_shown = 0
            state = ST_SELECT_FILE
        END IF
    END IF
END

' ---------------------------------------------------------------------------
' hosts_draw_list: rows 2..9, one per host slot. Column 0 is the cursor,
' column 1 the slot number, columns 3..19 the hostname (or "<EMPTY>").
' SC_HOSTS is the verbatim 8x32 READ_HOST_SLOTS mirror populated by
' fj_read_host_slots -- an empty slot is a NUL byte at its first offset.
' ---------------------------------------------------------------------------
hosts_draw_list: PROCEDURE
    FOR hd_i = 0 TO NUM_HOST_SLOTS - 1
        hd_row = 2 + hd_i
        hd_color = COL_NORMAL
        IF hd_i = sel_row THEN hd_color = COL_HILIGHT

        hd_col = 32                     ' cursor: '>' if selected, else blank
        IF hd_i = sel_row THEN hd_col = 62
        #BACKTAB(hd_row * SCREEN_COLS + 0) = (hd_col - 32) * 8 + hd_color

        hd_col = 48 + hd_i + 1          ' slot number, 1-8
        #BACKTAB(hd_row * SCREEN_COLS + 1) = (hd_col - 32) * 8 + hd_color
        #BACKTAB(hd_row * SCREEN_COLS + 2) = CS_BLACK

        IF (PEEK(SC_HOSTS + hd_i * HOST_NAME_LEN) AND 255) = 0 THEN
            PRINT AT screenpos(3, hd_row) COLOR hd_color,"<EMPTY>          "
        ELSE
            s_row = hd_row : s_col = 3 : s_max = 17 : s_col_color = hd_color
            #s_src = SC_HOSTS + hd_i * HOST_NAME_LEN
            GOSUB scr_puts
        END IF
    NEXT hd_i
END

' ---------------------------------------------------------------------------
' hosts_edit_selected: grid_entry edits a scratch COPY (SC_EDIT) of the
' selected slot's name, pre-loaded so the user edits rather than retypes.
' SC_HOSTS (and the firmware's copy) is only touched on accept, so
' cancelling leaves both untouched.
' ---------------------------------------------------------------------------
hosts_edit_selected: PROCEDURE
    FOR hd_i = 0 TO HOST_NAME_LEN - 1
        POKE (SC_EDIT + hd_i), PEEK(SC_HOSTS + sel_row * HOST_NAME_LEN + hd_i) AND 255
    NEXT hd_i

    GOSUB scr_clear
    PRINT AT screenpos(0,0) COLOR COL_NORMAL,"EDIT HOST SLOT"
    #ge_dst = SC_EDIT : g_max = HOST_NAME_LEN
    GOSUB grid_entry

    IF fn_ok = 1 THEN
        FOR hd_i = 0 TO HOST_NAME_LEN - 1
            POKE (SC_HOSTS + sel_row * HOST_NAME_LEN + hd_i), PEEK(SC_EDIT + hd_i) AND 255
        NEXT hd_i
        GOSUB fj_write_host_slots
    END IF

    hosts_shown = 0   ' force a full redraw of the host list on return
END
