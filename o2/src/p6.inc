; p6.inc -- page 6: WiFi. The boot's check (E_CHECK), the networks found
; (E_RUN scans, E_LIST draws them again), the editor's answers (E_EDDONE),
; connecting (E_CONN).
;
; SET_SSID's payload is the SSID (33) then the password (64). The editor runs
; between the two, and the editor never transacts: so the transaction is
; begun and the SSID streamed in before it, the password after -- the cart
; holds the half-built TX stream, and nothing else has to.

WS_SSID equ 1                   ; WSTEP: the SSID typed (OTHER)
WS_PASS equ 2                   ; ... the password (the SSID is streamed)
WSIG_HI equ 196                 ; RSSI -60 dBm and up: strong
WSIG_MID equ 181                ; -75 and up: fair

    PAGE_HEAD
    db e_scan&0FFh              ; E_RUN
    db e_check&0FFh             ; E_CHECK
    db e_eddone&0FFh            ; E_EDDONE
    db e_list&0FFh              ; E_LIST
    db e_conn&0FFh              ; E_CONN
e_scan:
    jmp ws_scan
e_check:
    jmp wc_check
e_eddone:
    jmp we_done
e_list:
    jmp ws_list
e_conn:
    jmp wn_conn

; ws_loop: the stick or the keys move the bar; the button, ENT or 1-8
; picks.
ws_loop:
    call in_poll
    mov r0,#JOYEV
    movx a,@r0
    jb0 ws_up
    jb1 ws_down
    jb2 ws_left
    jb3 ws_rt
    jb4 ws_pk
    mov r0,#KEY
    movx a,@r0
    mov r5,a
    xrl a,#K_ENT
    jz ws_pk
    mov a,r5
    xrl a,#K_R
    jz ws_rescan
    mov a,r5
    xrl a,#K_CLR
    jz ws_skip
    mov a,r5
    xrl a,#K_H
    jz ws_help
    mov a,r5
    xrl a,#K_QUEST
    jz ws_help
    mov a,r5                    ; 1-8: that row, if it has one
    jz ws_loop
    add a,#256-9
    jc ws_loop
    mov r0,#WTOP
    movx a,@r0
    add a,r5
    dec a
    mov r6,a
    mov r0,#WNUM
    movx a,@r0
    cpl a
    add a,r6                    ; C: past OTHER
    jc ws_loop
    mov a,r5
    dec a
    mov r0,#SEL
    movx @r0,a
    call ws_bar
    jmp ws_pick

ws_pk:
    jmp ws_pick
ws_rt:
    jmp ws_right
ws_rescan:
    jmp ws_scan
ws_skip:
    mov a,#SND_BACK
    call beep
    jmp to_hosts
ws_help:
    HELP HC_WIFI,PG_WIFI,E_LIST

ws_up:
    mov r0,#SEL
    movx a,@r0
    jz ws_pgup
    dec a
    movx @r0,a
    call ws_bar
    jmp ws_loop
ws_pgup:
    mov r0,#WTOP                ; up off the top: the page before, its last
    movx a,@r0
    jz ws_loop
    mov r0,#SEL
    mov a,#NROWS-1
    movx @r0,a
ws_left:
    mov r0,#WTOP
    movx a,@r0
    jz ws_loop
    add a,#256-NROWS
    movx @r0,a
    jmp ws_list
ws_down:
    call ws_idx                 ; not past OTHER
    mov r0,#WNUM
    movx a,@r0
    xrl a,r6
    jz ws_loop
    mov r0,#SEL
    movx a,@r0
    xrl a,#NROWS-1
    jz ws_pgdn
    movx a,@r0
    inc a
    movx @r0,a
    call ws_bar
    jmp ws_loop
ws_pgdn:
    clr a                       ; down off the bottom: the next page's first
    movx @r0,a
ws_right:
    mov r0,#WTOP                ; a next page, if OTHER isn't on this one
    movx a,@r0
    add a,#NROWS
    mov r6,a
    mov r0,#WNUM
    movx a,@r0
    cpl a
    add a,r6                    ; C: WTOP + 8 > WNUM
    JCL ws_loop
    mov r0,#WTOP
    mov a,r6
    movx @r0,a
    mov r0,#SEL                 ; (the bar can't sit past OTHER)
    clr a
    movx @r0,a
    jmp ws_list

; w_pause: about a second. Clobbers A, r0-r7.
w_pause:
    mov r0,#TMP0
    mov a,#50
    movx @r0,a
wp_l:
    call in_poll
    mov r0,#TMP0
    movx a,@r0
    dec a
    movx @r0,a
    jnz wp_l
    ret

; w_cmd0: Fuji command r3, no parameters; A = 0 if it worked. Clobbers A,
; r0-r7.
w_cmd0:
    mov r4,#0
    call fuji_begin
    jmp fn_go

; st_0: A = the reply's first byte. Clobbers r0, r2, r3, r6.
st_0:
    mov r3,#0
    mov r2,#0
    jmp st_byte

; to_hosts: on to the hosts.
to_hosts:
    GOTO PG_HOSTS,E_RUN

    if $ > 0900h
    error "the WiFi page's page $8 has run out"
    endif

; ===========================================================================
    org 0900h
; ---------------------------------------------------------------------------
; wc_check: the cart's link to FujiNet (STATUS bit 0, up to 15 seconds); then
; WiFi off: the hosts; connected: the hosts; an SSID saved: connect to it;
; else the scan. A key skips to the hosts.
wc_check:
    call scr_text
    mov r4,#L_TOP
    mov a,#s_wifi&0FFh
    call say_w
    mov r4,#3
    mov a,#s_check&0FFh
    call row_w
    mov r0,#TMP0                ; 15 s: 900 frames, 4 x 225
    mov a,#4
    movx @r0,a
    mov r0,#TMP1
    mov a,#225
    movx @r0,a
wc_link:
    call in_poll
    mov r0,#KEY
    movx a,@r0
    cpl a
    JNZL to_hosts
    mov a,#FN_R_STATUS&0FFh
    call mb_get
    jb0 wc_up
    mov r0,#TMP1
    movx a,@r0
    dec a
    movx @r0,a
    jnz wc_link
    mov a,#225
    movx @r0,a
    mov r0,#TMP0
    movx a,@r0
    dec a
    movx @r0,a
    jnz wc_link
wc_up:
    if WIFIFAKE
    jmp ws_scan                 ; (a test build: fujinet-pc is always on)
    endif
    mov r3,#FC_WIFIEN
    call w_cmd0
    JNZL to_hosts
    call st_0
    JZL to_hosts                 ; WiFi off: nothing to set up
    mov r3,#FC_WIFISTAT
    call w_cmd0
    jnz wc_ssid
    call st_0
    xrl a,#WL_CONNECTED
    JZL to_hosts
wc_ssid:
    mov r3,#FC_GETSSID
    call w_cmd0
    JNZL ws_scan
    call st_0
    JZL ws_scan                 ; none saved
    ; fall into wn_conn

; ---------------------------------------------------------------------------
; wn_conn: connect to the SSID saved: its name, then GET_WIFISTATUS every
; second, 20 times. Connected: the hosts; never: the scan. A key skips to
; the hosts.
wn_conn:
    call scr_text
    mov r4,#L_TOP
    mov a,#s_conn&0FFh
    call say_w
    mov r3,#FC_GETSSID          ; its name, row 2
    call w_cmd0
    jnz wn_go
    call lb_clear
    mov r3,#0
    mov r2,#0
    mov r1,#LINEBUF
    mov r7,#NCOLS
    call rx_lb
    mov r4,#2
    call row_str
wn_go:
    mov r4,#L_BOT
    mov a,#s_skip&0FFh
    call say_w
    mov r0,#WTRY
    mov a,#20
    movx @r0,a
wn_try:
    mov r0,#WCLK
    mov a,#60
    movx @r0,a
wn_wait:
    call in_poll
    mov r0,#KEY
    movx a,@r0
    cpl a
    JNZL to_hosts
    mov r0,#WCLK
    movx a,@r0
    dec a
    movx @r0,a
    jnz wn_wait
    mov r0,#WTRY                ; the dots: one a try, on row 4
    movx a,@r0
    cpl a
    add a,#21                   ; 20 - tries left
    mov r6,a
    call lb_clear
    mov r0,#LINEBUF
wn_dots:
    mov a,r6
    jz wn_dot0
    mov a,#'.'
    movx @r0,a
    inc r0
    mov a,r0
    xrl a,#LINEBUF+NCOLS
    jz wn_dot0
    djnz r6,wn_dots
wn_dot0:
    mov r4,#4
    call row_str
    mov r3,#FC_WIFISTAT
    call w_cmd0
    jnz wn_next
    call st_0
    xrl a,#WL_CONNECTED
    jz wn_ok
wn_next:
    mov r0,#WTRY
    movx a,@r0
    dec a
    movx @r0,a
    jnz wn_try
    mov a,#SND_ERROR
    call beep
    mov r4,#4
    mov a,#s_fail&0FFh
    call row_w
    call w_pause
    jmp ws_scan
wn_ok:
    mov a,#SND_OK
    call beep
    mov r4,#4
    mov a,#s_ok&0FFh
    call row_w
    call w_pause
    jmp to_hosts

    if $ > 0A00h
    error "the WiFi page's page $9 has run out"
    endif

; ===========================================================================
    org 0A00h
; ---------------------------------------------------------------------------
; ws_scan: SCAN_NETWORKS, then the list from the top.
ws_scan:
    call scr_text
    mov r4,#L_TOP
    mov a,#s_nets&0FFh
    call say_w
    mov r4,#3
    mov a,#s_scan&0FFh
    call row_w
    mov r3,#FC_SCAN
    mov r4,#0
    call fuji_begin
    mov r5,#6                   ; a scan takes seconds: 25 at most
    call fn_go_long
    mov r6,#0
    jnz ws_n
    call st_0
    mov r6,a
ws_n:
    mov r0,#WNUM
    mov a,r6
    movx @r0,a
    clr a
    mov r0,#WTOP
    movx @r0,a
    mov r0,#SEL
    movx @r0,a
    ; fall into ws_list

; ws_list: the networks from WTOP, OTHER after the last; SEL's bar.
ws_list:
    call scr_text
    mov r4,#L_TOP
    mov a,#s_nets&0FFh
    call say_w
    mov r4,#L_BOT
    mov a,#s_wlegend&0FFh
    call sayk_w
    mov r4,#0
wl_row:
    mov r0,#TMP0
    mov a,r4
    movx @r0,a
    call ws_row
    mov r0,#TMP0
    movx a,@r0
    inc a
    mov r4,a
    xrl a,#NROWS
    jnz wl_row
    call ws_bar
    jmp ws_loop

; ws_pick: OTHER: the SSID from the editor. A network: SET_SSID begun, its
; SSID streamed; then the password from the editor.
ws_pick:
    mov a,#SND_SELECT
    call beep
    call ws_idx
    mov r0,#WNUM
    movx a,@r0
    xrl a,r6
    jnz wp_net
    mov r0,#WSTEP
    mov a,#WS_SSID
    movx @r0,a
    mov r5,#ED_SSID
    mov r6,#32
    jmp w_edit
wp_net:
    mov a,r6
    call ws_result
    JNZL ws_loop
    call ws_ssid_tx
    mov r3,#0                   ; the SSID: 33 bytes, from the reply
    mov r2,#0
    mov r7,#33
wp_ssid:
    call st_next
    call fn_tx
    djnz r7,wp_ssid
    mov r0,#WSTEP
    mov a,#WS_PASS
    movx @r0,a
    mov r5,#ED_PASS
    mov r6,#EDBUF_LEN
    ; fall into w_edit

; w_edit: the editor on an empty EDBUF: kind r5, longest r6; back to
; E_EDDONE.
w_edit:
    mov r0,#EDKIND
    mov a,r5
    movx @r0,a
    inc r0                      ; EDMAX
    mov a,r6
    movx @r0,a
    inc r0                      ; EDLEN
    clr a
    movx @r0,a
    mov r0,#RETPG
    mov a,#PG_WIFI
    movx @r0,a
    inc r0
    mov a,#E_EDDONE
    movx @r0,a
    GOTO PG_EDIT,E_RUN

; ws_ssid_tx: SET_SSID begun: its one parameter (save) is in. Clobbers A,
; r0, r2-r4, r6, r7.
ws_ssid_tx:
    mov r3,#FC_SETSSID
    mov r4,#1
    call fuji_begin
    mov a,#1
    jmp fn_par1

; we_done: the editor is back. Cancelled: the list. The SSID typed:
; SET_SSID begun and it streamed, then the password. The password: streamed,
; posted, then connect.
we_done:
    mov r0,#EDRES
    movx a,@r0
    JZL ws_list
    mov r0,#WSTEP
    movx a,@r0
    xrl a,#WS_SSID
    jnz wd_pass
    call ws_ssid_tx
    mov r0,#EDLEN
    movx a,@r0
    mov r5,a
    mov r1,#EDBUF
    mov r7,#33
    call xr_tx
    mov r0,#WSTEP
    mov a,#WS_PASS
    movx @r0,a
    mov r5,#ED_PASS
    mov r6,#EDBUF_LEN
    jmp w_edit
wd_pass:
    mov r0,#EDLEN
    movx a,@r0
    mov r5,a
    mov r1,#EDBUF
    mov r7,#64
    call xr_tx
    call fn_go
    JZL wn_conn
    mov a,#SND_ERROR
    call beep
    jmp ws_list

    if $ > 0B00h
    error "the WiFi page's page $A has run out"
    endif

; ===========================================================================
    org 0B00h
; ws_idx: r6 = WTOP + SEL, the network the bar is on. Clobbers A, r0.
ws_idx:
    mov r0,#WTOP
    movx a,@r0
    mov r6,a
    mov r0,#SEL
    movx a,@r0
    add a,r6
    mov r6,a
    ret

; ws_bar: the bar on SEL. Clobbers A, r0-r2, r6.
ws_bar:
    mov r0,#SEL
    movx a,@r0
    jmp bar_row

; ws_result: GET_SCAN_RESULT A: SSID (33), RSSI. A = 0 if it worked.
; Clobbers A, r0-r7.
ws_result:
    mov r5,a
    mov r3,#FC_SCANRES
    mov r4,#1
    call fuji_begin
    mov a,r5
    call fn_par1
    jmp fn_go

; ws_row: row TMP0 = network WTOP + it, its SSID and a block for its
; signal; OTHER after the last; blank past it. Clobbers A, r0-r7.
ws_row:
    mov r0,#WTOP
    movx a,@r0
    mov r6,a
    mov r0,#TMP0
    movx a,@r0
    add a,r6
    mov r6,a
    mov r0,#WNUM
    movx a,@r0
    xrl a,r6
    jz wr_other
    mov r0,#WNUM                ; C: the index is past OTHER
    movx a,@r0
    cpl a
    add a,r6
    jc wr_blank
    mov a,r6
    call ws_result
    jnz wr_blank
    call lb_clear
    mov r3,#0
    mov r2,#0
    mov r1,#LINEBUF
    mov r7,#NCOLS-1
    call rx_lb
    mov r3,#0                   ; the RSSI, a signed dBm
    mov r2,#33
    call st_byte
    mov r5,#C_GREEN
    jb7 wr_neg
    jmp wr_sig
wr_neg:
    add a,#256-WSIG_HI
    jc wr_sig
    mov r5,#C_TAN
    add a,#WSIG_HI-WSIG_MID
    jc wr_sig
    mov r5,#C_RED
wr_sig:
    mov r0,#LINEBUF+NCOLS-1
    mov a,#CH_BLOCK|80h
    movx @r0,a
    mov r0,#TXCOL
    mov a,#C_WHITE
    movx @r0,a
    inc r0
    mov a,r5
    movx @r0,a
    mov r0,#RICON
    clr a
    movx @r0,a
    mov r0,#TMP0
    movx a,@r0
    mov r4,a
    jmp row_line
wr_other:
    mov r0,#TMP0
    movx a,@r0
    mov r4,a
    mov a,#s_other&0FFh
    jmp row_w
wr_blank:
    call lb_clear
    mov r0,#TMP0
    movx a,@r0
    mov r4,a
    jmp row_str

    include "local.inc"
    if $ > 0C00h
    error "the WiFi page's page $B has run out"
    endif

; ===========================================================================
    org 0C00h
    STRPAGE w
; sayk_w: line r4 = the string at offset A, white, its bit-7 keys tan.
sayk_w:
    call lbs0_w
    mov a,#C_WHITE
    mov r0,#TXCOL
    movx @r0,a
    inc r0
    mov a,#C_TAN
    movx @r0,a
    jmp say_line_c
s_wifi:     db "      WIFI",0
s_check:    db " CHECKING",0
s_conn:     db "   CONNECTING",0
s_skip:     db " ANY KEY: SKIP",0
s_fail:     db " NOT FOUND",0
s_ok:       db " CONNECTED",0
s_nets:     db " WIFI NETWORKS",0
s_scan:     db " SCANNING",0
s_other:    db "OTHER...",0
s_wlegend:  db 'R'|80h," RESCAN ",'?'|80h," HELP",0
    if $ > 0D00h
    error "the WiFi page's page $C has run out"
    endif

; ===========================================================================
    org 0D00h
    include "mail.inc"
    include "beep.inc"

; ===========================================================================
    org 0E00h
    include "input.inc"
    include "snd.inc"

    PAGE_TAIL
