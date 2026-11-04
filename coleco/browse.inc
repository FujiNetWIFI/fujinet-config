; browse.inc -- the file browser: paging, descend and climb, the filter,
; and the long-name scroll.
;
; One seek a page, then sequential reads (READ_DIR_ENTRY advances by
; itself), names drawn straight out of the reply window at listing width
; and re-read at full width when one is acted on: the firmware cuts a name
; of 30 or more characters to 28 with "..." in the middle.
;
; Let the pointer rest on a cut name and the whole of it bounces through
; the row, drawn out of the reply window. Every event stops the scroll
; first, so the window is never held across a transaction.
;
; `path` is '/'-prefixed and '/'-terminated: appended to on descend,
; truncated at the previous '/' on climb, which lands on page one.

SC_IDLE         EQU     45              ; rest this long before scrolling,
SC_STEP         EQU     6               ; step a character this often,
SC_HOLD         EQU     30              ; and pause this long at each end
SCR_WAIT         EQU     0
SCR_RUN          EQU     1
SCR_OFF          EQU     2

; sel_pos: HL = the directory position of row `cur`.
sel_pos:
        LD      HL,(top)
        LD      A,(cur)
        ADD     A,L
        LD      L,A
        RET     NC
        INC     H
        RET

; read_selected: row `cur` re-read at full width. Carry set on failure
; (A = the error); otherwise B = the name's length, Z if it is empty.
read_selected:
        CALL    sel_pos
        LD      A,FULLLEN
        CALL    cmd_seek_read
        RET     C
        LD      HL,FN_REPLY
        LD      C,FULLLEN
        CALL    name_len
        LD      A,B
        OR      A
        RET

; is_folder: Z if the B-character name in the reply window ends in '/'.
is_folder:
        LD      HL,FN_REPLY-1
        LD      E,B
        LD      D,0
        ADD     HL,DE
        LD      A,(HL)
        CP      '/'
        RET

; path_room: carry clear if B more characters fit in `path`.
path_room:
        LD      HL,path
        PUSH    BC
        LD      C,B
        CALL    str_len
        LD      A,B
        ADD     A,C
        POP     BC
        CP      PATH_MAX
        CCF
        RET

; too_long: say the path can't grow, and put the legend back.
too_long:
        LD      HL,str_path_long
say_wait:
        PUSH    HL
        LD      A,SND_ERROR
        CALL    snd_play
        POP     HL
        CALL    status_line
        LD      A,90
        CALL    in_wait
        JP      files_legend

files_legend:
        LD      HL,str_pick_folder
        LD      A,(copy_mode)
        OR      A
        JR      NZ,fl_say
        LD      HL,str_select_file
        LD      A,(fn_entry)
        OR      A
        JR      Z,fl_say
        LD      HL,str_select_filt
fl_say: CALL    status_line
        LD      HL,str_leg_files
        JP      legend_line

; files_page: one page from `top`. Every row is cleared, even past the
; end. Carry set if the seek failed.
files_page:
        CALL    win_clear
        XOR     A
        LD      (nrows),A
        LD      (long_rows),A
        LD      HL,(top)
        CALL    cmd_seek
        JR      NC,fp_read
        LD      HL,str_eseek
        CALL    fail_code
        SCF
        RET
fp_read:
        XOR     A
        LD      (at_end),A
fp_row: LD      A,(nrows)
        CP      LIST_ROWS
        JR      NC,fp_done
        LD      A,NAMELEN
        CALL    cmd_read_entry
        JR      NC,fp_draw
        LD      A,1
        LD      (at_end),A
        JR      fp_done
fp_draw:
        LD      A,(nrows)
        CALL    list_row
        LD      E,TEXT_L
        LD      HL,FN_REPLY
        LD      B,NAMELEN
        CALL    disp_strn
        LD      HL,FN_REPLY
        LD      C,NAMELEN
        CALL    name_len
        LD      A,B
        CP      CUT_LEN
        JR      C,fp_count
        LD      A,(nrows)               ; long_rows |= 1 << row
        LD      B,A
        INC     B
        XOR     A
        SCF
fp_bit: RLA
        DJNZ    fp_bit
        LD      HL,long_rows
        OR      (HL)
        LD      (HL),A
fp_count:
        LD      HL,nrows
        INC     (HL)
        JR      fp_row
fp_done:
        LD      A,(nrows)
        OR      A
        JR      NZ,fp_sel
        LD      HL,str_empty
        LD      A,(fn_entry)
        OR      A
        JR      Z,fp_none
        LD      HL,str_no_matches
fp_none:
        LD      DE,LIST_TOP*256+TEXT_L
        CALL    disp_str
        OR      A
        RET
fp_sel: LD      B,A
        LD      A,(cur)
        CP      B
        JR      C,fp_ptr
        LD      A,B
        DEC     A
        LD      (cur),A
fp_ptr: CALL    select_cur
        OR      A
        RET

; files_draw: the whole screen, titled with the tail of the path.
files_draw:
        LD      HL,path
        CALL    str_len
        LD      A,B
        SUB     TITLE_MAX
        JR      C,fd_title
        LD      E,A
        LD      D,0
        ADD     HL,DE
fd_title:
        CALL    draw_frame
        CALL    files_page
        JP      NC,files_legend
        LD      HL,str_leg_noseek
        JP      legend_line

; page_back / page_next: a page up or down, the pointer on row A.
page_back:
        PUSH    AF
        LD      HL,(top)
        LD      DE,LIST_ROWS
        OR      A
        SBC     HL,DE
        LD      (top),HL
        JR      page_show
page_next:
        PUSH    AF
        LD      HL,(top)
        LD      DE,LIST_ROWS
        ADD     HL,DE
        LD      (top),HL
page_show:
        POP     AF
        LD      (cur),A
        CALL    files_page
        LD      A,SND_MOVE
        JP      snd_play

; has_prev: NZ if `top` is past the first page.
has_prev:
        LD      HL,(top)
        LD      A,H
        OR      A
        RET     NZ
        LD      A,L
        CP      LIST_ROWS
        CCF
        SBC     A,A
        RET

; files_up / files_down: the pointer, with page-crossing. Up from row 0
; goes back a page even when this one is empty, so paging past the end of
; an exactly full directory never traps it.
files_up:
        LD      A,(nrows)
        OR      A
        JR      Z,fu_page
        LD      A,(cur)
        OR      A
        JR      Z,fu_page
        CALL    cur_hide
        LD      HL,cur
        DEC     (HL)
fu_move:
        CALL    select_cur
        LD      A,SND_MOVE
        JP      snd_play
fu_page:
        CALL    has_prev
        RET     Z
        LD      A,LIST_ROWS-1
        JR      page_back

files_down:
        LD      A,(nrows)
        OR      A
        RET     Z
        LD      B,A
        LD      A,(cur)
        INC     A
        CP      B
        JR      NC,fdn_page
        CALL    cur_hide
        LD      HL,cur
        INC     (HL)
        JR      fu_move
fdn_page:
        LD      A,(at_end)
        OR      A
        RET     NZ
        XOR     A
        JR      page_next

; leave_host: close the directory, best effort, and go to the hosts.
leave_host:
        CALL    cmd_close_dir
        LD      A,(host)
        LD      (cur),A
        LD      A,STATE_HOSTS
        LD      (state),A
        RET

; climb: up a directory; at the root, back to the hosts (mid-copy that
; picks another destination host).
climb:  LD      HL,path
        CALL    str_len
        LD      A,B
        CP      2
        JR      C,leave_host
        LD      E,B                     ; back from inside the last '/'
        LD      D,0
        ADD     HL,DE
        DEC     HL
cl_back:
        DEC     HL
        LD      A,(HL)
        CP      '/'
        JR      NZ,cl_back
        INC     HL
        LD      (HL),0
        CALL    top_home
        LD      A,SND_BACK
        CALL    snd_play
        LD      HL,str_reading
        CALL    status_now
        CALL    cmd_reopen_dir
        JP      NC,files_draw
        LD      HL,str_eopen
        JP      fail_code

; top_home: page one, row one.
top_home:
        XOR     A
        LD      (top),A
        LD      (top+1),A
        LD      (cur),A
        RET

; open_or_boot: fire. A folder: descend into it. A file: boot it, or
; mid-copy, point at COPY HERE.
open_or_boot:
        LD      A,(nrows)
        OR      A
        RET     Z
        LD      A,SND_OK
        CALL    snd_play
        LD      HL,str_reading
        CALL    status_now
        CALL    read_selected
        JR      NC,ob_name
        LD      HL,str_eread
        JP      fail_code
ob_name:
        JR      NZ,ob_kind
        LD      A,(FN_ERR)
        LD      HL,str_eempty
        JP      fail_code
ob_kind:
        CALL    is_folder
        JR      NZ,ob_file
        CALL    path_room
        JP      C,too_long
        LD      HL,path                 ; append the name, '/' and all
        CALL    str_end
        PUSH    HL                      ; where the parent ends
        EX      DE,HL
        LD      HL,FN_REPLY
        LD      C,B
        LD      B,0
        LDIR
        XOR     A
        LD      (DE),A
        CALL    top_home
        CALL    cmd_reopen_dir
        POP     HL
        JP      NC,files_draw
        PUSH    HL
        LD      HL,str_eopen
        CALL    fail_code
        LD      A,90
        CALL    in_wait
        POP     HL
        LD      (HL),0                  ; back to the parent
        CALL    cmd_reopen_dir
        JP      files_draw
ob_file:
        LD      A,(copy_mode)
        OR      A
        JR      Z,ob_boot
        LD      HL,str_menu_copy
        JP      status_line
ob_boot:
        CALL    boot_reply_entry        ; comes back only on failure
        LD      A,120
        CALL    in_wait
        JP      files_draw

; do_filter: the filter is fn_entry while the browser is up, so the
; keyboard shows what is applied, and END and ESC both keep the edit.
do_filter:
        XOR     A
        LD      (fn_entry+FILTER_MAX),A
        LD      HL,str_filter
        LD      DE,str_filter_hint
        LD      A,FILTER_MAX
        CALL    fn_edit
        CALL    top_home
        LD      HL,str_reading
        CALL    status_now
        CALL    cmd_reopen_dir
        JP      NC,files_draw
        LD      HL,str_eopen
        CALL    fail_code
        JP      files_draw

; ---- the scroll ---------------------------------------------------------------

; scroll_stop: before any event: the row back as listed, and the rest
; counted again from now.
scroll_stop:
        LD      A,(sc_phase)
        CP      SCR_RUN
        JR      NZ,ss_reset
        LD      A,(cur)
        CALL    list_row
        LD      E,TEXT_L
        LD      B,NAMELEN
        CALL    restore_cells
ss_reset:
        LD      A,SCR_WAIT
        LD      (sc_phase),A
        LD      A,SC_IDLE
        LD      (sc_wait),A
        CALL    in_frames
        LD      (sc_frame),A
        RET

; scroll_row: columns 1-30 of row `cur` from the name at sc_off.
scroll_row:
        LD      A,(sc_off)
        LD      L,A
        LD      H,FN_REPLY/256
        LD      A,(cur)
        CALL    list_row
        JP      disp_row30

; scroll_tick: between events, at most once a frame. The re-read is its
; only transaction; if it fails the row stays as listed and the next
; real action reports the error.
scroll_tick:
        LD      A,(sc_phase)
        CP      SCR_OFF
        RET     Z
        CALL    in_frames
        LD      HL,sc_frame
        CP      (HL)
        RET     Z
        LD      (HL),A
        LD      HL,sc_wait
        DEC     (HL)
        RET     NZ
        LD      A,(sc_phase)
        CP      SCR_WAIT
        JR      NZ,st_run
        LD      A,SCR_OFF
        LD      (sc_phase),A
        LD      A,(cur)                 ; only a row that may be cut
        LD      B,A
        INC     B
        LD      A,(long_rows)
st_bit: RRCA
        DJNZ    st_bit
        RLA
        RET     NC
        CALL    read_selected
        RET     C
        LD      A,B
        LD      (sc_len),A
        CP      NAMELEN
        RET     C                       ; whole after all
        XOR     A
        LD      (sc_off),A
        INC     A
        LD      (sc_dir),A
        LD      A,SC_HOLD
        LD      (sc_wait),A
        LD      A,SCR_RUN
        LD      (sc_phase),A
        LD      A,(cur)
        CALL    list_row
        LD      E,TEXT_L
        LD      B,NAMELEN
        CALL    save_cells
        JR      scroll_row
st_run: LD      A,SC_HOLD
        LD      (sc_wait),A
        LD      A,(sc_len)
        CP      NAMELEN
        RET     Z                       ; it fits: nothing to move
        LD      A,(sc_dir)
        LD      HL,sc_off
        ADD     A,(HL)
        LD      (HL),A
        PUSH    AF
        CALL    scroll_row
        POP     AF
        OR      A
        JR      Z,st_turn
        LD      B,A
        LD      A,(sc_len)
        SUB     NAMELEN
        CP      B
        JR      Z,st_turn
        LD      A,SC_STEP
        LD      (sc_wait),A
        RET
st_turn:
        LD      A,(sc_dir)
        NEG
        LD      (sc_dir),A
        RET

files_menu_names:
        DW      str_m_filter,str_m_copy,str_m_hosts
files_menu_events:
        DB      EV_KEY0+4,EV_KEY0+5,EV_HASH
copy_menu_names:
        DW      str_m_copy_here,str_m_cancel_copy
copy_menu_events:
        DB      EV_KEY0+5,EV_HASH

; st_files: the browser's event loop.
st_files:
        CALL    files_draw
        CALL    scroll_stop
sf_loop:
        LD      A,(state)
        CP      STATE_FILES
        RET     NZ
        CALL    in_read
        OR      A
        JR      NZ,sf_event
        CALL    scroll_tick
        JR      sf_loop
sf_event:
        PUSH    AF
        CALL    scroll_stop
        POP     AF
        CP      EV_MENU
        JR      NZ,sf_back
        LD      HL,files_menu_names
        LD      DE,files_menu_events
        LD      B,3
        LD      A,(copy_mode)
        OR      A
        JR      Z,sf_menu
        LD      HL,copy_menu_names
        LD      DE,copy_menu_events
        LD      B,2
sf_menu:
        CALL    menu_pick
sf_back:
        CP      EV_BACK                 ; the right button climbs; at the
        JR      NZ,sf_up                ; root it leaves, as HOSTS does
        LD      A,(path+1)
        OR      A
        LD      A,EV_KEY0+1
        JR      NZ,sf_up
        LD      A,EV_HASH
sf_up:  CP      EV_UP
        JR      NZ,sf_down
        CALL    files_up
        JR      sf_loop
sf_down:
        CP      EV_DOWN
        JR      NZ,sf_left
        CALL    files_down
        JR      sf_loop
sf_left:
        CP      EV_LEFT
        JR      NZ,sf_right
        CALL    has_prev
        JR      Z,sf_loop
        XOR     A
        CALL    page_back
        JR      sf_loop
sf_right:
        CP      EV_RIGHT
        JR      NZ,sf_fire
        LD      A,(nrows)
        OR      A
        JR      Z,sf_loop
        LD      A,(at_end)
        OR      A
        JR      NZ,sf_loop
        CALL    page_next
        JR      sf_loop
sf_fire:
        CP      EV_FIRE
        JR      NZ,sf_k1
        CALL    open_or_boot
        JR      sf_loop
sf_k1:  CP      EV_KEY0+1
        JR      NZ,sf_k4
        CALL    climb
        JP      sf_loop
sf_k4:  CP      EV_KEY0+4
        JR      NZ,sf_k5
        CALL    do_filter
        JP      sf_loop
sf_k5:  CP      EV_KEY0+5
        JR      NZ,sf_hash
        LD      A,(copy_mode)
        OR      A
        JR      NZ,sf_here
        CALL    copy_mark
        JP      sf_loop
sf_here:
        CALL    copy_here
        JR      sf_redraw
sf_hash:
        CP      EV_HASH
        JP      NZ,sf_loop
        LD      A,(copy_mode)
        OR      A
        JR      NZ,sf_cancel
        CALL    leave_host
        JP      sf_loop
sf_cancel:
        CALL    copy_cancel
sf_redraw:
        LD      A,(state)
        CP      STATE_FILES
        CALL    Z,files_draw
        JP      sf_loop
