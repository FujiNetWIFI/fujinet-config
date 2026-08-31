' st_hosts.bas -- ST_HOSTS: list the 8 host slots, select one to mount and
' browse (-> ST_SELECT_FILE), edit a slot's name via the character grid, or
' jump to the info screen.
'
' Drawn in foreground/background mode -- the one screen that is. See fgbg.bas
' for why (a tan selection bar inside the dark green list needs a fifth
' background colour, one more than the color stack can hold) and for the
' uppercase-only restriction that comes with it.
'
' Keys: disc/1-8 move the selection bar, BTN mounts+browses the selected slot,
' ENTER edits its name, 9 goes to the info screen, 0 mounts and boots the
' FujiNet Game Lobby ROM from ec.tnfs.io (st_lobby.bas). (Not CLEAR for
' edit -- CLEAR is free for a future "clear slot" action, and there's no
' need for a "back" key here since this is the program's home screen.)
'
' This screen doubles as the copy destination picker (src/config's separate
' DESTINATION_HOST_SLOT state): when st_copy.bas sets copy_mode it re-enters
' here, and the only differences are cosmetic plus a narrower key set --
' BTN's mount-and-browse path is already exactly what a destination browse
' needs. CLEAR is bound only in that mode, to cancel the copy.

    DIM hosts_shown, hd_i, hd_row, hd_col, hd_color, hd_new

do_hosts: PROCEDURE
    IF hosts_shown = 0 THEN
        GOSUB fj_read_host_slots
        sel_row = 0
        GOSUB scr_clear
        ' Drawn in FGBG mode (config.bas's main loop has already switched):
        ' text first, then scr_fgbg_row stamps that row's background across all
        ' 20 columns. Both recolor helpers mask with AND $FFF8 and add, so they
        ' have to run BEFORE the background goes in or the add carries into it.
        IF copy_mode = 1 THEN
            PRINT AT screenpos(0,0) COLOR COL_NORMAL,"COPY TO HOST"
            s_row = 0 : #s_bg = BG_BLUE : GOSUB scr_fgbg_row
            GOSUB hosts_draw_list
            s_row = 9 : #s_bg = BG_DARKGREEN : GOSUB scr_fgbg_row
            ' no lobby line mid-copy -- row 10 just extends the list block
            s_row = 10 : #s_bg = BG_DARKGREEN : GOSUB scr_fgbg_row
            PRINT AT screenpos(0,11) COLOR COL_NORMAL,"BTN=DEST  CLR=CANCEL"
            s_col_color = COL_HILIGHT : s_row = 11
            s_col = 0  : s_max = 3 : GOSUB scr_recolor     ' BTN
            s_col = 10 : s_max = 3 : GOSUB scr_recolor     ' CLR
            #s_bg = BG_PURPLE : GOSUB scr_fgbg_row
        ELSE
            PRINT AT screenpos(0,0) COLOR COL_NORMAL,"HOST SLOTS"
            s_row = 0 : #s_bg = BG_BLUE : GOSUB scr_fgbg_row
            GOSUB hosts_draw_list
            s_row = 9 : #s_bg = BG_DARKGREEN : GOSUB scr_fgbg_row

            PRINT AT screenpos(0,10) COLOR COL_NORMAL,"0=PLAY GAME LOBBY"
            s_col_color = COL_HILIGHT
            s_row = 10 : GOSUB scr_hilite_digits           ' the 0
            #s_bg = BG_RED : GOSUB scr_fgbg_row

            ' keys yellow, labels white -- same rule as the file browser's
            ' footer (st_copy.bas's sf_hint_keys), but BTN and ENT aren't
            ' digits, so scr_hilite_digits can't find them.
            PRINT AT screenpos(0,11) COLOR COL_NORMAL,"BTN=OPEN ENT=EDT 9=I"
            s_col_color = COL_HILIGHT : s_row = 11
            s_col = 0  : s_max = 3 : GOSUB scr_recolor     ' BTN
            s_col = 9  : s_max = 3 : GOSUB scr_recolor     ' ENT
            s_col = 17 : s_max = 1 : GOSUB scr_recolor     ' 9
            #s_bg = BG_PURPLE : GOSUB scr_fgbg_row
        END IF
        hosts_shown = 1
    END IF

    GOSUB in_poll

    ' hosts_set_sel (selbar.bas) owns sel_row and recolours only the two rows
    ' that change -- do NOT assign sel_row here, and do NOT call
    ' hosts_draw_list, which repaints all eight rows and takes several frames.
    IF in_disc = DISC_UP AND sel_row > 0 THEN
        hd_new = sel_row - 1
        GOSUB hosts_set_sel
    END IF
    IF in_disc = DISC_DOWN AND sel_row < NUM_HOST_SLOTS - 1 THEN
        hd_new = sel_row + 1
        GOSUB hosts_set_sel
    END IF
    IF in_key >= KEYPAD_1 AND in_key <= KEYPAD_8 THEN
        hd_new = in_key - 1
        GOSUB hosts_set_sel
    END IF

    ' Lobby, info and rename are all meaningless mid-copy -- the only
    ' choices there are "which host" and "cancel".
    IF copy_mode = 1 THEN
        IF in_key = KEYPAD_CLEAR THEN
            copy_mode = 0
            hosts_shown = 0    ' redraw as the ordinary host list
            RETURN
        END IF
    ELSE
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
    END IF

    IF in_btn <> 0 THEN
        host_slot = sel_row
        fc_hs = host_slot
        GOSUB fj_mount_host
        IF fn_ok = 0 THEN
            PRINT AT screenpos(0,11) COLOR COL_ERROR,"MOUNT FAILED        "
            s_row = 11 : #s_bg = BG_PURPLE : GOSUB scr_fgbg_row
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
' hosts_draw_list: rows 1..8, one per host slot. Column 0 is the slot number,
' column 1 blank, columns 2..19 the hostname (or "<EMPTY>").
' SC_HOSTS is the verbatim 8x32 READ_HOST_SLOTS mirror populated by
' fj_read_host_slots -- an empty slot is a NUL byte at its first offset.
'
' Selection is the whole row going black-on-tan rather than the '>' cursor it
' used to be, which is what freed column 0 and bought the hostname field its
' 18th character.
'
' This is the FULL draw and runs once, on entry. It is NOT how the bar moves:
' at ~40 BACKTAB cell operations per row it costs the better part of seven
' frames, and #BACKTAB is the live display list, so calling it on every cursor
' move had the list visibly repainting in pieces. (The names coming from
' SC_HOSTS scratch RAM rather than the mailbox is beside the point -- the cost
' is the cell writes.) hosts_set_sel in selbar.bas moves the bar instead, by
' recolouring only the two rows that changed.
'
' Each row's text must be final before scr_fgbg_row stamps its background, and
' that stamp must happen exactly once per row -- see fgbg.bas.
' ---------------------------------------------------------------------------
hosts_draw_list: PROCEDURE
    FOR hd_i = 0 TO NUM_HOST_SLOTS - 1
        hd_row = 1 + hd_i
        hd_color = COL_NORMAL : #s_bg = BG_DARKGREEN
        IF hd_i = sel_row THEN hd_color = CS_BLACK : #s_bg = BG_TAN

        hd_col = 48 + hd_i + 1          ' slot number, 1-8
        #BACKTAB(hd_row * SCREEN_COLS + 0) = (hd_col - 32) * 8 + hd_color
        #BACKTAB(hd_row * SCREEN_COLS + 1) = hd_color   ' card 0 = space

        IF (PEEK(SC_HOSTS + hd_i * HOST_NAME_LEN) AND 255) = 0 THEN
            PRINT AT screenpos(2, hd_row) COLOR hd_color,"<EMPTY>           "
        ELSE
            s_row = hd_row : s_col = 2 : s_max = 18 : s_col_color = hd_color
            #s_src = SC_HOSTS + hd_i * HOST_NAME_LEN
            GOSUB scr_puts
        END IF

        ' #s_bg survives scr_puts untouched, so no second variable is needed.
        s_row = hd_row : GOSUB scr_fgbg_row
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

    ' The grid runs its own loop inline, with state still ST_HOSTS, so
    ' config.bas's mode switch never sees it -- and the 6x16 charset it offers
    ' is half lowercase, which FGBG cannot draw. grid_video swaps to the grid's
    ' own color stack palette and leaves vid_now at a sentinel, so the main
    ' loop restores FGBG on return (hosts_shown = 0 below forces the redraw
    ' that goes with it).
    GOSUB grid_video

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
