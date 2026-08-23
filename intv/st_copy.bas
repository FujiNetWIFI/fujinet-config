' st_copy.bas -- the file browser's filter (keypad 4) and copy (keypad 5)
' actions, plus the footer text that advertises them. Ported from
' fujinet-config's src/select_file.c (SF_FILTER / SF_COPY),
' src/destination_host_slot.c and src/perform_copy.c.
'
' Lives in the $D000 segment (see config.bas) purely for space: the default
' $5000-$6FFF segment is essentially full, so st_file.bas keeps only the two
' GOSUBs and everything with screen text in it lands here. Nothing in this
' file is called from anywhere but st_file.bas.
'
' COPY, end to end:
'   1. keypad 5 on a highlighted FILE  -> sf_do_copy records SC_SRC (the
'      file's full path) and copy_host_slot, sets copy_mode, and bounces to
'      ST_HOSTS, which renders itself as "COPY TO HOST".
'   2. picking a host slot there mounts it and re-enters ST_SELECT_FILE at
'      that host's root -- st_hosts.bas's ordinary BTN path already does
'      exactly this, so there is no separate destination-host screen the way
'      src/destination_host_slot.c is one.
'   3. keypad 5 again, anywhere in the destination tree -> cp_perform builds
'      the "source|dest" copySpec and issues FUJICMD_COPY_FILE, then puts
'      the browser back where the copy started.
'
' There is no confirmation step, matching every other platform's CONFIG.

    DIM cp_i, cp_len, cp_dlen, cp_base, cp_ok, cp_cfg
    DIM #cp_total

' ---------------------------------------------------------------------------
' sf_draw_hint: paint row 11, the file browser's one-line footer. Four
' mutually exclusive states, most urgent first. 20 columns is the whole
' budget, hence the crushed-together key list in the ordinary case.
'
' A filter set during a destination browse is deliberately not shown -- the
' "copy here" affordance is the more important thing on screen at that
' moment, and the filter is still visible in the listing itself.
' ---------------------------------------------------------------------------
sf_draw_hint: PROCEDURE
    IF copy_mode = 1 THEN
        PRINT AT screenpos(0,11) COLOR COL_DIM,"5=COPY HERE  CLR=CAN"
        GOSUB sf_hint_keys
        RETURN
    END IF

    IF num_rows = 0 THEN
        ' An empty page is far more often the filter's doing than a genuinely
        ' empty directory, so point at the filter key rather than just "back".
        PRINT AT screenpos(0,11) COLOR COL_DIM,"<EMPTY> 4=FILTER CLR"
        GOSUB sf_hint_keys
        RETURN
    END IF

    ' NOT digit-highlighted: the filter is arbitrary user text, and a pattern
    ' like "*.b1n" would come out with a stray yellow character.
    IF (PEEK(SC_FILTER) AND 255) <> 0 THEN
        PRINT AT screenpos(0,11) COLOR COL_DIM,"F:"
        s_row = 11 : s_col = 2 : s_max = 18 : s_col_color = COL_VALUE
        #s_src = SC_FILTER : GOSUB scr_puts
        RETURN
    END IF

    PRINT AT screenpos(0,11) COLOR COL_DIM,"1UP2PV3NX4FL5CP CLR "
    GOSUB sf_hint_keys
END

' sf_hint_keys: yellow keys, blue labels, on the footer just drawn.
sf_hint_keys: PROCEDURE
    s_row = 11 : s_col_color = COL_HILIGHT
    GOSUB scr_hilite_digits
END

' ---------------------------------------------------------------------------
' sf_do_filter: keypad 4. Edits SC_FILTER through the character grid.
'
' The grid gets a scratch COPY (SC_EDIT) rather than SC_FILTER itself:
' grid_entry mutates #ge_dst live whether the user eventually accepts or
' cancels (input.bas's contract), so aiming it at the real buffer would make
' ESC destructive. Same reasoning, same shape as hosts_edit_selected.
'
' Redisplay is unconditional -- grid_entry owns the whole screen while it
' runs, so even a cancel has to repaint the listing.
' ---------------------------------------------------------------------------
sf_do_filter: PROCEDURE
    FOR cp_i = 0 TO FILTER_LEN - 1
        POKE (SC_EDIT + cp_i), PEEK(SC_FILTER + cp_i) AND 255
    NEXT cp_i

    GOSUB scr_clear
    PRINT AT screenpos(0,0) COLOR COL_NORMAL,"ENTER FILTER"
    PRINT AT screenpos(0,11) COLOR COL_DIM,"!NAME=SEARCH SUBDIRS"

    #ge_dst = SC_EDIT : g_max = FILTER_LEN
    GOSUB grid_entry

    IF fn_ok = 1 THEN
        FOR cp_i = 0 TO FILTER_LEN - 1
            POKE (SC_FILTER + cp_i), PEEK(SC_EDIT + cp_i) AND 255
        NEXT cp_i
    END IF

    ' A new pattern renumbers every entry, so the page stack and position
    ' are meaningless -- start over at the top of the filtered listing.
    #dp_start = 0
    pstk_depth = 0
    sel_row = 0
    sf_sub = SF_DISPLAY
END

' ---------------------------------------------------------------------------
' sf_do_copy: keypad 5. Marks the highlighted file as a copy source, or --
' if a copy is already in flight -- performs it into the current directory.
' ---------------------------------------------------------------------------
sf_do_copy: PROCEDURE
    IF copy_mode = 1 THEN
        GOSUB cp_perform
        RETURN
    END IF

    IF num_rows = 0 THEN RETURN

    IF (PEEK(SC_EDIR + sel_row) AND 255) = 1 THEN
        PRINT AT screenpos(0,11) COLOR COL_ERROR,"CANNOT COPY A FOLDER"
        ws_delay = 90 : GOSUB ws_pause
        GOSUB sf_draw_hint
        RETURN
    END IF

    ' The listing never caches filenames (see st_file.bas's header) -- seek
    ' back to this row's recorded position and read the full name again.
    fc_maxlen = 128
    GOSUB sf_re_read_selected
    IF fn_ok = 0 THEN RETURN

    #fn_src = SC_PATH : ls_max = 192 : GOSUB fn_strlen
    cp_len = fn_len
    #fn_src = FN_RX : ls_max = 128 : GOSUB fn_strlen

    IF cp_len + fn_len > 223 THEN
        PRINT AT screenpos(0,11) COLOR COL_ERROR,"PATH TOO LONG       "
        ws_delay = 90 : GOSUB ws_pause
        GOSUB sf_draw_hint
        RETURN
    END IF

    ' SC_SRC = SC_PATH + entry: the source file's full path. Under a "!name"
    ' recursive filter the entry can itself contain '/', which is fine here
    ' and handled again by the basename step in cp_perform.
    FOR cp_i = 0 TO cp_len - 1
        POKE (SC_SRC + cp_i), PEEK(SC_PATH + cp_i) AND 255
    NEXT cp_i
    FOR cp_i = 0 TO fn_len
        POKE (SC_SRC + cp_len + cp_i), PEEK(FN_RX + cp_i) AND 255
    NEXT cp_i

    copy_host_slot = host_slot
    copy_mode = 1
    hosts_shown = 0      ' force st_hosts.bas to redraw in its copy livery
    state = ST_HOSTS
END

' ---------------------------------------------------------------------------
' ---------------------------------------------------------------------------
' cp_copy_sibling: re-send the copySpec still sitting in FN_TX with both
' ends' extensions swapped to .cfg, so a .bin arrives at the destination
' with the memory map it needs. Sets cp_cfg = 1 if a sibling was copied.
'
' Not .bin-specific, because the firmware's own lookup isn't: diskTypeROM
' .cpp replaces the BASENAME's extension with ".cfg" for any ROM it mounts
' (.bin/.rom/.int/.itv), then retries ".CFG" for case-sensitive hosts. This
' does the same, and writes the destination lowercase either way since
' that's the spelling the firmware tries first.
'
' A miss is the normal case, not a failure -- most .rom files have no
' sibling at all, and a .bin without one still boots off the emulator's
' size-guess table. So a NAK here never touches cp_ok.
'
' Patching in place only works because every extension involved is exactly
' 4 characters, which keeps #cp_total (and therefore #fn_txlen, which
' fn_transact re-reads) correct. Anything else is skipped rather than
' rebuilt -- none of the Intellivision ROM extensions are affected.
' ---------------------------------------------------------------------------
cp_copy_sibling: PROCEDURE
    IF cp_len < 5 THEN RETURN
    IF cp_len - 4 <= cp_base THEN RETURN            ' ".bin" with no basename
    IF (PEEK(SC_SRC + cp_len - 4) AND 255) <> 46 THEN RETURN   ' no 4-char ext

    ' destination always lowercase; source lowercase on the first attempt
    POKE (FN_TX + #cp_total - 3), 99  : POKE (FN_TX + cp_len - 3), 99   ' c
    POKE (FN_TX + #cp_total - 2), 102 : POKE (FN_TX + cp_len - 2), 102  ' f
    POKE (FN_TX + #cp_total - 1), 103 : POKE (FN_TX + cp_len - 1), 103  ' g

    fc_hs = copy_host_slot + 1
    fc_ds = host_slot + 1
    GOSUB fj_copy_file
    IF fn_ok = 1 THEN
        cp_cfg = 1
        RETURN
    END IF

    ' retry the SOURCE as .CFG -- a case-sensitive host may spell it that way
    POKE (FN_TX + cp_len - 3), 67    ' C
    POKE (FN_TX + cp_len - 2), 70    ' F
    POKE (FN_TX + cp_len - 1), 71    ' G
    GOSUB fj_copy_file
    IF fn_ok = 1 THEN cp_cfg = 1
END

' ---------------------------------------------------------------------------
' cp_perform: issue the copy into the directory currently being browsed.
'
' The copySpec is "sourcefullpath|destfullpath", staged straight into FN_TX
' the way fj_open_directory stages its path -- no scratch buffer, and no
' terminating NUL (fj_copy_file's comment explains why that matters).
'
' The destination is SC_PATH (which always ends in '/') plus only the
' BASENAME of the source. That last part is what stops a "!name" recursive
' hit like "roms/a/game.bin" from being asked for as a nested path the
' destination host has no directories for -- same fix as
' src/perform_copy.c's strrchr().
' ---------------------------------------------------------------------------
cp_perform: PROCEDURE
    #fn_src = SC_SRC : ls_max = 224 : GOSUB fn_strlen
    cp_len = fn_len

    cp_base = 0
    FOR cp_i = 0 TO cp_len - 1
        IF (PEEK(SC_SRC + cp_i) AND 255) = 47 THEN cp_base = cp_i + 1
    NEXT cp_i

    #fn_src = SC_PATH : ls_max = 192 : GOSUB fn_strlen
    cp_dlen = fn_len

    ' cp_base is >= 1 (SC_SRC always begins '/') and < cp_len (a folder can
    ' never be marked), so every fn_putstr below gets a non-zero length --
    ' its FOR loop would run 256 times on a length of 0.
    #cp_total = cp_len + 1 + cp_dlen + cp_len - cp_base
    IF (#cp_total > 256) OR (cp_base >= cp_len) OR (cp_dlen = 0) THEN
        PRINT AT screenpos(0,11) COLOR COL_ERROR,"PATH TOO LONG       "
        ws_delay = 120 : GOSUB ws_pause
        GOSUB sf_draw_hint
        RETURN
    END IF

    PRINT AT screenpos(0,11) COLOR COL_DIM,"COPYING...          "

    #fn_txlen = 0
    #fn_src = SC_SRC : fn_len = cp_len : GOSUB fn_putstr
    POKE (FN_TX + #fn_txlen), 124            ' '|'
    #fn_txlen = #fn_txlen + 1
    #fn_src = SC_PATH : fn_len = cp_dlen : GOSUB fn_putstr
    #fn_src = SC_SRC + cp_base : fn_len = cp_len - cp_base : GOSUB fn_putstr

    ' COPY_FILE numbers host slots 1-8, not 0-7 -- see fj_copy_file.
    fc_hs = copy_host_slot + 1
    fc_ds = host_slot + 1
    GOSUB fj_copy_file
    cp_ok = fn_ok

    cp_cfg = 0
    IF cp_ok = 1 THEN GOSUB cp_copy_sibling

    IF cp_ok = 0 THEN
        PRINT AT screenpos(0,11) COLOR COL_ERROR,"COPY FAILED         "
    ELSE
        IF cp_cfg = 1 THEN
            PRINT AT screenpos(0,11) COLOR COL_NORMAL,"COPIED + CFG        "
        ELSE
            PRINT AT screenpos(0,11) COLOR COL_NORMAL,"COPIED              "
        END IF
    END IF
    ws_delay = 120 : GOSUB ws_pause

    ' Back to where the copy started: the source host, the source directory
    ' (SC_SRC truncated after its last '/'), unfiltered. src/select_file.c's
    ' backFromCopy path does the same, filter included -- whatever pattern
    ' was in effect belonged to the destination browse, not this one.
    copy_mode = 0
    host_slot = copy_host_slot
    ' SC_SRC is 224 bytes but SC_PATH only 192, and a deep "!name" recursive
    ' hit can put more directory in front of the filename than SC_PATH holds
    ' -- land on the host's root rather than overrun it into SC_ENTRY.
    IF cp_base > 191 THEN
        POKE (SC_PATH), 47
        POKE (SC_PATH + 1), 0
    ELSE
        FOR cp_i = 0 TO cp_base - 1
            POKE (SC_PATH + cp_i), PEEK(SC_SRC + cp_i) AND 255
        NEXT cp_i
        POKE (SC_PATH + cp_base), 0
    END IF
    POKE (SC_FILTER), 0

    ' sf_display remounts the host on every page draw, so no explicit
    ' fj_mount_host is needed here.
    #dp_start = 0
    pstk_depth = 0
    sel_row = 0
    sf_sub = SF_DISPLAY
    state = ST_SELECT_FILE
END
