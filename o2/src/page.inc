; page.inc -- the two ends every page carries.
;
; PAGE_HEAD at $800: farcall's dispatcher. A = an entry number; the table
; that follows (the page's own) holds the low bytes of its entry stubs, all
; within $800-$8FF because JMPP stays in its page.
;
; PAGE_TAIL: mb_get at $F00, the read stub for the mailbox page -- MOVP reads
; the page the PC is in, and $F00-$F1F is each page's own -- and on page 0 the
; FUJI claim and the APPBANK header (fuji_mailbox.h), both under the mailbox
; at run time.

; FCALL: a call into the fixed half. AS puts SEL MB0 in front of it and
; nothing behind; RET does not restore MB, so this does (tools/checkmb.py).
FCALL   macro target
    call target
    sel mb1
    endm

; STB: A = reply byte `off` (a 16-bit offset). Clobbers r0, r2, r3, r6.
STB     macro off
    mov r3,#(off)>>8
    mov r2,#(off)&0FFh
    call st_byte
    endm

; JZL, JNZL, JCL, JNCL: a conditional jump to anywhere in the 2K bank (the
; 8048's own reach only to its 256-byte page): the opposite test over a JMP.
JZL     macro t
    jnz lskip
    jmp t
lskip:
    endm
JNZL    macro t
    jz lskip
    jmp t
lskip:
    endm
JCL     macro t
    jnc lskip
    jmp t
lskip:
    endm
JNCL    macro t
    jc lskip
    jmp t
lskip:
    endm

; JB0L, JB7L: JBn to anywhere in the bank (there is no "jump if clear").
JB0L    macro t
    jb0 ltake
    jmp lskip
ltake:
    jmp t
lskip:
    endm
JB7L    macro t
    jb7 ltake
    jmp lskip
ltake:
    jmp t
lskip:
    endm

; MOVP reads only the 256-byte page the code is in, so every page that holds
; strings carries its own readers:
; STRPAGE: the string readers for this 256-byte page, suffix SFX:
;   say_SFX   line r4 (L_*) = the string at offset A, white
;   row_SFX   row r4 = the string at offset A, white, no folder
;   sayc_SFX, rowc_SFX: the same in TXCOL / TXCOL2 (RICON) as they are
;   lbs0_SFX  LINEBUF = spaces, then the string at offset A
;   lbs_SFX   the string at offset A into LINEBUF; lba_SFX: from r0
; The string is NUL-terminated. Clobbers A, r0-r7.
STRPAGE macro SFX,{GLOBALSYMBOLS}
say_SFX:
    call lbs0_SFX
    jmp say_line
row_SFX:
    call lbs0_SFX
    jmp row_str
sayc_SFX:
    call lbs0_SFX
    jmp say_line_c
rowc_SFX:
    call lbs0_SFX
    jmp row_line
lbs0_SFX:
    mov r6,a
    call lb_clear
    mov a,r6
lbs_SFX:
    mov r0,#LINEBUF
lba_SFX:
    mov r1,a
SFX_lbl:
    mov a,r1
    movp a,@a
    jz SFX_lbx
    movx @r0,a
    inc r0
    inc r1
    jmp SFX_lbl
SFX_lbx:
    ret
    endm

PAGE_HEAD macro {GLOBALSYMBOLS}
    org 800h
pg_disp:
    add a,#pg_tab&0FFh
    jmpp @a
pg_tab:
    endm

PAGE_TAIL macro {GLOBALSYMBOLS}
    org 0F00h
mb_get:                         ; A = offset in page $F00 -> the byte there
    movp a,@a
    ret

; far_go: enter page A at its entry r7, one way (a screen hands over to the
; next). The store to CODEBANK changes the code under the PC, so everything
; after it is the same bytes, at the same address, in every page.
far_go:
    mov r1,a
    mov r0,#CURPG
    movx @r0,a
    mov r0,#FN_W_REGSEL
    mov a,#FN_REG_CODEBANK
    movx @r0,a
    inc r0
    mov a,r1
    movx @r0,a                  ; the page changes here
    mov a,r7
    jmp 800h
    if PAGE=0
    org 0F2Ch
    db "FUJI"
    org O2_APP_HDR
    db "APPB",O2_APP_VERSION,NPAGES
    endif
    endm
