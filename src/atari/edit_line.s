        .export     _edit_line

        ; we will reuse this 256 byte buffer that is only used for copying disks, so quite a chunk that's hardly ever used

        .import     _cgetc
        .import     _copySpec
        .import     _cursor_ptr
        .import     _set_cursor
        .import     _strlen
        .import     _strncpy
        .import     popa
        .import     popax
        .import     pusha
        .import     pushax
        .import     return0
        .import     return1

        .include    "atari.inc"
        .include    "zeropage.inc"
        .include    "cfg-macros.inc"
        .include    "cfg-constants.inc"

; int edit_line(unsigned char x, unsigned char y, char *s, unsigned char maxlen)

; re-implementation of edit_line
; returns 1 if there was an edit, 0 if there was none.
.proc _edit_line
        sta     el_max_len          ; max length for editing
        popax   el_str              ; the original string being edit
        popa    el_y
        popa    el_x

        ; copy the src string into our buffer _copySpec
        pushax  #_copySpec          ; dst
        pushax  el_str              ; src
        ldx     #$00
        lda     el_max_len
        jsr     _strncpy

        ; get x,y coordinate on screen into _cursor_ptr
        pusha   el_x
        lda     el_y
        jsr     _set_cursor

        ; setup initial length variables
        setax   el_str
        jsr     _strlen
        sta     el_buf_len
        sta     el_crs_idx
        ; if length of string we initially edit is at max, need to fix cursor position slightly
        jsr     cap_cursor

        mwa     _cursor_ptr, ptr4   ; the holy ptr4 screen location

        ; check if the target string is empty, and clear edit area if it is
        ldy     #$00
        lda     _copySpec
        bne     :+
        jsr     clear_edit          ; clears any <Empty> type text

        ; initialise cursor at end of string
:       ldy     el_crs_idx
        lda     (ptr4), y
        eor     #$80
        sta     (ptr4), y

        mwa     #_copySpec, ptr3

        lda     el_max_len
        sec
        sbc     #$02
        sta     el_max_len_min2         ; required for some end of cursor checks

        ; from now on
        ;   ptr3 = target location of copy string we are editing (_copySpec)
        ;   ptr4 = _cursor_ptr

keyboard_loop:
        ; jsr     _kb_get_c
        jsr     _cgetc
        cmp     #$00
        beq     keyboard_loop

        ldy     el_crs_idx

; ---------------------------------
; ESC
        cmp     #CFK_ESC
        bne     not_esc

        ; abandon edits, and show original string in edit area
        jsr     clear_edit
        mwa     el_str, ptr1
        jsr     put_ptr1_s
        jmp     return0

not_esc:
; ---------------------------------
; BACKSPACE
        cmp     #CFK_BS
        bne     not_bs

        ; y is cursor index, don't allow if it's 0
        cpy     #$00
        beq     keyboard_loop

        ; move all bytes up to end of line down 1 position
:       lda     (ptr3), y
        dey
        sta     (ptr3), y
        iny
        iny
        cpy     el_max_len
        bne     :-

        ; put a zero at the end
        dey
        mva     #$00, {(ptr3), y}

        ; reduce edit index and buffer length
        dec     el_crs_idx
        dec     el_buf_len
        jsr     refresh_line
        jmp     keyboard_loop

not_bs:

; --------------------------------------------------------------------------
; DELETE
        cmp     #CFK_DEL
        bne     not_del

        ; check if cursor is not at the end first. if it is, nothing to do
        lda     el_crs_idx
        cmp     el_buf_len
        bcc     can_del
        bcs     keyboard_loop   ; can't delete as there is nothing ahead of us

can_del:
        ; move bytes forward of ourselves down a position
        iny
:       lda     (ptr3), y
        dey
        sta     (ptr3), y
        iny
        iny
        cpy     el_max_len
        bne     :-

        ; put a zero at the end
        dey
        mva     #$00, {(ptr3), y}

        ; reduce sbuffer length
        dec     el_buf_len
        jsr     refresh_line
        jmp     keyboard_loop

not_del:

; --------------------------------------------------------------------------
; INSERT
        cmp     #CFK_INS
        bne     not_insert

        ; check if cursor is at end, do nothing if it is
        lda     el_crs_idx
        cmp     el_buf_len
        bcc     can_ins
        bcs     keyboard_loop

can_ins:
        ; push everything forward 1 char, up to end of buffer.
        ; string is always terminated up to end of buffer with zeroes due to initial strncpy
        ; last char if there was one, drops off end if string too big.
        ; put space in current location
        ; set y = max_len - 2 (one off for null, one off for not going too far)
        ldy     el_max_len
        dey
        dey

:       lda     (ptr3), y       ; load char from end, push it forward one, down to current position
        iny
        sta     (ptr3), y
        dey
        dey
        bmi     :+              ; were we editing at position 0? TODO: will this break with long strings?
        cpy     el_crs_idx
        bcs     :-

        ; put space at our current location, y is 1 less than cursor location at moment
:       iny
        mva     #' ', {(ptr3), y}

        ; put 0 at end to ensure string is always nul terminated
        ldy     el_max_len
        dey
        mva     #$00, {(ptr3), y}

        ; increase buffer len if we can
        mva     el_max_len_min2, tmp1
        lda     el_buf_len
        cmp     tmp1
        bcs     no_extra
        inc     el_buf_len

no_extra:
        jsr     refresh_line
        jmp     keyboard_loop

not_insert:

; --------------------------------------------------------------------------
; LEFT CURSOR
        cmp     #CFK_LEFT
        bne     not_left

        ; if cursor already at 0, don't move
        lda     el_crs_idx
        cmp     #$00
        beq     :+

        dec     el_crs_idx
        jsr     refresh_line

:
        jmp     keyboard_loop

not_left:

; --------------------------------------------------------------------------
; RIGHT CURSOR
        cmp     #CFK_RIGHT
        bne     not_right

        ; if cursor already at max, don't move.
        lda     el_crs_idx
        cmp     el_max_len_min2

        beq     :+
        ; also can't be longer than the current buffer length
        cmp     el_buf_len
        bcs     :+

        ; allowed to move cursor as it isn't at the ends
        inc     el_crs_idx

:       jsr     refresh_line
        jmp     keyboard_loop

not_right:

; --------------------------------------------------------------------------
; HOME
        cmp     #CFK_HOME
        bne     not_home

        ; set cursor to 0
        mva     #$00, el_crs_idx
        jsr     refresh_line
        jmp     keyboard_loop

not_home:

; --------------------------------------------------------------------------
; END
        cmp     #CFK_END
        bne     not_end_key

        ; set cursor to buf len (end of editing buffer)
        mva     el_buf_len, el_crs_idx
        jsr     cap_cursor

        jsr     refresh_line
        jmp     keyboard_loop

not_end_key:

; --------------------------------------------------------------------------
; KILL
        cmp     #CFK_KILL ; kill text to end of buffer
        bne     not_kill

        ; set buffer length to current char index (y)
        sty     el_buf_len

        lda     #$00
:       sta     (ptr3), y       ; current cursor position forward should become 0s
        iny
        cpy     el_max_len
        bcc     :-

        jsr     refresh_line
        jmp     keyboard_loop

not_kill:

; --------------------------------------------------------------------------
; ATASCII CHAR (between CFK_ASCIIL and CFK_ASCIIH inclusive)
        cmp     #CFK_ASCIIL
        bcs     space_or_more
        bcc     not_ascii
space_or_more:
        cmp     #CFK_ASCIIH+1
        bcs     not_ascii
        sta     tmp1            ; save it while we check bounds

        ; check bounds, if current edit position is on last char, we can't add more
        ldx     el_max_len
        dex
        stx     tmp2
        cpy     tmp2
        bcs     :+

        ; ok! save this char to current edit index (y)
        mva     tmp1, {(ptr3), y}

        ; did we extend the buffer, or overwrite a char?
        ; we are overwriting if y index is less than buf len
        cpy     el_buf_len
        bcc     :+
        inc     el_buf_len

        ; can we move cursor on? yes if not now at end
:       cpy     el_max_len_min2
        beq     :+

        ; allowed to move cursor
        inc     el_crs_idx

:       jsr     refresh_line
        jmp     keyboard_loop

not_ascii:

; --------------------------------------------------------------------------
; ENTER
        cmp     #CFK_ENTER
        bne     not_eol

        ; invert the char at edit location to remove cursor
        lda     (ptr4), y
        eor     #$80            ; invert high bit only. This is actually always 1->0 as we don't allow inverted chars  
        sta     (ptr4), y

        ; save the buffer into the original string's memory
        pushax  el_str          ; dst
        pushax  ptr3            ; src
        ldx     #$00
        lda     el_max_len
        jsr     _strncpy
        ; restore screen pointer for print routines
        mwa     _cursor_ptr, ptr4

end_enter:
        ; mark that we made an edit, so caller must act appropriately.
        jmp     return1

not_eol:
        ; not a char we recognised, go back for next char
        jmp     keyboard_loop

.endproc

; ensure the cursor isn't beyond bounds
.proc cap_cursor
        ; if we're at max-1, put us at max-2 as can't move cursor into last byte
        ldx     el_max_len
        dex
        cpx     el_crs_idx
        bne     :+
        dex
        stx     el_crs_idx

:       rts
.endproc

.proc put_ptr1_s
        ldy     #$00
l1:
        lda     (ptr1), y
        beq     out

        ; convert ascii to screen code, from cputc
        asl     a               ; shift out the inverse bit
        adc     #$c0            ; grab the inverse bit; convert ATASCII to screen code
        bpl     codeok          ; screen code ok?
        eor     #$40            ; needs correction
codeok: lsr     a               ; undo the shift
        bcc     :+
        eor     #$80            ; restore the inverse bit

:       sta     (ptr4), y
        iny
        bne     l1
out:
        rts
.endproc

; show current editing string at screen location and show cursor
.proc refresh_line
        ; copy buffer to screen memory and deal with last char by blanking whole line out first
        jsr     clear_edit
        mwa     ptr3, ptr1
        jsr     put_ptr1_s

        ; load Y with current location index
        ldy     el_crs_idx

        ; invert the screen location's char for cursor display, which handles cursor INSIDE a string
        ; this is visual only, and doesn't affect buffer
        lda     (ptr4), y
        ora     #$80
        sta     (ptr4), y
        rts
.endproc

.proc clear_edit
        lda     #$00    ; screen space
        ldy     #$00
:       sta     (ptr4), y
        iny
        cpy     el_max_len
        bne     :-
        rts
.endproc


.bss
el_max_len:         .res 1
el_max_len_min2:    .res 1
el_str:             .res 2  ; pointer to original string being edit
el_x:               .res 1
el_y:               .res 1
el_crs_idx:         .res 1  ; index in the string of the cursor position
el_buf_len:         .res 1  ; buffer length, keep track as we make edits so don't have to redo strlen