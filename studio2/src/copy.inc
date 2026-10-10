; copy.inc -- copying the browsed file to another host slot.
;
; fl_copy appended the file to V_PATH and set V_MODE; the host list then
; picks the destination, and the file lands in that host's root under its
; own name (COPY_FILE's "src|/"). Either way the name comes off V_PATH again
; and the browser comes back where it was.

; cp_go -- copy to host slot V_CUR. Not a root file to its own host: the
; destination would be the source, opened for writing before it is read.
        RFIT    106
cp_go:  LDV     V_CUR
        STV     V_HSEL
        STR     R2
        LDV     V_SRCH
        XOR
        BNZ     cpg0
        LDR     R8, V_PATH+1
cpg00:  LDA     R8
        BZ      cpg9                    ; no '/' past the first: in the root
        XRI     '/'
        BNZ     cpg00
cpg0:   FCALL   pr_leg
        DB      "COPYING...", 0
        FCALL   c_copy
        BZ      cpg1
        FCALL   fail
        DB      9
        DW      cp_end
cpg1:   FCALL   pr_leg
        DB      "COPIED: ANY KEY", 0
        FCALL   key_wait
cp_end: STATE
        FCALL   path_cut
        SETV    V_MODE, 0
        LDV     V_FCUR
        STV     V_CUR
        FJMP    st_files
cpg9:   FJMP    hs_bad
