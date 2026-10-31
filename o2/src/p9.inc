; p9.inc -- page 9: the browser's folders. Where it is lives in FujiNet, as
; the host's prefix (SET_HOST_PREFIX: a path from "/" replaces it, a name is
; added to it), so the console keeps no path: OPEN_DIRECTORY's path is "",
; and a mount's is just the name. Here: a folder opened and counted, its
; path for a title; up a folder, into one; the filter to the editor and back;
; a file marked for a copy. Each ends on the browser's page (p3.inc).

    PAGE_HEAD
    db e_open&0FFh              ; E_OPEN
    db e_reopen&0FFh            ; E_REOPEN
    db e_parent&0FFh            ; E_PARENT
    db e_enter&0FFh             ; E_ENTER
    db e_filter&0FFh            ; E_FILTER
    db e_mark&0FFh              ; E_MARK
    db e_new&0FFh               ; E_NEW
    db e_fedone&0FFh            ; E_FEDONE
e_open:
    jmp dr_open
e_reopen:
    jmp dr_reopen
e_parent:
    jmp dr_parent
e_enter:
    jmp dr_enter
e_filter:
    jmp dr_filter
e_mark:
    jmp dr_mark
e_new:
    jmp dr_new
e_fedone:
    jmp dr_fedone

; dr_new: a folder from its top: nothing counted, no pages behind, the bar
; at the top; then open it.
dr_new:
    clr a
    mov r0,#POSL
    movx @r0,a
    inc r0
    movx @r0,a
    mov r0,#PGN
    movx @r0,a
    mov r0,#SEL
    movx @r0,a
    mov r0,#CNTH
    mov a,#80h
    movx @r0,a
    ; fall into dr_open

; dr_open: the screen, the path, the folder opened and counted; its page.
; dr_reopen: the same, counted already.
dr_open:
    call dr_screen
    jnz dr_fail
    call dr_count
    jmp dr_show
dr_reopen:
    call dr_screen
    jnz dr_fail
dr_show:
    GOTO PG_FILES,E_SHOW
dr_fail:
    mov a,#SND_ERROR
    call beep
    mov r4,#3
    mov a,#s_noopen&0FFh
    call row_d
    call dr_pause
    GOTO PG_HOSTS,E_RUN

; dr_screen: a blank screen, the path in T0, the legend in T1, the folder
; opened. A = 0 if it opened. Clobbers A, r0-r7.
dr_screen:
    call scr_text
    call folder
    call dr_title
    call dr_legend
    ; fall into dr_opendir

; dr_opendir: OPEN_DIRECTORY on the host: "" (the prefix is the folder),
; then the filter if there is one, NUL-padded to 256. A = 0 if it worked.
; Clobbers A, r0-r7.
dr_opendir:
    mov r3,#FC_OPENDIR
    mov r4,#1
    call fuji_begin
    mov r0,#HOST
    movx a,@r0
    call fn_par1
    clr a                       ; the path: none
    call fn_tx
    mov r5,#0
    mov r0,#FLAGS               ; (copying: the destination has no filter)
    movx a,@r0
    anl a,#FL_FILT|FL_COPY
    xrl a,#FL_FILT
    jnz do_pat
    mov r0,#FLEN
    movx a,@r0
    mov r5,a
do_pat:
    mov r1,#EDBUF
    mov r7,#255
    call xr_tx
    jmp fn_go

; dr_title: GET_HOST_PREFIX: the path's last 16 characters onto T0.
; Clobbers A, r0-r7.
dr_title:
    call lb_clear
    call dr_prefix
    jnz dt_out
    mov a,r5                    ; from its length - 16, or its start
    add a,#256-TXT_LEN
    jc dt_from
    clr a
dt_from:
    mov r2,a
    mov r3,#0
    mov r1,#LINEBUF
    mov r7,#TXT_LEN
    call rx_lb
dt_out:
    mov a,#C_TAN
    call col_set
    mov r4,#L_TOP
    jmp say_line_c

; dr_prefix: GET_HOST_PREFIX into the reply, r5 = its length (255 at
; most). A = 0 if it worked. Clobbers A, r0-r7.
dr_prefix:
    mov r3,#FC_HPREFIXGET
    mov r4,#1
    call fuji_begin
    mov r0,#HOST
    movx a,@r0
    call fn_par1
    call fn_go
    jnz dp_x
    mov r3,#0
    mov r2,#0
    mov r5,#0
dp_len:
    call st_next
    jz dp_have
    inc r5
    mov a,r5
    xrl a,#255
    jnz dp_len
dp_have:
    clr a
dp_x:
    ret

; dr_legend: T1: copying, "C COPY HERE"; a filter, "F" and it; else the
; keys. Clobbers A, r0-r7.
dr_legend:
    mov r4,#L_BOT
    mov r0,#FLAGS
    movx a,@r0
    jb0 dl_copy
    jb1 dl_filt
    mov a,#s_flegend&0FFh
    jmp sayk_d
dl_copy:
    mov a,#s_copyhere&0FFh
    jmp sayk_d
dl_filt:
    mov a,#s_filter&0FFh
    call lbs0_d
    mov r0,#FLEN
    movx a,@r0
    mov r7,a
    mov r0,#EDBUF
    mov r1,#LINEBUF+3
dl_f:
    movx a,@r0
    movx @r1,a
    inc r0
    inc r1
    mov a,r1
    xrl a,#LINEBUF+TXT_LEN
    jz dl_out
    djnz r7,dl_f
dl_out:
    jmp dl_say

    if $ > 900h
    error "the folders' page $8 has run out"
    endif

; ===========================================================================
    org 900h

; dr_count: the folder's entries, by probing: SET_DIRECTORY_POSITION n,
; READ_DIR_ENTRY -- there is an n'th or not -- 1, 2, 4 ... until there isn't,
; then halving between the last there and the first not. CNT; uncounted if
; FujiNet balks. Clobbers A, r0-r7.
dr_count:
    mov r6,#0
    mov r5,#0
    call dr_has
    jnz dc_zero
    mov r0,#PLO                 ; 0 is there; try 1
    clr a
    movx @r0,a
    inc r0
    movx @r0,a
    inc r0                      ; PHI
    inc a
    movx @r0,a
    inc r0
    clr a
    movx @r0,a
dc_up:
    mov r0,#PHI
    call w_ld
    call dr_has
    jnz dc_bis
    mov r0,#PHI                 ; there: PLO = PHI, PHI doubled
    call w_ld
    mov r0,#PLO
    call w_st
    mov a,r6
    add a,r6
    mov r6,a
    mov a,r5
    addc a,r5
    mov r5,a
    jb7 dc_cap                  ; (32768 is past any folder: not there)
    mov r0,#PHI
    call w_st
    jmp dc_up
dc_cap:
    mov r0,#PHI
    call w_st
dc_bis:
    mov r0,#PHI                 ; PHI - PLO
    call w_ld
    mov a,r6
    mov r4,a
    mov a,r5
    mov r3,a
    mov r0,#PLO
    call w_ld
    mov a,r6                    ; r4:r3 - r6:r5 = ~(~PHI + PLO)
    xch a,r4
    cpl a
    add a,r4
    cpl a
    mov r4,a
    mov a,r5
    xch a,r3
    cpl a
    addc a,r3
    cpl a
    mov r3,a                    ; r3:r4 = the gap (hi:lo)
    jnz dc_mid
    mov a,r4
    xrl a,#1
    jz dc_done                  ; next to each other: PLO + 1 is the count
dc_mid:
    clr c                       ; PLO + gap / 2
    mov a,r3
    rrc a
    mov r3,a
    mov a,r4
    rrc a
    add a,r6
    mov r6,a
    mov a,r3
    addc a,r5
    mov r5,a
    mov r0,#TMP2                ; (dr_has keeps nothing)
    mov a,r6
    movx @r0,a
    mov r0,#TMP1
    mov a,r5
    movx @r0,a
    call dr_has
    mov r7,a
    mov r0,#TMP2
    movx a,@r0
    mov r6,a
    mov r0,#TMP1
    movx a,@r0
    mov r5,a
    mov a,r7
    mov r0,#PLO                 ; there: PLO = mid; not: PHI = mid
    jz dc_set
    mov r0,#PHI
dc_set:
    call w_st
    jmp dc_bis
dc_done:
    mov r0,#PLO
    call w_ld
    inc r6
    mov a,r6
    jnz dc_put
    inc r5
dc_put:
    mov r0,#CNTL
    jmp w_st
dc_zero:
    mov r6,#0
    mov r5,#0
    mov r0,#CNTL
    jmp w_st

; dr_has: A = 0 if the folder has an entry at r6:r5 (low, high).
; Clobbers A, r0-r7.
dr_has:
    mov r0,#TMP0
    mov a,r6
    movx @r0,a
    mov r3,#FC_DIRPOS
    mov r4,#1
    call fuji_begin             ; (keeps r5)
    mov r0,#TMP0
    movx a,@r0
    mov r6,a
    call fn_par2
    call fn_go
    jnz dh_no
    mov r3,#FC_READDIR          ; two characters will do: 7F 7F is the end
    mov r4,#2
    call fuji_begin
    mov a,#2
    call fn_par1
    clr a
    call fn_par1
    call fn_go
    jnz dh_no
    mov r3,#0
    mov r2,#0
    call st_next
    xrl a,#7Fh
    jnz dh_yes
    call st_byte
    xrl a,#7Fh
    jnz dh_yes
dh_no:
    mov a,#1
    ret
dh_yes:
    clr a
    ret

; w_ld: r6 (low) r5 (high) = the word at r0. w_st: the word at r0 = them.
; Clobber A, r0.
w_ld:
    movx a,@r0
    mov r6,a
    inc r0
    movx a,@r0
    mov r5,a
    ret
w_st:
    mov a,r6
    movx @r0,a
    inc r0
    mov a,r5
    movx @r0,a
    ret

    if $ > 0A00h
    error "the folders' page $9 has run out"
    endif

; ===========================================================================
    org 0A00h

; dr_parent: up a folder: the prefix up to its last '/' (a trailing one
; aside); "/" itself: back to the hosts.
dr_parent:
    call dr_prefix
    jnz dpa_hosts
    mov a,r5                    ; "" or "/": the top
    add a,#256-2
    jnc dpa_hosts
    dec r5                      ; the last character: a '/' doesn't count
    mov a,r5
    mov r2,a
    mov r3,#0
    call st_byte
    xrl a,#'/'
    jnz dpa_scan
    dec r5
dpa_scan:                       ; back to the '/' before
    mov a,r5
    jz dpa_root
    mov r2,a
    mov r3,#0
    call st_byte
    xrl a,#'/'
    jz dpa_cut
    dec r5
    jmp dpa_scan
dpa_root:
    inc r5                      ; "/"
dpa_cut:
    mov r0,#TMP1                ; the new prefix: its first r5 characters
    mov a,r5
    movx @r0,a
    call dr_pfxbegin
    mov r0,#TMP1
    movx a,@r0
    mov r7,a
    mov r6,a
    cpl a
    inc a
    mov r0,#TMP2                ; the zeros after it: 256 - length
    movx @r0,a
    mov r3,#0
    mov r2,#0
dpa_tx:
    call st_next
    call fn_tx
    djnz r7,dpa_tx
    jmp dr_pfxend
dpa_hosts:
    GOTO PG_HOSTS,E_RUN

; dr_enter: into the bar's folder: SET_HOST_PREFIX its name (added to the
; prefix), streamed from the block still in the reply.
dr_enter:
    mov r0,#SEL
    movx a,@r0
    rl a
    add a,#ROWT
    mov r0,a
    movx a,@r0
    mov r2,a
    inc r0
    movx a,@r0
    anl a,#3
    mov r3,a
    mov r0,#TMP0
    mov a,r2
    movx @r0,a
    inc r0
    mov a,r3
    movx @r0,a
    call dr_pfxbegin
    mov r0,#TMP0
    movx a,@r0
    mov r2,a
    inc r0
    movx a,@r0
    mov r3,a
    mov r7,#255
    mov r6,#0                   ; the name's length
de_tx:
    call st_next
    jz de_end
    call fn_tx
    inc r6
    djnz r7,de_tx
de_end:
    mov a,r6                    ; the zeros after it: 256 - length
    cpl a
    inc a
    mov r0,#TMP2
    movx @r0,a
    ; fall into dr_pfxend

; dr_pfxend: TMP2 zeros (0: 256) to fill the 256-byte path, post it, and a
; new folder. dr_pfxbegin: SET_HOST_PREFIX begun, its host parameter in.
dr_pfxend:
    mov r0,#TMP2
    movx a,@r0
    mov r6,a
    call fn_zeros
    call fn_go
    jmp dr_new
dr_pfxbegin:
    mov r3,#FC_HPREFIXSET
    mov r4,#1
    call fuji_begin
    mov r0,#HOST
    movx a,@r0
    jmp fn_par1

; dr_filter: the filter to the editor (EDBUF holds it; none: empty); back
; at dr_fedone.
dr_filter:
    mov r0,#FLAGS
    movx a,@r0
    anl a,#FL_FILT
    jz df_len
    mov r0,#FLEN
    movx a,@r0
df_len:
    mov r0,#EDLEN
    movx @r0,a
    mov r0,#EDKIND
    mov a,#ED_FILTER
    movx @r0,a
    inc r0                      ; EDMAX
    mov a,#FILTMAX
    movx @r0,a
    mov r0,#RETPG
    mov a,#PG_FILES
    movx @r0,a
    inc r0
    mov a,#E_FEDIT
    movx @r0,a
    GOTO PG_EDIT,E_RUN

; dr_fedone: the editor is back with the filter. Accepted: it is the filter
; (empty: none), the folder from its top. Cancelled: the page again.
dr_fedone:
    mov r0,#EDRES
    movx a,@r0
    jz dfd_same
    mov r0,#EDLEN
    movx a,@r0
    mov r0,#FLEN
    movx @r0,a
    mov r6,a
    mov r0,#FLAGS
    movx a,@r0
    anl a,#~FL_FILT
    mov r5,a
    mov a,r6
    jz dfd_set
    mov a,r5
    orl a,#FL_FILT
    mov r5,a
dfd_set:
    mov a,r5
    movx @r0,a
    jmp dr_new
dfd_same:
    jmp dr_reopen

    if $ > 0B00h
    error "the folders' page $A has run out"
    endif

; ===========================================================================
    org 0B00h
    include "local.inc"
    STRPAGE d
; sayk_d: line r4 = the string at offset A, white, its bit-7 keys tan.
sayk_d:
    call lbs0_d
dl_say:
    mov a,#C_WHITE
    mov r0,#TXCOL
    movx @r0,a
    inc r0
    mov a,#C_TAN
    movx @r0,a
    jmp say_line_c
s_flegend:  db 'F'|80h," FILTER  ",'?'|80h," HELP",0
s_copyhere: db 'C'|80h," COPY HERE",0
s_filter:   db 'F'|80h,": ",0
s_noopen:   db " NO FOLDER",0

; dr_mark: the bar's file is the copy's source: its host and its place in
; the folder (the page's first + its row's, ROWT bits 2-6); then the hosts,
; to pick where it goes.
dr_mark:
    mov r0,#SEL
    movx a,@r0
    rl a
    add a,#ROWT+1
    mov r0,a
    movx a,@r0
    rr a
    rr a
    anl a,#1Fh
    mov r6,a
    mov r0,#POSL
    movx a,@r0
    add a,r6
    mov r0,#COPYPL
    movx @r0,a
    mov r0,#POSH
    movx a,@r0
    addc a,#0
    mov r0,#COPYPH
    movx @r0,a
    mov r0,#HOST
    movx a,@r0
    mov r0,#COPYH
    movx @r0,a
    mov r0,#FLAGS
    movx a,@r0
    orl a,#FL_COPY
    movx @r0,a
    mov a,#SND_OK
    call beep
    GOTO PG_HOSTS,E_RUN

; dr_pause: about a second and a half. Clobbers A, r0-r7.
dr_pause:
    mov r0,#TMP2
    mov a,#90
    movx @r0,a
dpz_l:
    call in_poll
    mov r0,#TMP2
    movx a,@r0
    dec a
    movx @r0,a
    jnz dpz_l
    ret

    if $ > 0C00h
    error "the folders' page $B has run out"
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
