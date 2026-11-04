; hosts.inc -- the host slots, and (in copy livery) the copy's destination
; picker; the GAME LOBBY action.
;
; Names are drawn straight out of the reply window. Nothing is kept but a
; byte of which slots are named, which stops fire mounting an empty one.

; hosts_draw: the eight slots, the pointer on `cur`.
hosts_draw:
        LD      HL,str_hosts
        LD      DE,str_leg_hosts
        LD      A,(copy_mode)
        OR      A
        JR      Z,hd_frame
        LD      HL,str_t_copy
        LD      DE,str_leg_copy
hd_frame:
        PUSH    DE
        CALL    draw_frame
        POP     HL
        CALL    legend_line
        CALL    cmd_read_hosts
        JR      NC,hd_list
        LD      HL,str_ehosts
        CALL    fail_code
        XOR     A
        LD      (nrows),A
        RET
hd_list:
        XOR     A
        LD      (hosts_mask),A
        LD      (tmp_a),A               ; tmp_a = slot
hd_slot:
        LD      A,(tmp_a)
        CALL    list_row
        LD      E,TEXT_L
        LD      A,(tmp_a)
        INC     A
        PUSH    DE
        CALL    disp_u8
        POP     DE
        LD      E,3
        LD      A,(tmp_a)
        RRCA                            ; x 32
        RRCA
        RRCA
        LD      L,A
        LD      H,FN_REPLY/256
        LD      A,(HL)
        OR      A
        JR      NZ,hd_name
        LD      HL,str_empty
        CALL    disp_str
        JR      hd_next
hd_name:
        LD      B,NAMELEN-2
        CALL    disp_strn
        LD      A,(tmp_a)               ; hosts_mask |= 1 << slot
        LD      B,A
        INC     B
        XOR     A
        SCF
hd_bit: RLA
        DJNZ    hd_bit
        LD      HL,hosts_mask
        OR      (HL)
        LD      (HL),A
hd_next:
        LD      HL,tmp_a
        INC     (HL)
        LD      A,(HL)
        CP      HOST_SLOTS
        JR      C,hd_slot
        LD      A,HOST_SLOTS            ; empty slots stay reachable:
        LD      (nrows),A               ; RENAME SLOT names them
        LD      (at_end),A
        LD      A,(cur)
        CP      HOST_SLOTS
        JR      C,hd_sel
        XOR     A
        LD      (cur),A
hd_sel: CALL    select_cur
        LD      HL,str_select_host
        LD      A,(copy_mode)
        OR      A
        JR      Z,hd_say
        LD      HL,str_copy_to
hd_say: JP      status_line

; slot_named: NZ if slot `cur` has a name.
slot_named:
        LD      A,(cur)
        LD      B,A
        INC     B
        LD      A,(hosts_mask)
sn_loop:
        RRCA
        DJNZ    sn_loop
        AND     80H
        RET

; enter_host: mount slot `cur` and browse its root.
enter_host:
        CALL    slot_named
        JR      NZ,eh_mount
        LD      A,SND_ERROR
        CALL    snd_play
        LD      HL,str_empty_rename
        LD      A,(copy_mode)
        OR      A
        JR      Z,eh_say
        LD      HL,str_slot_empty
eh_say: JP      status_line
eh_mount:
        LD      A,SND_OK
        CALL    snd_play
        LD      HL,str_mounting_host
        CALL    status_now
        LD      A,(cur)
        CALL    cmd_mount_host
        JR      NC,eh_open
        LD      HL,str_ehost
        JP      fail_code
eh_open:
        LD      A,(cur)
        LD      (host),A
        LD      HL,path
        LD      (HL),'/'
        INC     HL
        LD      (HL),0
        XOR     A
        LD      (fn_entry),A            ; the filter starts clear
        LD      (top),A
        LD      (top+1),A
        CALL    cmd_open_dir
        JR      NC,eh_files
        LD      HL,str_eopen
        JP      fail_code
eh_files:
        XOR     A
        LD      (cur),A
        LD      A,STATE_FILES
        LD      (state),A
        RET

; rename_host: slot `cur`'s name through the keyboard and back.
rename_host:
        CALL    cmd_read_hosts
        JR      NC,rh_fetch
        LD      HL,str_ehosts
        JP      fail_code
rh_fetch:
        LD      A,(cur)
        RRCA
        RRCA
        RRCA
        LD      L,A
        LD      H,FN_REPLY/256
        LD      DE,fn_entry
        LD      B,HOST_STRIDE-1
rh_copy:
        LD      A,(HL)
        LD      (DE),A
        OR      A
        JR      Z,rh_edit
        INC     HL
        INC     DE
        DJNZ    rh_copy
        XOR     A
        LD      (DE),A
rh_edit:
        LD      HL,str_host_name
        LD      DE,str_input_host
        LD      A,HOST_STRIDE-1
        CALL    fn_edit
        JR      C,rh_draw
        LD      HL,str_saving
        CALL    status_now
        LD      A,(cur)
        LD      HL,fn_entry
        CALL    cmd_write_host
        JR      NC,rh_draw
        LD      HL,str_ewrite
        CALL    fail_code
        LD      A,90
        CALL    in_wait
rh_draw:
        JP      hosts_draw              ; the keyboard had the screen

host_menu_names:
        DW      str_m_info,str_m_lobby,str_m_rename,str_m_wifi
host_menu_events:
        DB      EV_HASH,EV_KEY0,EV_KEY0+9,EV_BACK

; st_hosts: the host list's event loop.
st_hosts:
        CALL    hosts_draw
sh_loop:
        LD      A,(state)
        CP      STATE_HOSTS
        RET     NZ
        CALL    in_read
        CP      EV_MENU
        JR      NZ,sh_ev
        LD      A,(copy_mode)
        OR      A
        JR      NZ,sh_loop
        LD      HL,host_menu_names
        LD      DE,host_menu_events
        LD      B,4
        CALL    menu_pick
sh_ev:  CP      EV_UP
        JR      NZ,sh_dn
        LD      A,0FFH
        CALL    bar_move
        JR      sh_loop
sh_dn:  CP      EV_DOWN
        JR      NZ,sh_fire
        LD      A,1
        CALL    bar_move
        JR      sh_loop
sh_fire:
        CP      EV_FIRE
        JR      NZ,sh_back
        CALL    enter_host
        JR      sh_loop
sh_back:
        CP      EV_BACK
        JR      NZ,sh_keys
        LD      A,SND_BACK
        CALL    snd_play
        LD      A,(copy_mode)
        OR      A
        JR      Z,sh_wifi
        CALL    copy_cancel
        LD      A,(state)
        CP      STATE_HOSTS
        CALL    Z,hosts_draw            ; the source would not reopen
        JR      sh_loop
sh_wifi:
        LD      A,STATE_SET_WIFI
        LD      (state),A
        RET
sh_keys:
        CP      EV_KEY0+1               ; 1-8: that slot, opened (or picked)
        JR      C,sh_actions
        CP      EV_KEY0+HOST_SLOTS+1
        JR      NC,sh_actions
        SUB     EV_KEY0+1
        PUSH    AF
        CALL    cur_hide
        POP     AF
        LD      (cur),A
        CALL    select_cur
        CALL    enter_host
        JR      sh_loop
sh_actions:
        LD      B,A
        LD      A,(copy_mode)           ; none of these mid-copy
        OR      A
        JR      NZ,sh_loop
        LD      A,B
        LD      C,STATE_INFO
        CP      EV_HASH
        JR      Z,sh_state
        LD      C,STATE_LOBBY
        CP      EV_KEY0
        JR      Z,sh_state
        CP      EV_KEY0+9
        JP      NZ,sh_loop
        CALL    rename_host
        JP      sh_loop
sh_state:
        LD      A,C
        LD      (state),A
        RET

; ---- the game lobby ----------------------------------------------------------
; Find the lobby's host among the slots, case-insensitively, or claim the
; last slot for it; then boot the lobby from it.

; lobby_match: Z if the slot name at HL is the lobby's host, ignoring
; case.
lobby_match:
        LD      DE,str_lobby_host
lm_loop:
        LD      A,(DE)
        CALL    fold_lower
        LD      B,A
        LD      A,(HL)
        CALL    fold_lower
        CP      B
        RET     NZ
        OR      A
        RET     Z
        INC     HL
        INC     DE
        JR      lm_loop

; fold_lower: A in lowercase if it is a capital.
fold_lower:
        CP      'A'
        RET     C
        CP      'Z'+1
        RET     NC
        ADD     A,20H
        RET

lobby_go:
        LD      HL,str_finding_lobby
        CALL    status_now
        CALL    cmd_read_hosts
        JR      NC,lg_find
        LD      HL,str_ehosts
        JP      fail_code
lg_find:
        XOR     A
        LD      (tmp_a),A
lg_slot:
        LD      A,(tmp_a)
        RRCA
        RRCA
        RRCA
        LD      L,A
        LD      H,FN_REPLY/256
        CALL    lobby_match
        JR      Z,lg_mount
        LD      HL,tmp_a
        INC     (HL)
        LD      A,(HL)
        CP      HOST_SLOTS
        JR      C,lg_slot
        LD      A,HOST_SLOTS-1          ; not there: claim the last slot
        LD      (tmp_a),A
        LD      HL,str_lobby_host
        CALL    cmd_write_host
        JR      NC,lg_mount
        LD      HL,str_ewrite
        JP      fail_code
lg_mount:
        LD      HL,str_mounting_lobby
        CALL    status_now
        LD      A,(tmp_a)
        CALL    cmd_mount_host
        JR      NC,lg_boot
        LD      HL,str_ehost
        JP      fail_code
lg_boot:
        LD      A,(tmp_a)
        LD      (host),A
        LD      HL,str_lobby_path
        CALL    boot_begin
        LD      A,(host)
        LD      HL,str_lobby_path
        CALL    cmd_path_fixed
        JP      NC,boot_mount_swap
        PUSH    AF
        CALL    boot_cancel
        POP     AF
        LD      HL,str_epath
        JP      fail_code

; st_lobby: on success the lobby boots; whatever failed is on screen.
st_lobby:
        CALL    lobby_go
        LD      A,120
        CALL    in_wait
        LD      A,STATE_HOSTS
        LD      (state),A
        RET
