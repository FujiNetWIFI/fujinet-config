' st_boot.bas -- ST_BOOT: SET_DEVICE_FULLPATH + MOUNT_IMAGE for the file
' chosen in st_file.bas (its full path already sitting in SC_BOOTPATH).
' MOUNT_IMAGE triggers the ESP32's MediaTypeROM dispatch, which pushes the
' file (and its .cfg sibling, if any) to the RP2040 and resets the
' console -- on success, this transaction never actually returns; the
' console reboots into the game. It only returns on failure or timeout.
'
' A plain fn_transact() would time out after the standard 900 frames
' (15s), which a real ROM+.cfg transfer over TNFS can plausibly exceed.
' So MOUNT_IMAGE is issued with a hand-rolled version of fn_transact here,
' with a much longer timeout, that also polls FN_BOOT_PCT (the RP2040
' updates it directly, out-of-band from the ACKSEQ handshake, while the
' transfer is in flight) to drive a progress bar.

    CONST BOOT_TIMEOUT_FRAMES = 3600   ' ~60s at 60Hz

    DIM bt_hstart, bt_t, bt_seq, bt_pct, bt_lastpct, bt_isboot

do_boot: PROCEDURE
    bt_isboot = 0
    GOSUB scr_clear
    PRINT AT screenpos(0,0) COLOR COL_NORMAL,"BOOTING"

    #fn_src = SC_BOOTPATH : ls_max = 255 : GOSUB fn_strlen
    bt_hstart = 0
    IF fn_len > SCREEN_COLS THEN bt_hstart = fn_len - SCREEN_COLS
    s_row = 2 : s_col = 0 : s_max = SCREEN_COLS : s_col_color = COL_VALUE
    #s_src = SC_BOOTPATH + bt_hstart : GOSUB scr_puts

    PRINT AT screenpos(0,11) COLOR COL_DIM,"DO NOT POWER OFF"

    fc_ds = DEVICE_SLOT : fc_hs = host_slot : fc_mode = MODE_READ
    #fn_src = SC_BOOTPATH
    GOSUB fj_set_device_fullpath
    IF fn_ok = 0 THEN
        GOSUB boot_fail
        RETURN
    END IF

    GOSUB boot_mount_with_progress

    ' Only reached on failure/timeout -- success resets the console.
    GOSUB boot_fail
END

' ---------------------------------------------------------------------------
' boot_mount_with_progress: MOUNT_IMAGE(DEVICE_SLOT, MODE_READ), no TX
' payload, with BOOT_TIMEOUT_FRAMES instead of fn_transact's 900, and a
' progress-bar redraw whenever FN_BOOT_PCT changes.
' ---------------------------------------------------------------------------
boot_mount_with_progress: PROCEDURE
    POKE (FN_DEVICE), FUJI_DEVICEID
    POKE (FN_CMD), FUJICMD_MOUNT_IMAGE
    POKE (FN_NPARAM), 2
    pm_i = 0 : pm_size = 1 : #pm_val = DEVICE_SLOT : GOSUB fn_param
    pm_i = 1 : pm_size = 1 : #pm_val = MODE_READ : GOSUB fn_param
    POKE (FN_TXLEN_LO), 0
    POKE (FN_TXLEN_HI), 0

    bt_seq = (PEEK(FN_ACKSEQ) AND 255) + 1
    IF bt_seq = 0 THEN bt_seq = 1
    POKE (FN_SEQ), bt_seq

    bt_lastpct = 255
    bt_t = 0
    WHILE ((PEEK(FN_ACKSEQ) AND 255) <> bt_seq) AND (bt_t < BOOT_TIMEOUT_FRAMES)
        WAIT
        bt_t = bt_t + 1
        bt_pct = PEEK(FN_BOOT_PCT) AND 255
        IF bt_pct <> bt_lastpct THEN
            GOSUB boot_draw_progress
            bt_lastpct = bt_pct
        END IF
    WEND

    bt_isboot = 0

    IF bt_t >= BOOT_TIMEOUT_FRAMES THEN
        fn_ok = 0
        mb_err = 0
        RETURN
    END IF

    IF (PEEK(FN_REPLY_CMD) AND 255) <> FUJICMD_ACK THEN
        fn_ok = 0
        ' FN_BOOT_STATE/ERR survive a NAK -- prefer the RP2040's boot
        ' verdict over FN_ERR (which is 0/FB_OK in exactly this case)
        IF (PEEK(FN_BOOT_STATE) AND 255) = FUJI_BOOT_FAILED THEN
            mb_err = PEEK(FN_BOOT_ERR) AND 255
            bt_isboot = 1
        ELSE
            mb_err = PEEK(FN_ERR) AND 255
        END IF
        RETURN
    END IF

    ' The MOUNT_IMAGE transaction itself ACKed (the ESP32 successfully
    ' pushed the file to the RP2040 over the DBC side-channel), but that
    ' push can still have failed the RP2040's own mapping/decode step --
    ' see FUJI_MB_BOOT_STATE/ERR in fuji_mailbox.h. A real success never
    ' reaches here at all (the console resets); reaching here with
    ' BOOT_STATE == FUJI_BOOT_FAILED means the boot itself was rejected,
    ' distinct from a transport-level NAK above. mb_err = 0xEE is jzIntv's
    ' emulator saying "the push worked, but I can't actually reboot the
    ' cart" -- that's the expected outcome on jzIntv, not a real failure;
    ' every other mb_err value (see FUJI_BOOT_ERR_* in fuji_mailbox.h) is
    ' a genuine mapping/decode failure on real hardware too.
    IF (PEEK(FN_BOOT_STATE) AND 255) = FUJI_BOOT_FAILED THEN
        fn_ok = 0
        mb_err = PEEK(FN_BOOT_ERR) AND 255
        bt_isboot = 1
        RETURN
    END IF

    fn_ok = 1
END

' boot_draw_progress: 20-cell bar on row 5, percentage on row 6.
boot_draw_progress: PROCEDURE
    s_row = 5
    FOR s_i = 0 TO SCREEN_COLS - 1
        s_c = 32
        IF s_i < (bt_pct * SCREEN_COLS) / 100 THEN s_c = 35   ' '#'
        #BACKTAB(s_row * SCREEN_COLS + s_i) = (s_c - 32) * 8 + COL_HILIGHT
    NEXT s_i

    s_row = 6 : GOSUB scr_row_clear
    #s_val = bt_pct
    PRINT AT screenpos(8,6) COLOR COL_VALUE,<.3>#s_val
    PRINT AT screenpos(11,6) COLOR COL_VALUE,"%"
END

boot_fail: PROCEDURE
    PRINT AT screenpos(0,8) COLOR COL_DIM,"ERR CODE:"
    #s_val = mb_err
    PRINT AT screenpos(10,8) COLOR COL_VALUE,<.3>#s_val
    ' named reasons mirror FUJI_BOOT_ERR_* (0xEE is jzIntv's, not this namespace)
    IF bt_isboot = 1 THEN
        IF mb_err = 1 THEN PRINT AT screenpos(0,9) COLOR COL_DIM,"BAD ROM HEADER"
        IF mb_err = 2 THEN PRINT AT screenpos(0,9) COLOR COL_DIM,"TRUNCATED XFER"
        IF mb_err = 3 THEN PRINT AT screenpos(0,9) COLOR COL_DIM,"NO MAPPING"
        IF mb_err = 4 THEN PRINT AT screenpos(0,9) COLOR COL_DIM,"JLP CONFLICT"
        IF mb_err = 5 THEN PRINT AT screenpos(0,9) COLOR COL_DIM,"BAD CFG FILE"
        IF mb_err = 6 THEN PRINT AT screenpos(0,9) COLOR COL_DIM,"RAM TOO BIG"
    END IF
    PRINT AT screenpos(0,11) COLOR COL_ERROR,"BOOT FAILED         "
    FOR bt_t = 0 TO 119
        WAIT
    NEXT bt_t
    state = ST_HOSTS
END
