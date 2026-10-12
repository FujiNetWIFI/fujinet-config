; boot.inc -- st_boot: mount the file V_PATH names into device slot 0 and
; hand the console to it.
;
; MOUNT_IMAGE makes the ESP32 push the file to the cart before it answers;
; the cart stages it and draws the progress bar in the status strip itself.
; Once BOOTSTATE is READY, fn_boot arms the swap and restarts the BIOS, which
; the cart answers with the new image. It returns only if the swap never arms.

        RFIT    182
st_boot:
        STATE
        FCALL   pr_cls
        DB      "BOOTING", 0
        LDI     20H                     ; the name, while the reply still has it
        FCALL   d_at
        LDI     TOP_MARQUEE
        FCALL   d_op
        LDV     V_CUR
        FCALL   ent_off
        LDI     0
        FCALL   d_rpl
        FCALL   pr_at
        DB      50H, "MOUNTING...", 0
        FCALL   c_cdir                  ; the listing is done with
        LDV     V_HOST
        FCALL   c_mhost
        BZ      bt1
        FCALL   fail
        DB      2
        DW      bt_fail
bt1:    LDV     V_HOST
        FCALL   c_sdfp
        BZ      bt2
        FCALL   fail
        DB      5
        DW      bt_fail
bt2:    FCALL   c_mimg
        BZ      bt3
        FCALL   fail
        DB      6
        DW      bt_fail
bt3:    LDI     10
        FCALL   fn_frames
bt4:    PEEK    FN_BOOTSTATE
        XRI     FN_BOOT_READY
        BZ      bt6
        XRI     FN_BOOT_READY ! FN_BOOT_FAILED ; AS: '!' is XOR ('^' is power)
        BZ      bt5
        FCALL   fn_tick
        BDF     bt4
        LDI     FNEWAIT
        BR      bt7
bt5:    PEEK    FN_BOOTERR
bt7:    FCALL   fail
        DB      7
        DW      bt_fail
bt6:    FCALL   pr_at
        DB      50H, "STARTING...", 0
        FCALL   fn_boot
        FCALL   fail
        DB      15
        DW      bt_fail

; bt_fail -- back to the listing, the file's name off V_PATH again.
bt_fail:
        STATE
        FCALL   path_cut
        FJMP    st_files
