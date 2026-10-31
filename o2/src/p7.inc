; p7.inc -- page 7: info (GET_ADAPTERCONFIG_EXTENDED, three pages of four,
; a long value scrolling) and the lobby (its host found or given a slot,
; then the boot page with its path set).

AC_SSID  equ 0                  ; GET_ADAPTERCONFIG_EXTENDED's strings
AC_HOST  equ 33
AC_VER   equ 125
AC_IP    equ 140
AC_GW    equ 156
AC_MASK  equ 172
AC_DNS   equ 188
AC_MAC   equ 204
AC_BSSID equ 222
IV_CFG   equ 0FEh               ; an item's value: CONFIG's version, not the
IV_CART  equ 0FDh               ; reply's; the cart's protocol; 0FFh none
INFOPGS  equ 3

    PAGE_HEAD
    db e_info&0FFh              ; E_INFO
    db e_lobby&0FFh             ; E_LOBBY
e_info:
    jmp in_run
e_lobby:
    jmp lb_run

; to_hosts: back to the hosts.
to_hosts:
    GOTO PG_HOSTS,E_RUN

; ---------------------------------------------------------------------------
; in_run: GET_ADAPTERCONFIG_EXTENDED, then its pages: the stick or 1-3
; turns them; CLR, ENT or the button: the hosts.
in_run:
    mov r3,#FC_ADAPTEXT
    mov r4,#0
    call fuji_begin
    call fn_go
    jnz to_hosts
    mov r0,#IPAGE
    clr a
    movx @r0,a
in_page:
    call scr_text
    mov r4,#L_TOP
    mov a,#s_info&0FFh
    call say_m
    mov r4,#L_BOT
    mov a,#s_ilegend&0FFh
    call sayk_m
    mov r0,#MQOFF
    clr a
    movx @r0,a
    inc r0                      ; MQCLK
    mov a,#MQ_WAIT
    movx @r0,a
    inc r0                      ; MQDIR
    mov a,#1
    movx @r0,a
    mov r0,#IMAX
    clr a
    movx @r0,a
    mov r5,#0                   ; the four items
ip_l:
    mov r0,#TMP0
    mov a,r5
    movx @r0,a
    call in_label
    mov r0,#TMP0
    movx a,@r0
    mov r5,a
    call in_value
    mov r0,#TMP0
    movx a,@r0
    inc a
    mov r5,a
    xrl a,#4
    jnz ip_l
in_loop:
    call in_poll
    call in_marq
    mov r0,#JOYEV
    movx a,@r0
    jb2 in_prev
    jb3 in_next
    jb0 in_prev
    jb1 in_next
    jb4 in_back
    mov r0,#KEY
    movx a,@r0
    mov r5,a
    xrl a,#K_CLR
    jz in_back
    mov a,r5
    xrl a,#K_ENT
    jz in_back
    mov a,r5
    xrl a,#K_H
    jz in_help
    mov a,r5
    xrl a,#K_QUEST
    jz in_help
    mov a,r5                    ; 1-3: that page
    jz in_loop
    add a,#256-INFOPGS-1
    jc in_loop
    mov a,r5
    dec a
    jmp in_set
in_back:
    mov a,#SND_BACK
    call beep
    jmp to_hosts
in_help:
    HELP HC_INFO,PG_MISC,E_INFO
in_prev:
    mov r0,#IPAGE
    movx a,@r0
    jz in_loop
    dec a
    jmp in_set
in_next:
    mov r0,#IPAGE
    movx a,@r0
    inc a
    xrl a,#INFOPGS
    jz in_loop
    xrl a,#INFOPGS
in_set:
    mov r0,#IPAGE
    movx @r0,a
    mov a,#SND_MOVE
    call beep
    jmp in_page

    if $ > 900h
    error "the misc page's page $8 has run out"
    endif

; ===========================================================================
    org 900h

; in_item: A = byte r6 (0 the label, 1 the value) of item r5 on page
; IPAGE. Clobbers A, r0.
in_item:
    mov r0,#IPAGE
    movx a,@r0
    rl a
    rl a
    add a,r5                    ; page x 4 + item
    rl a
    add a,r6                    ; x 2 + which
    add a,#items&0FFh
    movp a,@a
    ret

; in_label: row 2 x item r5 = its label, tan. Clobbers A, r0-r7.
in_label:
    mov r6,#0
    call in_item
    mov r6,a
    mov a,r5
    rl a
    mov r4,a
    mov a,r6
    jz il_x
    call lbs0_m
    mov a,#C_TAN
    call col_set
    mov r0,#RICON
    clr a
    movx @r0,a
    jmp row_line
il_x:
    ret

; in_value: row 2 x item r5 + 1 = its value, white, from MQOFF if it's
; longer than a row (IMAX: the longest's overhang, for the scroll).
; Clobbers A, r0-r7.
in_value:
    mov a,r5
    mov r0,#TMP1
    movx @r0,a
    call lb_clear
    mov r6,#1
    call in_item
    mov r2,a
    xrl a,#IV_CFG
    jz iv_cfg
    mov a,r2
    xrl a,#IV_CART
    jz iv_cart
    mov a,r2
    inc a
    jz iv_out                   ; (none)
    mov r3,#0                   ; its length: up to the NUL, 64 at most
    mov r5,#0
iv_len:
    call st_byte
    jz iv_have
    inc r2
    inc r5
    mov a,r5
    xrl a,#64
    jnz iv_len
iv_have:
    mov a,r2                    ; back to its start
    cpl a
    inc a
    add a,r5
    cpl a
    inc a
    mov r2,a
    mov a,r5                    ; its overhang: length - 12
    add a,#256-NCOLS
    jnc iv_fit
    mov r6,a
    mov r0,#IMAX
    movx a,@r0
    cpl a
    add a,r6
    jnc iv_mq
    mov a,r6
    movx @r0,a
iv_mq:
    mov r0,#MQOFF               ; from MQOFF, the overhang at most
    movx a,@r0
    mov r7,a
    cpl a
    add a,r6
    jc iv_off
    mov a,r6
    mov r7,a
iv_off:
    mov a,r7
    add a,r2
    mov r2,a
iv_fit:
    mov r1,#LINEBUF
    mov r7,#NCOLS
    call rx_lb
    jmp iv_out
iv_cfg:
    mov a,#s_cfgver&0FFh
    call lbs0_m
    jmp iv_out
iv_cart:
    mov a,#FN_R_PROTO_VER&0FFh
    call mb_get
    add a,#'0'
    mov r0,#LINEBUF
    movx @r0,a
iv_out:
    call col_white
    mov r0,#RICON
    clr a
    movx @r0,a
    mov r0,#TMP1
    movx a,@r0
    rl a
    inc a
    mov r4,a
    jmp row_line

; in_marq: the long values: after MQ_WAIT frames still, a character every
; MQ_STEP, to the longest's end and back; each redrawn. Clobbers A, r0-r7.
in_marq:
    mov r0,#IMAX
    movx a,@r0
    jz im_x
    mov r6,a
    mov r0,#MQCLK
    movx a,@r0
    dec a
    movx @r0,a
    jnz im_x
    mov a,#MQ_STEP
    movx @r0,a
    mov r0,#MQDIR
    movx a,@r0
    mov r7,a
    mov r0,#MQOFF
    movx a,@r0
    add a,r7
    movx @r0,a
    jz im_end
    xrl a,r6
    jnz im_draw
im_end:
    mov r0,#MQDIR
    movx a,@r0
    cpl a
    inc a
    movx @r0,a
    mov r0,#MQCLK
    mov a,#MQ_WAIT
    movx @r0,a
im_draw:
    mov r5,#0
im_l:
    mov r0,#TMP0
    mov a,r5
    movx @r0,a
    call in_value
    mov r0,#TMP0
    movx a,@r0
    inc a
    mov r5,a
    xrl a,#4
    jnz im_l
im_x:
    ret

items:                          ; by page: 4 x (label, value)
    db s_lssid&0FFh, AC_SSID, s_lhost&0FFh, AC_HOST
    db s_lip&0FFh, AC_IP, s_lver&0FFh, AC_VER
    db s_lmask&0FFh, AC_MASK, s_lgw&0FFh, AC_GW
    db s_ldns&0FFh, AC_DNS, s_lmac&0FFh, AC_MAC
    db s_lbssid&0FFh, AC_BSSID, s_lcfg&0FFh, IV_CFG
    db s_lcart&0FFh, IV_CART, 0, 0FFh

    if $ > 0A00h
    error "the misc page's page $9 has run out"
    endif

; ===========================================================================
    org 0A00h

; lb_same: A = 0 if slot r5's name (reply 32 x r5) is LOBBY_HOST, any
; case. Clobbers A, r0-r3, r6, r7.
lb_same:
    mov a,r5
    swap a
    rl a
    mov r2,a
    mov r3,#0
    mov r7,#lob_host&0FFh
ls_l:
    mov a,r7                    ; LOBBY_HOST's next, then the name's
    movp a,@a
    mov r1,a
    call st_next
    orl a,#20h                  ; (a NUL stays a space: compared below)
    mov r6,a
    mov a,r1
    jz ls_end
    orl a,#20h
    xrl a,r6
    jnz ls_x
    inc r7
    jmp ls_l
ls_end:
    mov a,r6                    ; the name has to end there too
    xrl a,#20h
ls_x:
    ret

; lb_write: WRITE_HOST_SLOTS: the slots as they are, slot TMP0 named
; LOBBY_HOST. A = 0 if it worked. Clobbers A, r0-r7.
lb_write:
    mov r3,#FC_WRHOSTS
    mov r4,#0
    call fuji_begin
    mov r5,#0
lw_slot:
    mov a,r5
    mov r0,#TMP2
    movx @r0,a
    mov r0,#TMP0
    movx a,@r0
    xrl a,r5
    jz lw_new
    mov a,r5
    swap a
    rl a
    mov r2,a
    mov r3,#0
    mov r7,#32
lw_old:
    call st_next
    call fn_tx
    djnz r7,lw_old
    jmp lw_next
lw_new:
    mov r7,#lob_host&0FFh
    mov r5,#32
lw_name:
    mov a,r7
    movp a,@a
    jz lw_pad
    call fn_tx
    inc r7
    djnz r5,lw_name
    jmp lw_next
lw_pad:
    mov a,r5
    mov r6,a
    call fn_zeros
lw_next:
    mov r0,#TMP2
    movx a,@r0
    inc a
    mov r5,a
    xrl a,#NROWS
    jnz lw_slot
    jmp fn_go

; lb_path: LOBBY_PATH into the TX stream, NUL-padded to 256. Clobbers A,
; r0, r5-r7.
lb_path:
    mov r7,#lob_path&0FFh
    mov r5,#0
lp_l:
    mov a,r7
    movp a,@a
    jz lp_pad
    call fn_tx
    inc r7
    inc r5
    jmp lp_l
lp_pad:
    mov a,r5
    cpl a
    inc a
    mov r6,a
    jmp fn_zeros

    include "lobby.inc"
    include "local.inc"

    if $ > 0B00h
    error "the misc page's page $A has run out"
    endif

; ===========================================================================
    org 0B00h

; ---------------------------------------------------------------------------
; lb_run: the lobby. Its host (LOBBY_HOST, any case) in the slots, or the
; first empty one, or slot 8, given it; mounted, its prefix "/"; the boot
; with LOBBY_PATH in slot 0.
lb_run:
    call scr_text
    mov r4,#L_TOP
    mov a,#s_lobby&0FFh
    call say_m
    mov r3,#FC_RDHOSTS
    mov r4,#0
    call fuji_begin
    call fn_go
    jnz lb_fail
    mov r0,#TMP0                ; the slot it's in
    mov a,#0FFh
    movx @r0,a
    mov r0,#TMP1                ; the first empty one
    movx @r0,a
    mov r5,#0
lb_slot:
    mov a,r5
    swap a
    rl a
    mov r2,a
    mov r3,#0
    call st_byte                ; empty?
    jnz lb_cmp
    mov r0,#TMP1
    movx a,@r0
    inc a
    jnz lb_next
    mov a,r5
    movx @r0,a
    jmp lb_next
lb_cmp:
    call lb_same
    jnz lb_next
    mov r0,#TMP0
    mov a,r5
    movx @r0,a
    jmp lb_have
lb_next:
    inc r5
    mov a,r5
    xrl a,#NROWS
    jnz lb_slot
    mov r0,#TMP1                ; not there: the first empty, or slot 8
    movx a,@r0
    mov r5,a
    inc a
    jnz lb_claim
    mov r5,#NROWS-1
lb_claim:
    mov r0,#TMP0
    mov a,r5
    movx @r0,a
    call lb_write
    jnz lb_fail
lb_have:
    mov r0,#TMP0
    movx a,@r0
    mov r0,#HOST
    movx @r0,a
    mov r3,#FC_MOUNTHOST
    mov r4,#1
    call fuji_begin
    mov r0,#HOST
    movx a,@r0
    call fn_par1
    call fn_go
    jnz lb_fail
    mov r3,#FC_HPREFIXSET       ; its prefix "/"
    mov r4,#1
    call fuji_begin
    mov r0,#HOST
    movx a,@r0
    call fn_par1
    mov a,#'/'
    call fn_tx
    mov r6,#255
    call fn_zeros
    call fn_go
    jnz lb_fail
    mov r3,#FC_DEVPATH          ; slot 0: the lobby's image
    mov r4,#3
    call fuji_begin
    clr a
    call fn_par1
    mov r0,#HOST
    movx a,@r0
    call fn_par1
    mov a,#1
    call fn_par1
    call lb_path
    call fn_go
    jnz lb_fail
    GOTO PG_MOUNT,E_MOUNT
lb_fail:
    mov a,#s_nolobby&0FFh
    jmp m_fail

    if $ > 0C00h
    error "the misc page's page $B has run out"
    endif

; ===========================================================================
    org 0C00h

; m_fail: the reason (the string at offset A of this page) on row 4, then
; a key: the hosts.
m_fail:
    mov r6,a
    mov a,#SND_ERROR
    call beep
    mov a,r6
    mov r4,#4
    call row_m
    mov r4,#L_BOT
    mov a,#s_anykey&0FFh
    call say_m
    call m_key
    jmp to_hosts

; m_key: until a key or the button. Clobbers A, r0-r7.
m_key:
    call in_poll
    mov r0,#KEY
    movx a,@r0
    cpl a
    jnz mk_x
    mov r0,#JOYEV
    movx a,@r0
    jb4 mk_x
    jmp m_key
mk_x:
    ret

    STRPAGE m
; sayk_m: line r4 = the string at offset A, white, its bit-7 keys tan.
sayk_m:
    call lbs0_m
    mov a,#C_WHITE
    mov r0,#TXCOL
    movx @r0,a
    inc r0
    mov a,#C_TAN
    movx @r0,a
    jmp say_line_c
s_info:     db "  FUJINET INFO",0
s_ilegend:  db CH_ARROW|80h," MORE  ",'C'|80h,'L'|80h,'R'|80h," BACK",0
s_lssid:    db "WIFI",0
s_lhost:    db "HOST NAME",0
s_lip:      db "IP ADDRESS",0
s_lver:     db "FUJINET",0
s_lmask:    db "NETMASK",0
s_lgw:      db "GATEWAY",0
s_ldns:     db "DNS",0
s_lmac:     db "MAC",0
s_lbssid:   db "BSSID",0
s_lcfg:     db "CONFIG",0
s_lcart:    db "CART PROTO",0
s_cfgver:   db "V",CFG_VER,0
s_lobby:    db "   GAME LOBBY",0
s_nolobby:  db " NO LOBBY",0
s_anykey:   db "   PRESS A KEY",0
    if $ > 0D00h
    error "the misc page's page $C has run out"
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
