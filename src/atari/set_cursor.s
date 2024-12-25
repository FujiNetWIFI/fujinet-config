        .export     _set_cursor

        .import     _active_screen
        .import     _cursor_ptr
        .import     _video_ptr
        .import     popa

        .include    "zeropage.inc"
        .include    "cfg-macros.inc"

; void set_cursor(uint8_t x, uint8_t y)
;
; A replacement for the original set_cursor, but using pre-computed offsets for each line
.proc _set_cursor
    sta     tmp2        ; y
    jsr     popa
    sta     tmp1        ; x

    ; find the relevant table for the active_screen value
    ldx     _active_screen
    lda     active_screen_lookup, x     ; this is 0, 1, 2 or 3 - the index into set_cursor_table_*
    tax
    lda     set_cursor_table_lo, x
    sta     ptr1
    lda     set_cursor_table_hi, x
    sta     ptr1+1

    ; ptr1 now points to start of relevant byte offsets for a line
    ; each table holds the WORD offsets for Y, need to double Y so we can get correct lo/hi byte from table for offest
    lda     tmp2
    asl     a
    tay             ; y coord * 2 = offset in table to memory to add

    lda     (ptr1), y
    sta     ptr2
    iny
    lda     (ptr1), y
    sta     ptr2+1

    mwa     #_video_ptr, ptr3   ; current _video_ptr

    ; add offset to its location
    ldy     #$00
    lda     (ptr3), y
    clc
    adc     ptr2
    sta     ptr2
    iny
    lda     (ptr3), y
    adc     ptr2+1
    sta     ptr2+1

    adw1    ptr2, tmp1      ; ptr2 = _video_ptr + line offset + x

    ; save it to _cursor_ptr
    mwa     ptr2, _cursor_ptr

    rts

.endproc

.rodata
sc_hosts_devs:
    ; 2 x 20, 8 x 40, 2 x 20, ... x 40
    .word 0, 20
    .word 40, 80, 120, 160, 200, 240, 280, 320
    .word 360, 380
    .word 400, 440, 480, 520, 560, 600, 640, 680, 720, 760, 800, 840, 880, 920, 960, 1000

sc_show_info:
    .word 0, 20
    .word 40, 80, 120
    .word 160, 180
    .word 200, 240, 280, 320, 360, 400, 440, 480, 520, 560, 600, 640, 680, 720, 760, 800, 840

sc_sel_file:
sc_sel_slot:
sc_mount_boot:
    ; 1 x 20, N x 40
    .word 0
    .word 20, 60, 100, 140, 180, 220, 260, 300, 340, 380, 420, 460, 500, 540, 580, 620, 660, 700, 740, 780, 820, 860, 900, 940, 980, 1020, 1060, 1100, 1140, 1180, 1220, 1260, 1300

sc_set_wifi:
    .word 0, 20
    .word 40, 80, 120, 160, 200, 240, 280, 320, 360, 400, 440, 480, 520, 560, 600, 640, 680, 720, 760, 800
    .word 840, 880
    .word 900, 940, 980, 1020, 1060, 1100, 1140, 1180, 1220

.define set_cursor_table sc_hosts_devs, sc_show_info, sc_sel_file, sc_set_wifi

set_cursor_table_lo:    .lobytes set_cursor_table
set_cursor_table_hi:    .hibytes set_cursor_table

;    SCREEN_HOSTS_AND_DEVICES,
;    SCREEN_SHOW_INFO,
;    SCREEN_SET_WIFI,
;    SCREEN_SELECT_FILE,
;    SCREEN_SELECT_SLOT,
;    SCREEN_MOUNT_AND_BOOT,
;    SCREEN_CONNECT_WIFI

; the offset in set_cursor_table_* for each screen type
active_screen_lookup:   .byte 0, 1, 3, 2, 2, 2, 3