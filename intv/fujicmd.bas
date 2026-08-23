' fujicmd.bas -- Fuji-device (0x70) command wrappers built on fujinet.bas's
' fn_transact. Each wrapper stages mb_dev/mb_cmd/mb_nparam/#fn_txlen (and the
' TX payload, if any) and leaves the reply in FN_RX with fn_ok set.
'
' Every payload size below was verified against fujinet-firmware's rs232
' device handlers (lib/device/rs232/rs232Fuji.cpp, lib/device/fujiDevice/
' fujiDevice.cpp), not assumed from other platforms' client code:
'   - transaction_get(buf, len) FAILS if fewer than len bytes arrive (it is
'     fine with more -- rs232.cpp:75 only checks avail != len after
'     clamping avail to len). So OPEN_DIRECTORY and SET_DEVICE_FULLPATH,
'     which both transaction_get() a fixed MAX_FILENAME_LEN=256-byte
'     buffer, must be sent as a full 256-byte NUL-padded payload -- not a
'     short "path\0filter\0" -- or the transaction NAKs.
'   - COPY_FILE is the exception to that rule: it never calls
'     transaction_get() at all, reading packet.dataAsString() instead, so
'     its payload must be sent at its EXACT length -- padding it out would
'     bury NULs inside the destination filename.
'   - SET_SSID requires nparam>=1 (rs232Fuji.cpp:241), though the param
'     value itself is ignored; payload is SSIDConfig, ssid[33]+password[64]
'     = 97 bytes exactly.
'   - Directory EOF is exactly two 0x7F bytes at offsets 0 and 1
'     (fujiDevice.cpp's fujicore_read_directory_entry: "return
'     std::string(2, char(0x7F))" when dir_nextfile() is null).
'   - Directories get a trailing '/' appended to the filename by the
'     firmware itself -- no separate "is a directory" flag to read.
'
' Deliberately NOT sent anywhere in this program: CONFIG_BOOT (0xD9). See
' the plan doc for why -- CONFIG here is the RP2040's own flash _bootrom,
' not a FujiNet-served image, so the flag has no effect and would unmount
' device slot 0, which MOUNT_IMAGE just used.

    CONST FUJICMD_GET_ADAPTERCONFIG_EXTENDED = $C4
    CONST FUJICMD_GET_WIFISTATUS             = $FA
    CONST FUJICMD_GET_WIFI_ENABLED           = $EA
    CONST FUJICMD_GET_SSID                   = $FE
    CONST FUJICMD_SCAN_NETWORKS              = $FD
    CONST FUJICMD_GET_SCAN_RESULT            = $FC
    CONST FUJICMD_SET_SSID                   = $FB
    CONST FUJICMD_READ_HOST_SLOTS            = $F4
    CONST FUJICMD_WRITE_HOST_SLOTS           = $F3
    CONST FUJICMD_MOUNT_HOST                 = $F9
    CONST FUJICMD_MOUNT_IMAGE                = $F8
    CONST FUJICMD_OPEN_DIRECTORY             = $F7
    CONST FUJICMD_READ_DIR_ENTRY             = $F6
    CONST FUJICMD_CLOSE_DIRECTORY            = $F5
    CONST FUJICMD_SET_DIRECTORY_POSITION     = $E4
    CONST FUJICMD_SET_DEVICE_FULLPATH        = $E2
    CONST FUJICMD_COPY_FILE                  = $D8

    CONST MODE_READ = 1

    ' Shared scratch used only by the payload-building wrappers below (open
    ' directory / set device fullpath), which both stage a 256-byte NUL-
    ' padded buffer. Kept separate from fujinet.bas's fn_i/fn_len so callers
    ' of those don't get clobbered mid-build.
    DIM fc_hs, fc_ds, fc_mode, fc_i, fc_c
    DIM fc_maxlen, fc_addtl
    DIM #fc_pos

' ---------------------------------------------------------------------------
' fj_get_adapter_config_extended: no params, no TX payload. Reply is the
' 240-byte AdapterConfigExtended struct in FN_RX. Offsets used by st_info.bas:
'   SSID       @ 0    (33 bytes, NUL-terminated)
'   fn_version @ 125
'   sLocalIP   @ 140
' ---------------------------------------------------------------------------
fj_get_adapter_config_extended: PROCEDURE
    mb_dev = FUJI_DEVICEID
    mb_cmd = FUJICMD_GET_ADAPTERCONFIG_EXTENDED
    mb_nparam = 0
    #fn_txlen = 0
    GOSUB fn_transact
END

' ---------------------------------------------------------------------------
' fj_get_wifi_status: no params. Reply byte 0 is the WifiStatus enum
' (3 = connected).
' ---------------------------------------------------------------------------
fj_get_wifi_status: PROCEDURE
    mb_dev = FUJI_DEVICEID
    mb_cmd = FUJICMD_GET_WIFISTATUS
    mb_nparam = 0
    #fn_txlen = 0
    GOSUB fn_transact
END

' ---------------------------------------------------------------------------
' fj_get_wifi_enabled: no params. Reply byte 0 = 0 if WiFi disabled in config.
' ---------------------------------------------------------------------------
fj_get_wifi_enabled: PROCEDURE
    mb_dev = FUJI_DEVICEID
    mb_cmd = FUJICMD_GET_WIFI_ENABLED
    mb_nparam = 0
    #fn_txlen = 0
    GOSUB fn_transact
END

' ---------------------------------------------------------------------------
' fj_get_ssid: no params. Reply = STORED ssid[33]+password[64]; byte 0 = 0
' means none configured.
' ---------------------------------------------------------------------------
fj_get_ssid: PROCEDURE
    mb_dev = FUJI_DEVICEID
    mb_cmd = FUJICMD_GET_SSID
    mb_nparam = 0
    #fn_txlen = 0
    GOSUB fn_transact
END

' ---------------------------------------------------------------------------
' fj_scan_networks: no params. Reply byte 0 is the network count.
' ---------------------------------------------------------------------------
fj_scan_networks: PROCEDURE
    mb_dev = FUJI_DEVICEID
    mb_cmd = FUJICMD_SCAN_NETWORKS
    mb_nparam = 0
    #fn_txlen = 0
    GOSUB fn_transact
END

' ---------------------------------------------------------------------------
' fj_get_scan_result: fc_i = network index (0-based). Reply is SSIDInfo:
' ssid[33] (NUL-terminated) followed by a signed rssi byte at offset 33.
' ---------------------------------------------------------------------------
fj_get_scan_result: PROCEDURE
    mb_dev = FUJI_DEVICEID
    mb_cmd = FUJICMD_GET_SCAN_RESULT
    mb_nparam = 1
    pm_i = 0 : pm_size = 1 : #pm_val = fc_i : GOSUB fn_param
    #fn_txlen = 0
    GOSUB fn_transact
END

' ---------------------------------------------------------------------------
' fj_set_ssid: SC_SSID/SC_PASS (already NUL-terminated by the grid editor)
' are packed into the 97-byte SSIDConfig payload (ssid[33]+password[64],
' NUL-padded) and sent. nparam must be >=1 even though the value is
' ignored by the firmware.
' ---------------------------------------------------------------------------
fj_set_ssid: PROCEDURE
    mb_dev = FUJI_DEVICEID
    mb_cmd = FUJICMD_SET_SSID
    mb_nparam = 1
    pm_i = 0 : pm_size = 1 : #pm_val = 1 : GOSUB fn_param

    FOR fc_i = 0 TO 32
        fc_c = 0
        IF fc_i < 33 THEN fc_c = PEEK(SC_SSID + fc_i) AND 255
        POKE (FN_TX + fc_i), fc_c
    NEXT fc_i
    FOR fc_i = 0 TO 63
        fc_c = PEEK(SC_PASS + fc_i) AND 255
        POKE (FN_TX + 33 + fc_i), fc_c
    NEXT fc_i
    #fn_txlen = 97

    GOSUB fn_transact
END

' ---------------------------------------------------------------------------
' fj_read_host_slots: no params. Reply is 256 bytes, 8 x 32-byte NUL-padded
' hostnames, into FN_RX -- caller copies to SC_HOSTS.
' ---------------------------------------------------------------------------
fj_read_host_slots: PROCEDURE
    mb_dev = FUJI_DEVICEID
    mb_cmd = FUJICMD_READ_HOST_SLOTS
    mb_nparam = 0
    #fn_txlen = 0
    GOSUB fn_transact
    IF fn_ok THEN
        FOR fc_i = 0 TO 255
            POKE (SC_HOSTS + fc_i), PEEK(FN_RX + fc_i) AND 255
        NEXT fc_i
    END IF
END

' ---------------------------------------------------------------------------
' fj_write_host_slots: sends the 256-byte SC_HOSTS block verbatim (all 8
' slots at once -- there is no single-slot write command).
' ---------------------------------------------------------------------------
fj_write_host_slots: PROCEDURE
    mb_dev = FUJI_DEVICEID
    mb_cmd = FUJICMD_WRITE_HOST_SLOTS
    mb_nparam = 0
    FOR fc_i = 0 TO 255
        POKE (FN_TX + fc_i), PEEK(SC_HOSTS + fc_i) AND 255
    NEXT fc_i
    #fn_txlen = 256
    GOSUB fn_transact
END

' ---------------------------------------------------------------------------
' fj_mount_host: fc_hs = host slot (0-7).
' ---------------------------------------------------------------------------
fj_mount_host: PROCEDURE
    mb_dev = FUJI_DEVICEID
    mb_cmd = FUJICMD_MOUNT_HOST
    mb_nparam = 1
    pm_i = 0 : pm_size = 1 : #pm_val = fc_hs : GOSUB fn_param
    #fn_txlen = 0
    GOSUB fn_transact
END

' ---------------------------------------------------------------------------
' fj_open_directory: fc_hs = host slot, #fn_src = address of a NUL-
' terminated path (e.g. SC_PATH). The wire format is "path\0filter\0" NUL-
' padded to exactly 256 bytes -- fujiDevice.cpp's
' fujicore_open_directory_success splits at the first NUL and treats
' everything after it as the pattern.
'
' The filter is always taken from SC_FILTER (empty = no pattern), so every
' call site -- page draw, re-read of the selected row, folder advance --
' automatically stays on the same view. Note this is only HALF the
' filtering: .cfg-sibling suppression still happens client-side in
' st_file.bas, because the firmware's wildcard matcher (util_wildcard_match:
' only '*'/'?', no alternation or negation) cannot express "not *.cfg" in a
' pattern, no matter what the user typed.
' ---------------------------------------------------------------------------
fj_open_directory: PROCEDURE
    mb_dev = FUJI_DEVICEID
    mb_cmd = FUJICMD_OPEN_DIRECTORY
    mb_nparam = 1
    pm_i = 0 : pm_size = 1 : #pm_val = fc_hs : GOSUB fn_param

    ls_max = 192 : GOSUB fn_strlen        ' fn_len = strlen(path), from #fn_src
    fn_len = fn_len + 1                   ' include the path's own NUL
    #fn_txlen = 0
    GOSUB fn_putstr                       ' copies path+NUL, advances #fn_txlen

    ' Safe to clobber #fn_src now -- the caller's path is already staged.
    #fn_src = SC_FILTER : ls_max = FILTER_LEN : GOSUB fn_strlen
    fn_len = fn_len + 1                   ' include the filter's own NUL
    GOSUB fn_putstr

    FOR fc_i = #fn_txlen TO 255
        POKE (FN_TX + fc_i), 0
    NEXT fc_i
    #fn_txlen = 256

    GOSUB fn_transact
END

' ---------------------------------------------------------------------------
' fj_set_directory_position: #fc_pos = absolute entry index within the
' currently open directory.
' ---------------------------------------------------------------------------
fj_set_directory_position: PROCEDURE
    mb_dev = FUJI_DEVICEID
    mb_cmd = FUJICMD_SET_DIRECTORY_POSITION
    mb_nparam = 1
    pm_i = 0 : pm_size = 2 : #pm_val = #fc_pos : GOSUB fn_param
    #fn_txlen = 0
    GOSUB fn_transact
END

' ---------------------------------------------------------------------------
' fj_read_dir_entry: fc_maxlen = max reply length, fc_addtl = flags (0 for
' the plain short/scroll form used throughout this program -- the 0x80
' "additional details" form is never needed here). Reply is a NUL-
' terminated filename in FN_RX (directories carry a trailing '/'), or two
' 0x7F bytes at offsets 0/1 on EOF.
' ---------------------------------------------------------------------------
fj_read_dir_entry: PROCEDURE
    mb_dev = FUJI_DEVICEID
    mb_cmd = FUJICMD_READ_DIR_ENTRY
    mb_nparam = 2
    pm_i = 0 : pm_size = 1 : #pm_val = fc_maxlen : GOSUB fn_param
    pm_i = 1 : pm_size = 1 : #pm_val = fc_addtl : GOSUB fn_param
    #fn_txlen = 0
    GOSUB fn_transact
END

' ---------------------------------------------------------------------------
' fj_dir_entry_is_eof: call right after fj_read_dir_entry. Returns (in
' fc_c, 1/0) whether the reply was the EOF marker.
' ---------------------------------------------------------------------------
fj_dir_entry_is_eof: PROCEDURE
    fc_c = 0
    IF (PEEK(FN_RX) AND 255) = $7F AND (PEEK(FN_RX + 1) AND 255) = $7F THEN fc_c = 1
END

' ---------------------------------------------------------------------------
' fj_close_directory: no params.
' ---------------------------------------------------------------------------
fj_close_directory: PROCEDURE
    mb_dev = FUJI_DEVICEID
    mb_cmd = FUJICMD_CLOSE_DIRECTORY
    mb_nparam = 0
    #fn_txlen = 0
    GOSUB fn_transact
END

' ---------------------------------------------------------------------------
' fj_set_device_fullpath: fc_ds = device slot, fc_hs = host slot,
' fc_mode = MODE_READ, #fn_src = address of a NUL-terminated full path
' (e.g. SC_PATH followed by the chosen filename). Same 256-byte exact-
' length payload requirement as OPEN_DIRECTORY.
' ---------------------------------------------------------------------------
fj_set_device_fullpath: PROCEDURE
    mb_dev = FUJI_DEVICEID
    mb_cmd = FUJICMD_SET_DEVICE_FULLPATH
    mb_nparam = 3
    pm_i = 0 : pm_size = 1 : #pm_val = fc_ds   : GOSUB fn_param
    pm_i = 1 : pm_size = 1 : #pm_val = fc_hs   : GOSUB fn_param
    pm_i = 2 : pm_size = 1 : #pm_val = fc_mode : GOSUB fn_param

    ls_max = 192 : GOSUB fn_strlen
    fn_len = fn_len + 1
    #fn_txlen = 0
    GOSUB fn_putstr

    FOR fc_i = #fn_txlen TO 255
        POKE (FN_TX + fc_i), 0
    NEXT fc_i
    #fn_txlen = 256

    GOSUB fn_transact
END

' ---------------------------------------------------------------------------
' fj_mount_image: fc_ds = device slot, fc_mode = MODE_READ. Triggers the
' ESP32 media-type dispatch (MediaTypeROM for a .bin/.rom/.int/.itv full
' path set by fj_set_device_fullpath), which pushes the ROM (and its .cfg
' sibling, if any) to the RP2040 and boots it. This transaction can run far
' longer than an ordinary mailbox round trip -- st_boot.bas polls
' FN_BOOT_PCT rather than using this raw wrapper directly. Kept here as the
' single source of truth for the param layout.
' ---------------------------------------------------------------------------
fj_mount_image: PROCEDURE
    mb_dev = FUJI_DEVICEID
    mb_cmd = FUJICMD_MOUNT_IMAGE
    mb_nparam = 2
    pm_i = 0 : pm_size = 1 : #pm_val = fc_ds   : GOSUB fn_param
    pm_i = 1 : pm_size = 1 : #pm_val = fc_mode : GOSUB fn_param
    #fn_txlen = 0
    GOSUB fn_transact
END

' ---------------------------------------------------------------------------
' fj_copy_file: fc_hs = SOURCE host slot, fc_ds = DESTINATION host slot,
' both ALREADY 1-BASED -- unlike every other command here, COPY_FILE numbers
' the slots 1-8 rather than 0-7 (fujiDevice.cpp's fujicore_copy_file_success
' rejects 0 outright and then decrements). st_copy.bas adds the one.
'
' The caller stages the copySpec ("sourcefullpath|destfullpath") into FN_TX
' and sets #fn_txlen to its EXACT byte count -- no NUL terminator and no
' padding, unlike OPEN_DIRECTORY/SET_DEVICE_FULLPATH. Those two are read by
' the firmware with transaction_get() into a fixed 256-byte buffer;
' COPY_FILE instead reads packet.dataAsString() (rs232Fuji.cpp), which
' returns exactly the bytes sent, so a trailing NUL would end up inside the
' destination filename.
' ---------------------------------------------------------------------------
fj_copy_file: PROCEDURE
    mb_dev = FUJI_DEVICEID
    mb_cmd = FUJICMD_COPY_FILE
    mb_nparam = 2
    pm_i = 0 : pm_size = 1 : #pm_val = fc_hs : GOSUB fn_param
    pm_i = 1 : pm_size = 1 : #pm_val = fc_ds : GOSUB fn_param
    GOSUB fn_transact
END
