; fujicmd.inc -- one wrapper per FujiNet command CONFIG sends.
;
; Each returns D = 0 on an ACK, else the FNE* error (FNENAK for a NAK), so a
; caller tests it with BZ. Payload rules the server enforces:
;   - OPEN_DIRECTORY and SET_DEVICE_FULLPATH read exactly 256 path bytes,
;     NUL-padded; all-NUL padding is OPEN_DIRECTORY's empty filter.
;   - SET_SSID is exactly 97 bytes: ssid[33] then password[64].
;   - WRITE_HOST_SLOTS is exactly 8 x 32.
;   - COPY_FILE is "source|destination" at its exact length, with 1-based
;     host slots where every other command's are 0-based.
;   - Never seek to directory position 0: an opened directory is there
;     already, and dir_seek(0) fails on an empty one.

; fc_beg -- FCALL fc_beg; DB command, nparam: begin a FUJI-device command.
        RFIT    9
fc_beg: LDI     FNDEVF
        PHI     R9
        LDA     R6
        PLO     R9
        LDA     R6
        FJMP    fn_beg                  ; its FRET returns past the two bytes

; go_ack -- commit, wait, and check the reply was an ACK.
        RFIT    9
go_ack: FCALL   fn_go
        BNZ     gak1
        FCALL   fn_ack
gak1:   FRET

; c_rhosts -- READ_HOST_SLOTS: 8 x 32 names in the reply.
        RFIT    8
c_rhosts:
        FCALL   fc_beg
        DB      FNCRHST, 0
        FJMP    go_ack

; c_whosts -- WRITE_HOST_SLOTS: the table in the reply (a fresh READ), with
; slot V_HSEL replaced by V_EDIT.
        RFIT    61
c_whosts:
        FCALL   fc_beg
        DB      FNCWHST, 0
        LDI     0
cwh1:   STV     V_I
        STR     R2
        LDV     V_HSEL
        XOR
        BNZ     cwh2
        LDR     R8, V_EDIT
        BR      cwh3
cwh2:   LDN     R2
        SHL
        SHL
        SHL
        SHL
        SHL
        PLO     R8
        LDI     HI(FN_REPLY)
        PHI     R8
cwh3:   LDI     32
        FCALL   tx_fix
        LDV     V_I
        ADI     1
        XRI     8
        BZ      cwh4
        XRI     8
        BR      cwh1
cwh4:   FJMP    go_ack

; c_mhost -- MOUNT_HOST(D = host slot).
        RFIT    21
c_mhost:
        STV     V_ARG
        FCALL   fc_beg
        DB      FNCMHST, 1
        LDV     V_ARG
        FCALL   fn_pb
        FJMP    go_ack

; c_odir -- OPEN_DIRECTORY(D = host slot) over V_PATH, empty filter.
        RFIT    32
c_odir: STV     V_ARG
        FCALL   fc_beg
        DB      FNCODIR, 1
        LDV     V_ARG
        FCALL   fn_pb
        LDR     R8, V_PATH
        LDI     0
        FCALL   tx_fix
        FJMP    go_ack

; c_spos -- SET_DIRECTORY_POSITION(V_POSL/H).
        RFIT    19
c_spos: FCALL   fc_beg
        DB      FNCSPOS, 1
        LDV     V_POSL
        PLO     R8
        INC     RB
        LDN     RB
        PHI     R8
        FCALL   fn_pw
        FJMP    go_ack

; c_rdblk -- READ_DIR_ENTRY in block mode: one group of NROWS entries (and
; whatever follows that fits) from the position the directory is at.
        RFIT    18
c_rdblk:
        FCALL   fc_beg
        DB      FNCRDIR, 2
        LDI     DIRPAGES
        FCALL   fn_pb
        LDI     DIRMODE
        FCALL   fn_pb
        FJMP    go_ack

; c_cdir -- CLOSE_DIRECTORY.
        RFIT    8
c_cdir: FCALL   fc_beg
        DB      FNCCDIR, 0
        FJMP    go_ack

; c_sdfp -- SET_DEVICE_FULLPATH(device 0, D = host slot, read, V_PATH).
        RFIT    42
c_sdfp: STV     V_ARG
        FCALL   fc_beg
        DB      FNCSDFP, 3
        LDI     0
        FCALL   fn_pb
        LDV     V_ARG
        FCALL   fn_pb
        LDI     FMREAD
        FCALL   fn_pb
        LDR     R8, V_PATH
        LDI     0
        FCALL   tx_fix
        FJMP    go_ack

; c_mimg -- MOUNT_IMAGE(device 0, read): the ESP32 pushes the file to the
; cart before it answers, so the wait is past the cart's own 60 s.
        RFIT    24
c_mimg: FCALL   fc_beg
        DB      FNCMIMG, 2
        LDI     0
        FCALL   fn_pb
        LDI     FMREAD
        FCALL   fn_pb
        SETV    V_TMO, 66
        FJMP    go_ack

; c_adpx -- GET_ADAPTERCONFIG_EXTENDED (AC_* offsets).
        RFIT    8
c_adpx: FCALL   fc_beg
        DB      FNCADPX, 0
        FJMP    go_ack

; c_wst -- GET_WIFISTATUS: reply byte 0 is 3 when connected.
        RFIT    8
c_wst:  FCALL   fc_beg
        DB      FNCWST, 0
        FJMP    go_ack

; c_scan -- SCAN_NETWORKS: reply byte 0 is the count.
        RFIT    8
c_scan: FCALL   fc_beg
        DB      FNCSCAN, 0
        FJMP    go_ack

; c_sres -- GET_SCAN_RESULT(D = index): ssid at reply offset 0.
        RFIT    21
c_sres: STV     V_ARG
        FCALL   fc_beg
        DB      FNCSRES, 1
        LDV     V_ARG
        FCALL   fn_pb
        FJMP    go_ack

; c_ssid -- SET_SSID from V_SSID and V_EDIT (the password), and save it.
        RFIT    35
c_ssid: FCALL   fc_beg
        DB      FNCSSID, 1
        LDI     1
        FCALL   fn_pb
        LDR     R8, V_SSID
        LDI     33
        FCALL   tx_fix
        LDR     R8, V_EDIT
        LDI     64
        FCALL   tx_fix
        FJMP    go_ack

; c_copy -- COPY_FILE(V_SRCH -> V_HSEL): V_PATH (directory and file) to the
; destination's root, under the same name.
        RFIT    51
c_copy: FCALL   fc_beg
        DB      FNCCOPY, 2
        LDV     V_SRCH
        ADI     1
        FCALL   fn_pb
        LDV     V_HSEL
        ADI     1
        FCALL   fn_pb
        LDR     R8, V_PATH
        FCALL   tx_str
        LDI     '|'
        FCALL   fn_txb
        LDI     '/'
        FCALL   fn_txb
        SETV    V_TMO, 66
        FJMP    go_ack
