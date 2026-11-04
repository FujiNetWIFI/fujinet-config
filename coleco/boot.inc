; boot.inc -- mount into device slot 0, watch the image arrive, and take
; the ROM swap.
;
; Booting is called from wherever the choice was made (the browser, the
; lobby) and comes back only on failure. Every server pushes the whole
; image to the cart inside MOUNT_IMAGE, before it ACKs, so the mount is
; sent by hand (mount_start) and, while its ACK is outstanding, this reads
; only the cart's status page and the VDP: a mailbox write would queue
; behind the mount. The percent, and the byte counts on carts that report
; them, go into a pop-up over the browser.

BW_X0           EQU     2               ; the window
BW_Y0           EQU     4
BW_X1           EQU     29
BW_Y1           EQU     12
BW_COL          EQU     3               ; its first interior column
BW_WIDTH        EQU     26
PHASE_ROW       EQU     5
NAME_ROW        EQU     7
BAR_ROW         EQU     9
BAR_COL         EQU     4               ; first cell; the caps either side
BAR_CELLS       EQU     20
PCT_COL         EQU     25              ; "100%" in 25-28
BYTES_ROW       EQU     11              ; "  13K OF 32K"
K_WIDTH         EQU     4               ; the count's columns

IDLE_GRACE      EQU     60              ; ACKed but nothing pushed: no ROM
QUIET_MAX       EQU     4200            ; 70 s with no change: past the
                                        ; cart's own 60 s per-frame deadline

; bw_phase: the window's phase word HL.
bw_phase:  LD      DE,PHASE_ROW*256+BW_COL
        JP      disp_str

; bar_cell: bar cell B drawn for a bar filled to bt_want_px pixels.
bar_cell:
        LD      A,B
        ADD     A,A
        ADD     A,A
        ADD     A,A
        LD      C,A                     ; C = the cell's first pixel
        LD      A,(bt_want_px)
        SUB     C
        JR      NC,bc_some
        XOR     A
bc_some:
        CP      9
        JR      C,bc_tile
        LD      A,8
bc_tile:
        ADD     A,T_BAR0
        LD      D,BAR_ROW
        PUSH    AF
        LD      A,B
        ADD     A,BAR_COL
        LD      E,A
        POP     AF
        JP      disp_tile

; prepare: the bar and the percent for A percent.
prepare:
        CP      101
        JR      C,pr_ok
        LD      A,100
pr_ok:  LD      (bt_want_pct),A
        LD      L,A                     ; 160 px = 100%: px = pct x 8 / 5
        LD      H,0
        ADD     HL,HL
        ADD     HL,HL
        ADD     HL,HL
        LD      DE,5
        LD      B,0
pr_div: OR      A
        SBC     HL,DE
        JR      C,pr_px
        INC     B
        JR      pr_div
pr_px:  LD      A,B
        LD      (bt_want_px),A
        RET

; apply: only what changed, and on the bar only the cells between the
; old edge and the new one.
apply:  CALL    apply_bar
        JP      apply_bytes

apply_bar:
        LD      A,(bt_shown_px)
        LD      C,A
        LD      A,(bt_want_px)
        CP      C
        JR      Z,ap_pct
        LD      B,A                     ; B = hi, C = lo
        JR      NC,ap_order
        LD      B,C
        LD      C,A
ap_order:
        LD      A,(bt_want_px)
        LD      (bt_shown_px),A
        LD      A,B
        LD      (bt_hi),A
        LD      A,C                     ; from cell lo / 8
        RRCA
        RRCA
        RRCA
        AND     1FH
        LD      B,A
ap_cell:
        LD      A,B
        CP      BAR_CELLS
        JR      NC,ap_pct
        ADD     A,A
        ADD     A,A
        ADD     A,A
        LD      HL,bt_hi
        CP      (HL)
        JR      NC,ap_pct
        PUSH    BC
        CALL    bar_cell
        POP     BC
        INC     B
        JR      ap_cell
ap_pct: LD      A,(bt_shown_pct)
        LD      C,A
        LD      A,(bt_want_pct)
        CP      C
        RET     Z
        LD      (bt_shown_pct),A
        LD      HL,numbuf               ; " 42%"
        LD      B,' '
        CP      100
        JR      C,ap_hund
        LD      B,'1'
        SUB     100
ap_hund:
        LD      (HL),B
        INC     HL
        LD      C,0                     ; C = tens, A = ones
ap_ten: CP      10
        JR      C,ap_ones
        SUB     10
        INC     C
        JR      ap_ten
ap_ones:
        ADD     A,'0'
        LD      B,A
        LD      A,C
        ADD     A,'0'
        CP      '0'
        JR      NZ,ap_tens
        LD      A,(numbuf)              ; a leading zero is a blank, but
        CP      '1'                     ; 100 keeps its
        LD      A,'0'
        JR      Z,ap_tens
        LD      A,' '
ap_tens:
        LD      (HL),A
        INC     HL
        LD      (HL),B
        INC     HL
        LD      (HL),'%'
        INC     HL
        LD      (HL),0
        LD      HL,numbuf
        LD      DE,BAR_ROW*256+PCT_COL
        JP      disp_str

; read_kb: HL = the 24-bit count at HL on the status page, in K. The cart
; writes it low byte first, so it is read until two reads agree.
read_kb:
        PUSH    HL
        CALL    rk_once
        POP     HL
        PUSH    DE
        PUSH    HL
        CALL    rk_once
        POP     HL
        POP     BC
        LD      A,B
        CP      D
        JR      NZ,read_kb
        LD      A,C
        CP      E
        JR      NZ,read_kb
        EX      DE,HL
        RET
; rk_once: DE = the count at HL over 1024. Clobbers A, HL.
rk_once:
        INC     HL
        LD      A,(HL)                  ; bits 8-15
        RRCA
        RRCA
        AND     3FH
        LD      E,A
        INC     HL
        LD      A,(HL)                  ; bits 16-23
        LD      D,A
        RRCA
        RRCA
        AND     0C0H
        OR      E
        LD      E,A
        LD      A,D
        SRL     A
        SRL     A
        LD      D,A
        RET

; div10: HL = HL / 10, A = the remainder. Preserves BC and DE.
div10:  PUSH    BC
        LD      B,16
        XOR     A
dv_bit: ADD     HL,HL
        RLA
        CP      10
        JR      C,dv_next
        SUB     10
        INC     L
dv_next:
        DJNZ    dv_bit
        POP     BC
        RET

; put_dec: HL in decimal, written leftward to end just before DE. Leaves
; DE on the first digit.
put_dec:
        CALL    div10
        ADD     A,'0'
        DEC     DE
        LD      (DE),A
        LD      A,H
        OR      L
        JR      NZ,put_dec
        RET

; ab_count: rowbuf's first K_WIDTH columns: bt_got_k, right-aligned.
ab_count:
        LD      HL,(bt_got_k)
        LD      (bt_shown_k),HL
        LD      DE,10000
        OR      A
        SBC     HL,DE
        ADD     HL,DE
        JR      C,abc_fits
        LD      HL,9999
abc_fits:
        LD      A,' '
        LD      (rowbuf),A
        LD      (rowbuf+1),A
        LD      (rowbuf+2),A
        LD      (rowbuf+3),A
        LD      DE,rowbuf+K_WIDTH
        JP      put_dec

; apply_bytes: "  13K OF 32K", drawn whole once, then only the count as
; it changes. Nothing on a cart that reports no total.
apply_bytes:
        LD      HL,(bt_tot_k)
        LD      A,H
        OR      L
        RET     Z
        LD      A,(bt_bytes_on)
        OR      A
        JR      Z,ab_line
        LD      HL,(bt_got_k)
        LD      DE,(bt_shown_k)
        OR      A
        SBC     HL,DE
        RET     Z
        CALL    ab_count
        XOR     A
        LD      (rowbuf+K_WIDTH),A
        JR      ab_draw
ab_line:
        LD      A,1
        LD      (bt_bytes_on),A
        CALL    ab_count
        LD      HL,str_k_of
        LD      DE,rowbuf+K_WIDTH
        CALL    str_copy
        PUSH    DE                      ; the total, digits then "K", built
        LD      A,'K'                   ; at the far end of rowbuf
        LD      (rowbuf+24),A
        XOR     A
        LD      (rowbuf+25),A
        LD      HL,(bt_tot_k)
        LD      DE,rowbuf+24
        CALL    put_dec
        EX      DE,HL
        POP     DE
        CALL    str_copy
ab_draw:
        LD      HL,rowbuf
        LD      DE,BYTES_ROW*256+BW_COL
        JP      disp_str

; boot_begin: the progress window over the browser, titled with the tail
; of the name at HL. Open it BEFORE setting the path: that reply repaints
; the window the name may be read from.
boot_begin:
        PUSH    HL
        LD      HL,str_please_wait2
        LD      D,MSG_LINE1
        CALL    msg_put
        LD      D,MSG_LINE2
        CALL    disp_row_blank
        LD      DE,BW_Y0*256+BW_X0
        LD      BC,BW_Y1*256+BW_X1
        CALL    ovl_open
        LD      HL,str_mounting
        CALL    bw_phase
        POP     HL
        LD      C,FULLLEN
        CALL    name_len
        LD      A,B
        SUB     BW_WIDTH
        JR      C,bb_name
        LD      E,A
        LD      D,0
        ADD     HL,DE
bb_name:
        LD      DE,NAME_ROW*256+BW_COL
        LD      B,BW_WIDTH
        CALL    disp_strn
        LD      DE,BAR_ROW*256+BAR_COL-1
        LD      A,T_CAPL
        CALL    disp_tile
        LD      E,BAR_COL+BAR_CELLS
        LD      A,T_CAPR
        CALL    disp_tile
        LD      HL,0
        LD      (bt_tot_k),HL
        LD      (bt_got_k),HL
        XOR     A
        LD      (bt_bytes_on),A
        LD      (bt_want_px),A
        LD      (bt_shown_px),A
        LD      B,A
bb_cell:
        PUSH    BC
        CALL    bar_cell
        POP     BC
        INC     B
        LD      A,B
        CP      BAR_CELLS
        JR      C,bb_cell
        LD      A,0FFH
        LD      (bt_shown_pct),A
        XOR     A
        CALL    prepare
        JP      apply

; boot_cancel: close the window again, on a failure.
boot_cancel:
        CALL    ovl_close
        JP      in_init                 ; drop presses made while it loaded

; boot_reply_entry: boot `path` plus the full-width name in the reply
; window. Comes back only on failure.
boot_reply_entry:
        LD      HL,FN_REPLY
        CALL    boot_begin
        CALL    cmd_path_reply
        JR      NC,boot_mount_swap
        PUSH    AF
        CALL    boot_cancel
        POP     AF
        LD      HL,str_epath
        JP      fail_code

boot_errors:
        DW      str_eload,str_eload_big,str_eload_short,str_eload_map
        DW      str_eload_busy

; boot_mount_swap: MOUNT_IMAGE with the window open, then the swap.
; Comes back only on failure, the window closed and the error said.
boot_mount_swap:
        IF      BARTEST
        JP      bar_test
        ENDIF
        CALL    mount_start
        LD      (bt_want),A
        XOR     A
        LD      (bt_acked),A
        LD      (bt_idle),A
        LD      (bt_quiet),A
        LD      (bt_quiet+1),A
        DEC     A
        LD      (bt_last_st),A
        LD      (bt_last_pct),A
        CALL    in_frames
        LD      (bt_frame),A
bm_frame:
        CALL    in_frames               ; ticks the sound and the cursor
        LD      HL,bt_frame
        CP      (HL)
        JR      Z,bm_frame
        LD      (HL),A
        CALL    apply                   ; early in the frame
        LD      A,(bt_acked)
        OR      A
        JR      NZ,bm_state
        LD      A,(bt_want)
        LD      HL,FN_ACKSEQ
        CP      (HL)
        JR      NZ,bm_progress
        LD      A,1
        LD      (bt_acked),A
        CALL    reply_ok
        JR      Z,bm_state
        LD      A,(FN_BOOTSTATE)        ; a refused image NAKs the mount;
        CP      BOOT_FAILED             ; the cart says why
        JR      Z,bm_failed
        LD      A,(FN_ERR)
        LD      HL,str_emount
        JR      bm_fail
; The outcome is read only once the ACK is in: until the cart takes the
; SEQ a FAILED may be left from an earlier mount, and leaving early would
; send the browser's next transaction out behind this one.
bm_state:
        LD      A,(FN_BOOTSTATE)
        CP      BOOT_READY
        JP      Z,bm_ready
        CP      BOOT_FAILED
        JR      Z,bm_failed
        OR      A
        JR      NZ,bm_progress
        LD      HL,bt_idle
        INC     (HL)
        LD      A,(HL)
        CP      IDLE_GRACE+1
        JR      C,bm_progress
        CALL    boot_cancel             ; mounted as a plain disk: the
        LD      A,SND_ERROR             ; server pushed nothing
        CALL    snd_play
        LD      HL,str_not_rom
        JP      status_line
bm_failed:
        LD      A,(FN_BOOTERR)
        LD      C,A
        CP      5
        JR      C,bm_name
        XOR     A
bm_name:
        ADD     A,A
        LD      E,A
        LD      D,0
        LD      HL,boot_errors
        ADD     HL,DE
        LD      E,(HL)
        INC     HL
        LD      D,(HL)
        EX      DE,HL
        LD      A,C
bm_fail:
        PUSH    AF
        PUSH    HL
        CALL    boot_cancel
        POP     HL
        POP     AF
        JP      fail_code
bm_progress:
        LD      A,(FN_BOOTSTATE)
        LD      HL,bt_last_st
        CP      (HL)
        JR      Z,bm_pct
        LD      (HL),A
        CALL    bm_alive
        LD      A,(bt_last_st)
        CP      BOOT_XFER
        JR      NZ,bm_pct
        LD      HL,str_loading
        CALL    bw_phase
bm_pct: LD      A,(bt_last_st)
        CP      BOOT_XFER
        JR      NZ,bm_quiet
        LD      A,(FN_BOOTPCT)
        LD      HL,bt_last_pct
        CP      (HL)
        JR      Z,bm_count
        LD      (HL),A
        CALL    prepare
        CALL    bm_alive
bm_count:
        LD      HL,FN_BOOTTOT
        CALL    read_kb
        LD      (bt_tot_k),HL
        LD      HL,FN_BOOTGOT
        CALL    read_kb
        LD      DE,(bt_got_k)
        LD      (bt_got_k),HL
        OR      A
        SBC     HL,DE
        CALL    NZ,bm_alive
bm_quiet:
        LD      HL,(bt_quiet)
        INC     HL
        LD      (bt_quiet),HL
        LD      DE,QUIET_MAX+1
        OR      A
        SBC     HL,DE
        JP      C,bm_frame
        LD      A,FN_ETIMEOUT
        LD      HL,str_etimeout
        JR      bm_fail
bm_alive:
        LD      HL,0
        LD      (bt_quiet),HL
        RET

; The window ends full, the chime plays, the scene fades out, and the PSG
; goes quiet so nothing carries into the game.
bm_ready:
        LD      HL,FN_BOOTTOT
        CALL    read_kb
        LD      (bt_tot_k),HL
        LD      (bt_got_k),HL
        LD      A,100
        CALL    prepare
        LD      A,1
        CALL    in_wait
        CALL    apply
        LD      HL,str_ready
        CALL    bw_phase
        LD      A,SND_READY
        CALL    snd_play
bm_chime:
        CALL    in_frames
        CALL    snd_busy
        JR      NZ,bm_chime
        CALL    fade_out
        CALL    snd_init

; boot_swap: arm the swap, then from RAM turn the VDP and PSG off, read
; FN_SWAP (every cartridge byte changes on that read), reset the VDP's
; registers and cold-start the new image through the BIOS.
boot_swap:
        LD      C,FR_BOOTLOCK
        LD      A,FN_BOOTLOCK_MAGIC
        CALL    FNREGWR
        LD      HL,swap_stub
        LD      DE,src_spec
        LD      BC,SWAP_STUB_LEN
        LDIR
        LD      B,0                     ; give the cart time to arm it
bs_wait:
        EX      (SP),HL
        EX      (SP),HL
        DJNZ    bs_wait
        JP      src_spec

; Copied to RAM and run there: it uses only relative jumps.
swap_stub:
        XOR     A                       ; screen and interrupt off
        OUT     (VDP_CTRL),A
        LD      A,81H
        OUT     (VDP_CTRL),A
        IN      A,(VDP_CTRL)
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        LD      A,9FH                   ; the PSG's four channels silent
        OUT     (PSG_PORT),A
        LD      A,0BFH
        OUT     (PSG_PORT),A
        LD      A,0DFH
        OUT     (PSG_PORT),A
        LD      A,0FFH
        OUT     (PSG_PORT),A
        LD      A,(FN_SWAP)             ; the swap
        LD      HL,8000H                ; VDP registers 0-7 = 0
        LD      B,8
ss_vreg:
        LD      A,L
        OUT     (VDP_CTRL),A
        LD      A,H
        OUT     (VDP_CTRL),A
        INC     H
        DJNZ    ss_vreg
        JP      0
SWAP_STUB_LEN   EQU     $-swap_stub

        IF      BARTEST
; bar_test: the window's bar runs 0-100% of a 32K image over five
; seconds with no mount, for screenshots; then READY, and it stays.
bar_test:
        LD      HL,str_loading
        CALL    bw_phase
        LD      HL,32
        LD      (bt_tot_k),HL
        XOR     A
        LD      (bt_last_pct),A
btest_loop:
        LD      A,3
        CALL    in_wait
        LD      A,(bt_last_pct)
        INC     A
        LD      (bt_last_pct),A
        CALL    prepare
        LD      A,(bt_last_pct)         ; K = percent x 32 / 100
        LD      L,A
        LD      H,0
        ADD     HL,HL
        ADD     HL,HL
        ADD     HL,HL
        ADD     HL,HL
        ADD     HL,HL
        LD      DE,100
        LD      B,0
btest_div:
        OR      A
        SBC     HL,DE
        JR      C,btest_got
        INC     B
        JR      btest_div
btest_got:
        LD      L,B
        LD      H,0
        LD      (bt_got_k),HL
        CALL    apply
        LD      A,(bt_last_pct)
        CP      100
        JR      C,btest_loop
        LD      HL,str_ready
        CALL    bw_phase
btest_hold:
        CALL    in_frames
        JR      btest_hold
        ENDIF
