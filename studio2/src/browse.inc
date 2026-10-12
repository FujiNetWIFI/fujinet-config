; browse.inc -- st_files: the directory browser.
;
; A page is OPEN_DIRECTORY, one SET_DIRECTORY_POSITION unless it is the
; first, and ONE READ_DIR_ENTRY in block mode: the reply window then holds
; the page's NROWS entries with full names and directory flags, fb_parse
; notes where each name starts, and every row -- and every move of the bar --
; prints from there with no further round trip. The directory stays open
; until the next OPEN_DIRECTORY, or a boot, closes it.
;
; Descending, copying and booting append the name to V_PATH straight from the
; reply window; path_cut takes a file name off again.

; fl_top -- the first page of V_PATH, cursor on top.
        RFIT    174
fl_top: SETV    V_POSL, 0
        SETV    V_POSH, 0
        SETV    V_CUR, 0
st_files:
        STATE
        FCALL   fb_title
        LDV     V_HOST
        FCALL   c_odir
        BZ      fl1
        FCALL   fail
        DB      3
        DW      st_hosts
fl1:    LDV     V_POSL
        INC     RB
        SEX     RB
        OR
        SEX     R2
        BZ      fl2                     ; the first page: no seek
        FCALL   c_spos
        BZ      fl2
        BR      fl_top                  ; past the end: start over
fl2:    FCALL   c_rdblk
        BZ      fl3
        FCALL   fail
        DB      4
        DW      st_hosts
fl3:    FCALL   fb_parse
        LDV     V_NROW                  ; keep the cursor on the page
        BZ      fl5
        STR     R2
        LDV     V_CUR
        SM
        BNF     fl6
        LDN     R2
        SMI     1
fl5:    STV     V_CUR
fl6:    LDV     V_NROW
        BNZ     fl7
        FCALL   pr_at
        DB      11H, "(EMPTY)", 0
        BR      fl_in
fl7:    LDI     0
fl8:    STV     V_I
        FCALL   fb_row
        LDV     V_I
        ADI     1
        STR     R2
        LDV     V_NROW
        XOR
        BZ      fl9
        LDN     R2
        BR      fl8
fl9:    LDI     0FH                     ; the title's last cell: more pages?
        FCALL   d_at
        LDV     V_MORE
        BZ      fl10
        LDI     SYM_DOWN
        BR      fl11
fl10:   LDV     V_POSL
        INC     RB
        SEX     RB
        OR
        SEX     R2
        BZ      fl_in
        LDI     SYM_UP
fl11:   FCALL   d_putc
fl_in:  FJMP    fl_key

        RFIT    34
fl_key: FCALL   key_wait
        FCALL   kdisp
        KEYTO   2, fl_up
        KEYTO   8, fl_dn
        KEYTO   4, fl_pu
        KEYTO   6, fl_pd
        KEYTO   5, fl_sel
        KEYTO   0, fl_back
        KEYTO   7, fl_copy
        KEYTO   3, fl_info
        DB      0FFH
        FJMP    fl_key

; Up past the top row is the previous page's bottom; down past the last row
; the next page's top.
        RFIT    140
fl_up:  FCALL   lst_up
        BDF     fl_mv
        LDI     NROWS-1
        BR      fl_pu2
fl_dn:  FCALL   lst_dn
        BDF     fl_mv
        BR      fl_pd
fl_mv:  LDV     V_OLD
        FCALL   fb_row
        LDV     V_CUR
        FCALL   fb_row
        FJMP    fl_key
fl_bad: LDI     8
        PLO     RD
        FJMP    fl_key
fl_pd:  LDV     V_MORE
        BZ      fl_bad
        LDV     V_POSL
        ADI     NROWS
        STR     RB
        INC     RB
        LDN     RB
        ADCI    0
        STR     RB
        SETV    V_CUR, 0
        FJMP    st_files
fl_pu:  LDI     0
fl_pu2: STV     V_ARG                   ; the cursor on the page above
        LDV     V_POSL
        INC     RB
        SEX     RB
        OR
        SEX     R2
        BZ      fl_bad                  ; the first page already
        LDV     V_ARG
        STV     V_CUR
        LDV     V_POSL
        SMI     NROWS
        STR     RB
        INC     RB
        LDN     RB
        SMBI    0
        STR     RB
        FJMP    st_files
fl_info:
        SETW    V_IRET, st_files
        FJMP    st_info

; fl_back -- up a directory, or back to the host slots from the root.
fl_back:
        LDR     R8, V_PATH+1
        LDN     R8
        BNZ     flb1
        FJMP    st_hosts
flb1:   FCALL   path_up
        FJMP    fl_top

; fl_sel -- a directory: go in. A file: boot it.
        RFIT    58
fl_sel: LDV     V_NROW
        BZ      fls9
        LDV     V_CUR
        FCALL   ent_dir
        STV     V_ARG
        FCALL   ent_abs
        FCALL   path_add
        BNF     fls9                    ; no room in V_PATH
        LDV     V_ARG
        BNZ     fls1
        FJMP    st_boot
fls1:   LDR     R8, s_slash
        FCALL   path_add
        BDF     fls2
        FCALL   path_cut
        BR      fls9
fls2:   FJMP    fl_top
fls9:   FJMP    fl_bad

; fl_copy -- this file is the copy's source: pick where it goes.
        RFIT    55
fl_copy:
        LDV     V_NROW
        BZ      flc9
        LDV     V_CUR
        FCALL   ent_dir
        BNZ     flc9                    ; not a directory
        FCALL   ent_abs
        FCALL   path_add
        BNF     flc9
        LDV     V_HOST
        STV     V_SRCH
        LDV     V_CUR
        STV     V_FCUR
        SETV    V_MODE, 1
        FJMP    hs_go
flc9:   FJMP    fl_bad

; ent_off -- R8 = entry D's name, as a reply-window offset.
; ent_abs -- R8 from an offset to its address.
; ent_dir -- ent_off, then D = $80 if the entry is a directory: bit 7 of the
;            entry's second byte, seven before its name.
        RFIT    33
ent_dir:
        FCALL   ent_off
        GLO     R8
        SMI     7
        PLO     R9
        GHI     R8
        SMBI    0
        ADI     HI(FN_REPLY)
        PHI     R9
        LDN     R9
        ANI     80H
        FRET
ent_off:
        SHL
        ADI     LO(V_EOFF)
        PLO     RB
        LDA     RB
        PHI     R8
        LDN     RB
        PLO     R8
        FRET
ent_abs:
        PHI     RD
        GHI     R8
        ADI     HI(FN_REPLY)
        PHI     R8
        GHI     RD
        FRET

; fb_row -- paint entry D: a folder or a blank, then the name.
        RFIT    65
fb_row: STV     V_ROW
        ADI     1
        SHL
        SHL
        SHL
        SHL
        FCALL   d_at
        FCALL   is_cur
        BNF     fr1
        LDI     TOP_MARQUEE
        FCALL   d_op
        LDI     TOP_INV_ON
        FCALL   d_op
fr1:    LDV     V_ROW
        FCALL   ent_dir
        BZ      fr2
        LDI     GL_DIR
        BR      fr3
fr2:    LDI     ' '
fr3:    FCALL   d_putc
        FCALL   is_cur
        LDI     15
        BNF     fr4
        LDI     0
fr4:    FCALL   d_rplpad
        LDI     TOP_INV_OFF
        FJMP    d_op

; fb_title -- the path's tail on row 0 ('<' if cut), short of the last cell
; (the page arrow's), and the legend on row 9.
        RFIT    57
fb_title:
        LDI     TOP_CLS
        FCALL   d_op
        LDR     R8, V_PATH
fbt1:   LDA     R8
        BNZ     fbt1
        DEC     R8
        GLO     R8                      ; the length: V_PATH is page-aligned
        SMI     16
        BNF     fbt2
        LDI     '<'
        FCALL   d_putc
        GLO     R8
        SMI     14
        BR      fbt3
fbt2:   LDI     LO(V_PATH)
fbt3:   PLO     R8
        FCALL   d_puts
        FCALL   pr_leg
        DB      "5:GO 0:UP 7:COPY", 0
        FRET

; fb_parse -- V_NROW, V_EOFF and V_MORE from the block-mode reply:
;   'M','F', header size, group count, then groups of
;   flags (bit 7: the last), count, size (2), index, then entries of
;   4 date/flag bytes (bit 7 of the second: a directory), 3 size, 1 type,
;   and the NUL-terminated name.
; More follows unless this group is the last, or the next one is empty.
        RFIT    140
fb_parse:
        SETV    V_NROW, 0
        SETV    V_MORE, 0
        LDR     R8, FN_REPLY
        LDA     R8
        XRI     'M'
        BNZ     fbp9
        LDA     R8
        XRI     'F'
        BNZ     fbp9
        LDA     R8                      ; the header's size
        PHI     R9
        LDN     R8
        STV     V_NGRP
        GHI     R9
        PLO     R8                      ; the first group
        LDA     R8
        STV     V_GFLG
        LDA     R8
        STV     V_GCNT
        SMI     NROWS+1
        BNF     fbp0
        LDI     NROWS
        STR     RB
fbp0:   INC     R8
        INC     R8
        INC     R8                      ; the first entry
        LDR     R9, V_EOFF
fbp1:   LDV     V_NROW
        STR     R2
        LDV     V_GCNT
        XOR
        BZ      fbp3
        GLO     R8
        ADI     8
        PLO     R8
        GHI     R8
        ADCI    0
        PHI     R8                      ; the name
        SMI     HI(FN_REPLY)
        STR     R9
        INC     R9
        GLO     R8
        STR     R9
        INC     R9
fbp2:   LDA     R8
        BNZ     fbp2
        GHI     R8
        XRI     HI(FN_STATUS)
        BZ      fbp9                    ; ran off the window
        LDV     V_NROW
        ADI     1
        STR     RB
        BR      fbp1
fbp3:   LDV     V_GFLG
        ANI     80H
        BNZ     fbp9
        LDV     V_NGRP
        SMI     2
        BNF     fbp4
        INC     R8
        LDN     R8                      ; the next group's count
        BZ      fbp9
fbp4:   SETV    V_MORE, 1
fbp9:   FRET
