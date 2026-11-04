; copy.inc -- host-to-host copy.
;
;   1. COPY FILE (or keypad 5) on a file marks it (src_spec, src_host) and
;      goes to the hosts, which show themselves as COPY TO WHICH HOST?
;   2. Picking a host mounts it and browses it like any other.
;   3. COPY HERE, anywhere in the destination, copies, then puts the
;      browser back where the copy started.
;
; No confirmation and no progress: COPY_FILE is one host-side
; transaction whose ACK comes when the copy is done.

; copy_mark: the selected file as the copy's source.
copy_mark:
        LD      A,(nrows)
        OR      A
        RET     Z
        LD      HL,str_reading
        CALL    status_now
        CALL    read_selected
        JR      NC,cm_name
        LD      HL,str_eread
        JP      fail_code
cm_name:
        JR      NZ,cm_kind
        LD      A,(FN_ERR)
        LD      HL,str_eempty
        JP      fail_code
cm_kind:
        CALL    is_folder
        JR      NZ,cm_room
        LD      HL,str_no_folder
        JP      say_wait
cm_room:
        CALL    path_room
        JP      C,too_long
        PUSH    BC                      ; src_spec = path + name
        LD      HL,path
        LD      DE,src_spec
        CALL    str_copy
        POP     BC
        LD      HL,FN_REPLY
        LD      C,B
        LD      B,0
        LDIR
        XOR     A
        LD      (DE),A
        LD      A,(host)
        LD      (src_host),A
        LD      (cur),A                 ; the hosts start on the source
        LD      A,1
        LD      (copy_mode),A
        CALL    cmd_close_dir
        LD      A,STATE_HOSTS
        LD      (state),A
        RET

; copy_return: back to the source's folder, page one, unfiltered. On
; success state is STATE_FILES and the caller redraws; on failure the hosts.
copy_return:
        XOR     A
        LD      (copy_mode),A
        LD      (fn_entry),A
        LD      A,(src_host)
        LD      (host),A
        LD      HL,src_spec             ; path = src_spec up to its last '/'
        LD      DE,path
        CALL    str_copy
        EX      DE,HL
crt_back:
        DEC     HL
        LD      A,(HL)
        CP      '/'
        JR      NZ,crt_back
        INC     HL
        LD      (HL),0
        CALL    top_home
        CALL    cmd_close_dir
        LD      A,(host)
        CALL    cmd_mount_host
        LD      HL,str_ehost
        JR      C,crt_fail
        CALL    cmd_open_dir
        LD      HL,str_eopen
        JR      C,crt_fail
        LD      A,STATE_FILES
        LD      (state),A
        RET
crt_fail:
        CALL    fail_code
        LD      A,90
        CALL    in_wait
        LD      A,(host)
        LD      (cur),A
        LD      A,STATE_HOSTS
        LD      (state),A
        RET

; copy_here: copy the source into the current folder.
copy_here:
        LD      HL,str_copying
        CALL    status_now
        CALL    cmd_copy
        JR      C,ch_fail
        LD      A,SND_READY
        CALL    snd_play
        LD      HL,str_copied
        CALL    status_line
        JR      ch_back
ch_fail:
        LD      HL,str_ecopy
        CALL    fail_code
ch_back:
        LD      A,120
        CALL    in_wait
        JR      copy_return

copy_cancel     EQU     copy_return
