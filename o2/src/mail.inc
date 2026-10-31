; mail.inc -- the cartridge mailbox (fuji_mailbox.h) and the Fuji device.
;
; A transaction is: fn_begin (device, command, parameter count; the TX stream
; rewound), the parameters and payload through FN_W_DATA, then fn_post, which
; writes the sequence number that launches it; fn_go posts and waits. Main
; program, P1 = P1_MAIN (the register file and external RAM both answer
; MOVX). Rules carried from the family's other carts:
;   - the sequence number is the cart's ACKSEQ + 1, never a counter of ours:
;     a console RESET restarts this program, not the cartridge;
;   - nothing else writes the register file while a transaction is out;
;   - the reply is the cart's reply buffer, read through the window (RXWIN):
;     the next transaction replaces it. Until then it is this program's
;     biggest RAM -- a page of directory entries, the host slots -- and the TX
;     stream is the other: a payload can be streamed in from the reply.
;   - waits count frames, never loops: the cart answers in milliseconds on
;     an emulator and in seconds on a real FujiNet.

; fn_reg: register r7 = A. Clobbers r0, r6.
fn_reg:
    mov r6,a
    mov r0,#FN_W_REGSEL
    mov a,r7
    movx @r0,a
    inc r0
    mov a,r6
    movx @r0,a
    ret

; fuji_begin: a Fuji device command r3 with r4 parameters to follow.
; fn_begin: device r2. Clobbers A, r0, r6, r7.
fuji_begin:
    mov r2,#FUJI_DEVICEID
fn_begin:
    mov a,r2
    mov r7,#FN_REG_DEVICE
    call fn_reg
    mov a,r3
    mov r7,#FN_REG_CMD
    call fn_reg
    mov a,r4
    mov r7,#FN_REG_NPARAM
    call fn_reg
    mov r7,#FN_REG_DATA_RST
    jmp fn_reg

; fn_tx: A into the TX stream. Clobbers r0.
fn_tx:
    mov r0,#FN_W_DATA
    movx @r0,a
    ret

; fn_par1: a one-byte parameter, A. Clobbers r0, r6.
fn_par1:
    mov r6,a
    mov a,#1
    call fn_tx
    mov a,r6
    jmp fn_tx

; fn_par2: a two-byte parameter, r6 (low) r5 (high). Clobbers A, r0.
fn_par2:
    mov a,#2
    call fn_tx
    mov a,r6
    call fn_tx
    mov a,r5
    jmp fn_tx

; fn_zeros: r6 zero bytes into the TX stream (a fixed-length payload's
; padding). Clobbers A, r0, r6.
fn_zeros:
    clr a
    call fn_tx
    djnz r6,fn_zeros
    ret

; fn_post: launch it -- SEQ = the cart's ACKSEQ + 1 (0 is never used), kept
; in NSEQ. Clobbers A, r0.
fn_post:
    mov a,#FN_R_ACKSEQ&0FFh
    call mb_get
    inc a
    jnz fp_ok
    inc a
fp_ok:
    mov r0,#NSEQ
    movx @r0,a
    mov r0,#FN_W_SEQ
    movx @r0,a
    ret

; fn_done: A = 0 once the cart has echoed NSEQ. Clobbers A, r0, r6.
fn_done:
    mov a,#FN_R_ACKSEQ&0FFh
    call mb_get
    mov r6,a
    mov r0,#NSEQ
    movx a,@r0
    xrl a,r6
    ret

; fn_ack: A = 0 if the transaction worked and was ACKed. Clobbers A, r0.
fn_ack:
    mov a,#FN_R_ERR&0FFh
    call mb_get
    jnz fa_bad
    mov a,#FN_R_REPLY_CMD&0FFh
    call mb_get
    xrl a,#FC_ACK
fa_bad:
    ret

; fn_go: post and wait, at most about 8 seconds (the cart's own deadline is
; 5); fn_go_long: at most r5 x 256 frames. A = 0 if it worked and was ACKed,
; 0FFh if the cart never answered. The cart polled hard for the first few
; milliseconds (a FujiNet answers most in one or two), then once a frame.
; The display runs on; nothing is read. Clobbers A, r0-r2, r5-r7.
fn_go:
    mov r5,#2
fn_go_long:
    call fn_post
    mov r7,#0                   ; 256 polls: about 15 ms
fgs_l:
    call fn_done
    jz fgo_done
    djnz r7,fgs_l
fgo_l:                          ; then r5 x 256 frames
    call frame_wait
    call fn_done
    jz fgo_done
    djnz r7,fgo_l
    djnz r5,fgo_l
    mov a,#0FFh
    ret
fgo_done:
    jmp fn_ack

; st_byte: A = byte r3:r2 (hi:lo) of the reply -- the window moved to cover
; it. Clobbers r0, r6.
st_byte:
    mov a,r2                    ; window = offset / 16
    anl a,#0F0h
    orl a,r3
    swap a                      ; (r3 < 16: offsets under 4K)
    mov r6,a
    mov r0,#FN_W_REGSEL
    mov a,#FN_REG_RXWIN
    movx @r0,a
    inc r0
    mov a,r6
    movx @r0,a
    mov a,r2
    anl a,#0Fh
    add a,#FN_R_DATA&0FFh
    jmp mb_get

; st_next: A = the reply byte at r3:r2, then r3:r2 on by one. Clobbers r0,
; r6.
st_next:
    call st_byte
    inc r2
    xch a,r2
    jnz stn_x
    inc r3
stn_x:
    xch a,r2
    ret

; rx_str: the NUL-terminated string at reply offset r3:r2 into the TX
; stream, its NUL left out; at most r7 bytes. Clobbers A, r0, r2, r3, r6,
; r7.
rx_str:
    call st_next
    jz rxs_x
    call fn_tx
    djnz r7,rx_str
rxs_x:
    ret

; rx_lb: the NUL-terminated string at reply offset r3:r2 into external RAM
; from r1 (LINEBUF, EDBUF), at most r7 bytes. Clobbers A, r0-r3, r6, r7.
rx_lb:
    call st_next
    jz rxl_x
    movx @r1,a
    inc r1
    djnz r7,rx_lb
rxl_x:
    ret

; xr_tx: r7 bytes of external RAM from r1 into the TX stream, zero-filled
; from r5 bytes on (a fixed-length field holding a string of r5). r5 = 0:
; all zeros. Clobbers A, r0, r1, r5, r7.
xr_tx:
    mov a,r5
    jz xt_z
    movx a,@r1
    inc r1
    dec r5
    jmp xt_put
xt_z:
    clr a
xt_put:
    call fn_tx
    djnz r7,xr_tx
    ret
