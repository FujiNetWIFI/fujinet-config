; fujicmd.inc -- Fuji device (70H) commands on the mailbox transport.
;
; Every command returns carry clear on an ACK, or carry set with A = the
; transport's error (0FFH: no answer in time; 0: a NAK). The reply stays
; in FN_REPLY until the next commit repaints it.
;
; Payload shapes the firmware insists on:
;   - OPEN_DIRECTORY and SET_DEVICE_FULLPATH carry a full 256-byte
;     NUL-padded payload.
;   - COPY_FILE reads its payload as a string, so it goes at its exact
;     length, and its host slots are 1-based.
;   - SET_SSID needs one parameter (ignored) and exactly 97 bytes.
;   - Every command sends all of its parameters: a short count crashes the
;     ESP32.

FC_ACFGX        EQU     0C4H            ; GET_ADAPTERCONFIG_EXTENDED
FC_COPY         EQU     0D8H            ; COPY_FILE
FC_SET_PATH     EQU     0E2H            ; SET_DEVICE_FULLPATH
FC_DIRPOS       EQU     0E4H            ; SET_DIRECTORY_POSITION
FC_WIFI_EN      EQU     0EAH            ; GET_WIFI_ENABLED
FC_WHSL         EQU     0F3H            ; WRITE_HOST_SLOTS
FC_RHSL         EQU     0F4H            ; READ_HOST_SLOTS
FC_CLOSE_DIR    EQU     0F5H            ; CLOSE_DIRECTORY
FC_READ_DIR     EQU     0F6H            ; READ_DIR_ENTRY
FC_OPEN_DIR     EQU     0F7H            ; OPEN_DIRECTORY
FC_MOUNT_IMG    EQU     0F8H            ; MOUNT_IMAGE
FC_MOUNT_HOST   EQU     0F9H            ; MOUNT_HOST
FC_WIFI_STAT    EQU     0FAH            ; GET_WIFISTATUS (3 = connected)
FC_SET_SSID     EQU     0FBH            ; SET_SSID
FC_SCAN_RES     EQU     0FCH            ; GET_SCAN_RESULT
FC_SCAN         EQU     0FDH            ; SCAN_NETWORKS
FC_GET_SSID     EQU     0FEH            ; GET_SSID

CGO_QUANTA      EQU     8               ; past the cart's own 5 s
COPY_QUANTA     EQU     80              ; past COPY_FILE's 60 s

; CGO: commit and check for an ACK.
CGO:    LD      B,CGO_QUANTA
CGO_B:  CALL    FNCOMMIT
        JR      C,cgo_tmo
        OR      A
        JR      NZ,cgo_fail
        LD      A,(FN_REPLYCMD)
        CP      FN_ACK
        RET     Z
        LD      A,(FN_ERR)
cgo_fail:
        SCF
        RET
cgo_tmo:
        LD      A,0FFH
        SCF
        RET

; CDO0: command E, no parameters.
CDO0:   LD      A,FN_DEV_FUJI
        LD      L,0
        CALL    FNBEGIN
        JR      CGO

; CDO1: command E, one 1-byte parameter A.
CDO1:   PUSH    AF
        LD      A,FN_DEV_FUJI
        LD      L,1
        CALL    FNBEGIN
        POP     AF
        CALL    FNPARB
        JR      CGO

; CTXSTR: stream the string at HL, adding its length to C. Clobbers A,
; D, E, HL.
CTXSTR: LD      A,(HL)
        OR      A
        RET     Z
        CALL    FNTXBYT
        INC     HL
        INC     C
        JR      CTXSTR

; CTXFIX: stream exactly B bytes (0 = 256) from HL, zeros once a NUL is
; met. Clobbers A, B, D, E, HL.
CTXFIX: LD      A,(HL)
        OR      A
        JR      Z,ctf_zero
        CALL    FNTXBYT
        INC     HL
        DJNZ    CTXFIX
        RET
ctf_zero:
        XOR     A
        CALL    FNTXBYT
        DJNZ    ctf_zero
        RET

; CPAD: zeros until C bytes sent reach 256 (C = 0: all 256). Clobbers
; A, B, D, E.
CPAD:   LD      B,C
cpad_loop:
        XOR     A
        CALL    FNTXBYT
        INC     B
        JR      NZ,cpad_loop
        RET

cmd_read_hosts:
        LD      E,FC_RHSL
        JR      CDO0

cmd_wifi_enabled:
        LD      E,FC_WIFI_EN
        JR      CDO0

cmd_wifi_status:
        LD      E,FC_WIFI_STAT
        JR      CDO0

cmd_get_ssid:
        LD      E,FC_GET_SSID
        JR      CDO0

cmd_scan:
        LD      E,FC_SCAN
        JR      CDO0

; cmd_scan_result: network A.
cmd_scan_result:
        LD      E,FC_SCAN_RES
        JR      CDO1

cmd_adapter_config:
        LD      E,FC_ACFGX
        JR      CDO0

; cmd_mount_host: host slot A.
cmd_mount_host:
        LD      E,FC_MOUNT_HOST
        JR      CDO1

cmd_close_dir:
        LD      E,FC_CLOSE_DIR
        JR      CDO0

; cmd_open_dir: host `host`, directory `path`, filter fn_entry.
cmd_open_dir:
        LD      A,FN_DEV_FUJI
        LD      E,FC_OPEN_DIR
        LD      L,1
        CALL    FNBEGIN
        LD      A,(host)
        CALL    FNPARB
        LD      C,0
        LD      HL,path
        CALL    CTXSTR
        XOR     A
        CALL    FNTXBYT
        INC     C
        LD      HL,fn_entry
        CALL    CTXSTR
        CALL    CPAD
        JP      CGO

; cmd_reopen_dir: close, best effort, then open: fujinet-pc keeps one
; directory handle a session.
cmd_reopen_dir:
        CALL    cmd_close_dir
        JR      cmd_open_dir

; cmd_seek: SET_DIRECTORY_POSITION HL, 16 bits on the wire.
cmd_seek:
        PUSH    HL
        LD      A,FN_DEV_FUJI
        LD      E,FC_DIRPOS
        LD      L,1
        CALL    FNBEGIN
        POP     HL
        CALL    FNPARW
        JP      CGO

; cmd_read_entry: READ_DIR_ENTRY at most A characters into FN_REPLY.
; Carry set at the end of the directory too: two 7FH bytes, or "." or
; "..", which an SD root repeats forever past its end; reading on
; poisons SET_DIRECTORY_POSITION for the rest of the session.
cmd_read_entry:
        PUSH    AF
        LD      A,FN_DEV_FUJI
        LD      E,FC_READ_DIR
        LD      L,2
        CALL    FNBEGIN
        POP     AF
        CALL    FNPARB
        XOR     A
        CALL    FNPARB
        CALL    CGO
        RET     C
        LD      HL,FN_REPLY
        LD      A,(HL)
        CP      7FH
        JR      NZ,cre_dot
        INC     HL
        LD      A,(HL)
        CP      7FH
        JR      Z,cre_end
        OR      A
        RET
cre_dot:
        CP      '.'
        JR      NZ,cre_ok
        INC     HL
        LD      A,(HL)
        OR      A
        JR      Z,cre_end
        CP      '.'
        JR      NZ,cre_ok
        INC     HL
        LD      A,(HL)
        OR      A
        JR      Z,cre_end
cre_ok: OR      A
        RET
cre_end:
        XOR     A
        SCF
        RET

; cmd_seek_read: entry HL at most A characters wide.
cmd_seek_read:
        PUSH    AF
        CALL    cmd_seek
        JR      C,csr_fail
        POP     AF
        JR      cmd_read_entry
csr_fail:
        POP     BC
        SCF
        RET

; cmd_write_host: host slot A named HL (RAM or ROM). Re-reads the slots
; and streams the other seven straight back out of the reply window.
cmd_write_host:
        LD      (cw_slot),A
        LD      (cw_name),HL
        CALL    cmd_read_hosts
        RET     C
        LD      A,FN_DEV_FUJI
        LD      E,FC_WHSL
        LD      L,0
        CALL    FNBEGIN
        LD      C,0                     ; C = slot
cwh_slot:
        LD      A,(cw_slot)
        CP      C
        JR      NZ,cwh_reply
        LD      HL,(cw_name)
        LD      B,HOST_STRIDE
        CALL    CTXFIX
        JR      cwh_next
cwh_reply:
        LD      A,C
        RRCA                            ; x 32
        RRCA
        RRCA
        LD      L,A
        LD      H,FN_REPLY/256
        LD      B,HOST_STRIDE
cwh_byte:
        LD      A,(HL)
        CALL    FNTXBYT
        INC     HL
        DJNZ    cwh_byte
cwh_next:
        INC     C
        LD      A,C
        CP      HOST_SLOTS
        JR      C,cwh_slot
        JP      CGO

; path_begin: SET_DEVICE_FULLPATH's head: device slot 0, host A, read.
path_begin:
        PUSH    AF
        LD      A,FN_DEV_FUJI
        LD      E,FC_SET_PATH
        LD      L,3
        CALL    FNBEGIN
        LD      A,DEVICE_SLOT
        CALL    FNPARB
        POP     AF
        CALL    FNPARB
        LD      A,MODE_READ
        JP      FNPARB

; cmd_path_reply: the device path on `host` is `path` plus the name the
; reply window holds: the name streams cart to cart.
cmd_path_reply:
        LD      A,(host)
        CALL    path_begin
        LD      C,0
        LD      HL,path
        CALL    CTXSTR
        LD      HL,FN_REPLY
cpr_name:
        LD      A,(HL)
        OR      A
        JR      Z,cpr_pad
        CALL    FNTXBYT
        INC     HL
        INC     C
        JR      NZ,cpr_name
        JP      CGO                     ; exactly 256: nothing to pad
cpr_pad:
        CALL    CPAD
        JP      CGO

; cmd_path_fixed: the device path on host A is the string at HL.
cmd_path_fixed:
        PUSH    HL
        CALL    path_begin
        POP     HL
        LD      B,0
        CALL    CTXFIX
        JP      CGO

; cmd_set_ssid: SSID src_spec, password fn_entry.
cmd_set_ssid:
        LD      A,FN_DEV_FUJI
        LD      E,FC_SET_SSID
        LD      L,1
        CALL    FNBEGIN
        XOR     A
        CALL    FNPARB
        LD      HL,src_spec
        LD      B,SSID_LEN
        CALL    CTXFIX
        LD      HL,fn_entry
        LD      B,ENTRY_MAX
        CALL    CTXFIX
        JP      CGO

; cmd_copy: src_spec on src_host into `path` on `host`, under the source's
; own name: "src|destdir/basename", at its exact length.
cmd_copy:
        LD      A,FN_DEV_FUJI
        LD      E,FC_COPY
        LD      L,2
        CALL    FNBEGIN
        LD      A,(src_host)
        INC     A
        CALL    FNPARB
        LD      A,(host)
        INC     A
        CALL    FNPARB
        LD      HL,src_spec
        CALL    CTXSTR
        LD      A,'|'
        CALL    FNTXBYT
        LD      HL,path
        CALL    CTXSTR
        LD      HL,src_spec             ; the basename: after the last '/'
        LD      B,H
        LD      C,L
ccp_scan:
        LD      A,(HL)
        OR      A
        JR      Z,ccp_base
        INC     HL
        CP      '/'
        JR      NZ,ccp_scan
        LD      B,H
        LD      C,L
        JR      ccp_scan
ccp_base:
        LD      H,B
        LD      L,C
        CALL    CTXSTR
        LD      B,COPY_QUANTA
        JP      CGO_B

; mount_start: MOUNT_IMAGE into device slot 0, sent but not waited for:
; the server pushes the whole image before it ACKs. A = the SEQ to watch
; FN_ACKSEQ for. Nothing may be written to the mailbox until it arrives.
mount_start:
        LD      A,FN_DEV_FUJI
        LD      E,FC_MOUNT_IMG
        LD      L,2
        CALL    FNBEGIN
        LD      A,DEVICE_SLOT
        CALL    FNPARB
        LD      A,MODE_READ
        CALL    FNPARB
        CALL    FN_NEXTSEQ
        PUSH    AF
        LD      C,FR_SEQ
        CALL    FNREGWR
        POP     AF
        RET

; reply_ok: Z if the last reply is an ACK with no transport error.
reply_ok:
        LD      A,(FN_ERR)
        OR      A
        RET     NZ
        LD      A,(FN_REPLYCMD)
        CP      FN_ACK
        RET
