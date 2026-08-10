' st_file.bas -- ST_SELECT_FILE: browse the mounted host's directory tree,
' with .cfg-sibling suppression, subfolder advance/devance, prev/next
' paging, and bounce-scrolling of the highlighted long filename. Choosing a
' non-directory entry builds its full path into SC_BOOTPATH and hands off
' to ST_BOOT.
'
' Nothing here ever caches a filename across frames -- only its length
' (SC_ELEN), directory flag (SC_EDIR) and absolute directory position
' (SC_EPOS) survive a page draw. Anywhere the real name is needed again
' (advancing into a folder, picking a file to boot, bounce-scrolling) it is
' re-fetched by seeking back to that row's recorded position. See the plan
' doc's ".cfg" and long-filename-scrolling sections for the full rationale.

    DIM sf_i, sf_row, sf_isdir, sf_hstart
    DIM sf_ch1, sf_ch2, sf_ch3, sf_ch4
    DIM #sf_abs, #dp_next

do_select_file: PROCEDURE
    IF sf_sub = SF_INIT THEN GOSUB sf_init
    IF sf_sub = SF_DISPLAY THEN GOSUB sf_display
    IF sf_sub = SF_CHOOSE THEN GOSUB sf_choose
END

sf_init: PROCEDURE
    sel_row = 0
    sf_sub = SF_DISPLAY
END

' ---------------------------------------------------------------------------
' sf_display: (re)open the directory at #dp_start, read up to
' ENTRIES_PER_PAGE non-.cfg entries, draw them, and drop into SF_CHOOSE.
' Reads run until ENTRIES_PER_PAGE entries have been DISPLAYED or EOF --
' not until ENTRIES_PER_PAGE have been READ -- since a directory full of
' .bin+.cfg pairs consumes roughly twice as many reads as displayed rows.
' ---------------------------------------------------------------------------
sf_display: PROCEDURE
    GOSUB scr_clear

    #fn_src = SC_PATH : ls_max = 192 : GOSUB fn_strlen
    sf_hstart = 0
    IF fn_len > SCREEN_COLS THEN sf_hstart = fn_len - SCREEN_COLS
    s_row = 0 : s_col = 0 : s_max = SCREEN_COLS : s_col_color = COL_DIM
    #s_src = SC_PATH + sf_hstart : GOSUB scr_puts

    PRINT AT screenpos(0,11) COLOR COL_DIM,"LOADING...          "

    fc_hs = host_slot : GOSUB fj_mount_host
    IF fn_ok = 0 THEN
        PRINT AT screenpos(0,11) COLOR COL_ERROR,"MOUNT ERROR CLR=BACK"
        num_rows = 0 : sf_sub = SF_CHOOSE : RETURN
    END IF

    #fn_src = SC_PATH : GOSUB fj_open_directory
    IF fn_ok = 0 THEN
        PRINT AT screenpos(0,11) COLOR COL_ERROR,"DIR ERROR   CLR=BACK"
        num_rows = 0 : sf_sub = SF_CHOOSE : RETURN
    END IF

    IF #dp_start > 0 THEN
        #fc_pos = #dp_start
        GOSUB fj_set_directory_position
    END IF

    #sf_abs = #dp_start
    num_rows = 0
    dir_eof = 0

    WHILE num_rows < ENTRIES_PER_PAGE AND dir_eof = 0
        fc_maxlen = DIR_MAX_LEN : fc_addtl = 0
        GOSUB fj_read_dir_entry
        IF fn_ok = 0 THEN
            dir_eof = 1
        ELSE
            GOSUB fj_dir_entry_is_eof
            IF fc_c = 1 THEN
                dir_eof = 1
            ELSE
                #fn_src = FN_RX : ls_max = DIR_MAX_LEN : GOSUB fn_strlen
                #sf_abs = #sf_abs + 1
                GOSUB sf_is_cfg
                IF fc_c = 0 THEN
                    sf_row = FILES_START_ROW + num_rows
                    POKE (SC_EPOS + num_rows * 2), (#sf_abs - 1) AND 255
                    POKE (SC_EPOS + num_rows * 2 + 1), ((#sf_abs - 1) / 256) AND 255
                    sf_isdir = 0
                    IF fn_len > 0 THEN
                        IF (PEEK(FN_RX + fn_len - 1) AND 255) = 47 THEN sf_isdir = 1
                    END IF
                    POKE (SC_EDIR + num_rows), sf_isdir
                    POKE (SC_ELEN + num_rows), fn_len
                    s_row = sf_row : s_col = 1 : s_max = SCREEN_COLS - 1 : s_col_color = COL_NORMAL
                    #s_src = FN_RX : GOSUB scr_puts
                    num_rows = num_rows + 1
                END IF
            END IF
        END IF
    WEND

    #dp_next = #sf_abs
    GOSUB fj_close_directory

    FOR sf_i = num_rows TO ENTRIES_PER_PAGE - 1
        s_row = FILES_START_ROW + sf_i : GOSUB scr_row_clear
    NEXT sf_i

    IF num_rows = 0 AND pstk_depth > 0 THEN
        ' overshot past the end via NEXT -- undo and redisplay
        GOSUB sf_pop_pstk
        sf_sub = SF_DISPLAY
        RETURN
    END IF

    IF num_rows = 0 THEN
        PRINT AT screenpos(0,11) COLOR COL_DIM,"<EMPTY>     CLR=BACK"
    ELSE
        PRINT AT screenpos(0,11) COLOR COL_DIM,"1=UP 2=PRV 3=NXT CLR"
        sel_row = 0
        s_row = FILES_START_ROW : s_col = 1 : s_max = SCREEN_COLS - 1 : s_col_color = COL_HILIGHT
        GOSUB scr_recolor
        sc_row = FILES_START_ROW : sc_col = 1 : sc_max = SCREEN_COLS - 1 : sc_color = COL_HILIGHT
        sc_active = 0 : sc_idle = 0
    END IF

    sf_sub = SF_CHOOSE
END

' ---------------------------------------------------------------------------
' sf_is_cfg: sets fc_c = 1 iff the entry currently in FN_RX (length fn_len,
' as just computed by the caller) ends in ".cfg", case-insensitively. This
' is the client-side half of hiding .cfg siblings -- the firmware's
' wildcard filter can't express the "not" needed to do it server-side; see
' the plan doc.
' ---------------------------------------------------------------------------
sf_is_cfg: PROCEDURE
    fc_c = 0
    IF fn_len >= 4 THEN
        sf_ch1 = PEEK(FN_RX + fn_len - 4) AND 255 : GOSUB sf_lower1
        sf_ch2 = PEEK(FN_RX + fn_len - 3) AND 255 : GOSUB sf_lower2
        sf_ch3 = PEEK(FN_RX + fn_len - 2) AND 255 : GOSUB sf_lower3
        sf_ch4 = PEEK(FN_RX + fn_len - 1) AND 255 : GOSUB sf_lower4
        IF sf_ch1 = 46 AND sf_ch2 = 99 AND sf_ch3 = 102 AND sf_ch4 = 103 THEN fc_c = 1
    END IF
END
sf_lower1: PROCEDURE
    IF sf_ch1 >= 65 AND sf_ch1 <= 90 THEN sf_ch1 = sf_ch1 + 32
END
sf_lower2: PROCEDURE
    IF sf_ch2 >= 65 AND sf_ch2 <= 90 THEN sf_ch2 = sf_ch2 + 32
END
sf_lower3: PROCEDURE
    IF sf_ch3 >= 65 AND sf_ch3 <= 90 THEN sf_ch3 = sf_ch3 + 32
END
sf_lower4: PROCEDURE
    IF sf_ch4 >= 65 AND sf_ch4 <= 90 THEN sf_ch4 = sf_ch4 + 32
END

' ---------------------------------------------------------------------------
' sf_choose: interactive paging/selection loop.
' ---------------------------------------------------------------------------
sf_choose: PROCEDURE
    GOSUB in_poll

    IF in_disc = DISC_UP AND sel_row > 0 THEN GOSUB sf_move_up
    IF in_disc = DISC_DOWN AND sel_row < num_rows - 1 THEN GOSUB sf_move_down

    IF in_key = KEYPAD_1 THEN GOSUB sf_start_devance : RETURN
    IF in_key = KEYPAD_2 THEN GOSUB sf_start_prev : RETURN
    IF in_key = KEYPAD_3 THEN GOSUB sf_start_next : RETURN
    IF in_key = KEYPAD_CLEAR THEN
        state = ST_HOSTS
        RETURN
    END IF

    IF in_btn <> 0 AND num_rows > 0 THEN
        GOSUB sf_choose_entry
        RETURN
    END IF

    IF num_rows > 0 THEN GOSUB scroll_step
END

sf_move_up: PROCEDURE
    sc_row = FILES_START_ROW + sel_row : sc_col = 1 : sc_max = SCREEN_COLS - 1 : sc_color = COL_NORMAL
    GOSUB scroll_reset
    s_row = sc_row : s_col = 1 : s_max = SCREEN_COLS - 1 : s_col_color = COL_NORMAL
    GOSUB scr_recolor

    sel_row = sel_row - 1

    s_row = FILES_START_ROW + sel_row : s_col = 1 : s_max = SCREEN_COLS - 1 : s_col_color = COL_HILIGHT
    GOSUB scr_recolor
    sc_row = FILES_START_ROW + sel_row : sc_col = 1 : sc_max = SCREEN_COLS - 1 : sc_color = COL_HILIGHT
END

sf_move_down: PROCEDURE
    sc_row = FILES_START_ROW + sel_row : sc_col = 1 : sc_max = SCREEN_COLS - 1 : sc_color = COL_NORMAL
    GOSUB scroll_reset
    s_row = sc_row : s_col = 1 : s_max = SCREEN_COLS - 1 : s_col_color = COL_NORMAL
    GOSUB scr_recolor

    sel_row = sel_row + 1

    s_row = FILES_START_ROW + sel_row : s_col = 1 : s_max = SCREEN_COLS - 1 : s_col_color = COL_HILIGHT
    GOSUB scr_recolor
    sc_row = FILES_START_ROW + sel_row : sc_col = 1 : sc_max = SCREEN_COLS - 1 : sc_color = COL_HILIGHT
END

' ---------------------------------------------------------------------------
' sf_start_devance: keypad 1, "go up a level". SC_PATH always ends in '/'
' below the root (directories are stored with the firmware's trailing
' slash appended on advance), so truncate at the previous '/'.
' ---------------------------------------------------------------------------
sf_start_devance: PROCEDURE
    #fn_src = SC_PATH : ls_max = 192 : GOSUB fn_strlen
    IF fn_len <= 1 THEN RETURN   ' already at root "/"

    sf_i = fn_len - 2
    WHILE sf_i > 0 AND (PEEK(SC_PATH + sf_i) AND 255) <> 47
        sf_i = sf_i - 1
    WEND
    POKE (SC_PATH + sf_i + 1), 0

    #dp_start = 0
    pstk_depth = 0
    sf_sub = SF_DISPLAY
END

sf_start_prev: PROCEDURE
    IF pstk_depth > 0 THEN
        GOSUB sf_pop_pstk
        sf_sub = SF_DISPLAY
    END IF
END

sf_start_next: PROCEDURE
    IF dir_eof = 1 AND num_rows < ENTRIES_PER_PAGE THEN RETURN  ' known EOF, nothing more
    GOSUB sf_push_pstk
    #dp_start = #dp_next
    sf_sub = SF_DISPLAY
END

sf_push_pstk: PROCEDURE
    IF pstk_depth < 10 THEN
        POKE (SC_PSTK + pstk_depth * 2), #dp_start AND 255
        POKE (SC_PSTK + pstk_depth * 2 + 1), (#dp_start / 256) AND 255
        pstk_depth = pstk_depth + 1
    END IF
END

sf_pop_pstk: PROCEDURE
    pstk_depth = pstk_depth - 1
    #dp_start = (PEEK(SC_PSTK + pstk_depth * 2) AND 255) + (PEEK(SC_PSTK + pstk_depth * 2 + 1) AND 255) * 256
END

sf_choose_entry: PROCEDURE
    sf_isdir = PEEK(SC_EDIR + sel_row) AND 255
    IF sf_isdir = 1 THEN
        GOSUB sf_advance
    ELSE
        GOSUB sf_pick_boot
    END IF
END

' ---------------------------------------------------------------------------
' sf_re_read_selected: shared by sf_advance/sf_pick_boot/sf_get_long_filename
' -- seek back to the selected row's recorded absolute position and read
' its full entry (up to fc_maxlen, caller-set) into FN_RX. Leaves fn_ok set.
' ---------------------------------------------------------------------------
sf_re_read_selected: PROCEDURE
    fc_hs = host_slot : GOSUB fj_mount_host
    IF fn_ok = 0 THEN RETURN
    #fn_src = SC_PATH : GOSUB fj_open_directory
    IF fn_ok = 0 THEN RETURN

    #fc_pos = (PEEK(SC_EPOS + sel_row * 2) AND 255) + (PEEK(SC_EPOS + sel_row * 2 + 1) AND 255) * 256
    GOSUB fj_set_directory_position

    fc_addtl = 0
    GOSUB fj_read_dir_entry
    GOSUB fj_close_directory
END

sf_advance: PROCEDURE
    fc_maxlen = 128
    GOSUB sf_re_read_selected
    IF fn_ok = 0 THEN RETURN

    #fn_src = SC_PATH : ls_max = 192 : GOSUB fn_strlen
    sf_hstart = fn_len
    #fn_src = FN_RX : ls_max = 128 : GOSUB fn_strlen
    IF sf_hstart + fn_len < 190 THEN
        FOR sf_i = 0 TO fn_len
            POKE (SC_PATH + sf_hstart + sf_i), PEEK(FN_RX + sf_i) AND 255
        NEXT sf_i
    END IF

    #dp_start = 0
    pstk_depth = 0
    sf_sub = SF_DISPLAY
END

sf_pick_boot: PROCEDURE
    fc_maxlen = 128
    GOSUB sf_re_read_selected
    IF fn_ok = 0 THEN RETURN

    #fn_src = SC_PATH : ls_max = 192 : GOSUB fn_strlen
    FOR sf_i = 0 TO fn_len - 1
        POKE (SC_BOOTPATH + sf_i), PEEK(SC_PATH + sf_i) AND 255
    NEXT sf_i
    sf_hstart = fn_len

    #fn_src = FN_RX : ls_max = 128 : GOSUB fn_strlen
    FOR sf_i = 0 TO fn_len
        POKE (SC_BOOTPATH + sf_hstart + sf_i), PEEK(FN_RX + sf_i) AND 255
    NEXT sf_i

    sf_sub = SF_DONE
    state = ST_BOOT
END

' ---------------------------------------------------------------------------
' sf_get_long_filename: GOSUB'd by scroll.bas after IDLE_FRAMES of no input
' on the highlighted row. Re-fetches the full (up to 128-byte) name for
' sel_row into SC_ENTRY and sets sc_len -- bypasses the 36-byte truncation
' the page listing itself gets.
' ---------------------------------------------------------------------------
sf_get_long_filename: PROCEDURE
    fc_maxlen = 128
    GOSUB sf_re_read_selected
    IF fn_ok = 0 THEN
        sc_len = 0
        RETURN
    END IF
    #fn_src = FN_RX : ls_max = 128 : GOSUB fn_strlen
    FOR sf_i = 0 TO fn_len
        POKE (SC_ENTRY + sf_i), PEEK(FN_RX + sf_i) AND 255
    NEXT sf_i
    sc_len = fn_len
END
