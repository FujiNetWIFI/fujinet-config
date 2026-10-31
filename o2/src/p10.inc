; p10.inc -- page 10: the editor's library (p5.inc reaches it by farcall,
; as the others reach page 1): T0's prompt, row 0's status, the key chart,
; and the magnifier with its glyphs -- with a copy of the display's own
; routines, which a page reached by farcall can't farcall for.

    PAGE_HEAD
    db l_edprompt&0FFh          ; L_EDPROMPT
    db l_edstat&0FFh            ; L_EDSTAT
    db l_edchart&0FFh           ; L_EDCHART
    db l_mag&0FFh               ; L_MAG
l_edprompt:
    jmp ed_prompt
l_edstat:
    jmp ed_status
l_edchart:
    jmp ed_chart
l_mag:
    jmp mag

    include "edtextb.inc"
    if $ > 900h
    error "the editor's library's page $8 has run out"
    endif

; ===========================================================================
    org 900h
    include "disp.inc"

; ===========================================================================
    org 0A00h
    include "text.inc"
    include "glyphsb.inc"
glb_get:                        ; A = glyphsb[A]
    add a,#glyphsb&0FFh
    movp a,@a
    ret
    if $ > 0B00h
    error "the editor's library's page $A has run out"
    endif

; ===========================================================================
    org 0B00h
    include "screen.inc"

; ===========================================================================
    org 0C00h
    include "edmag.inc"
    if $ > 0D00h
    error "the editor's library's page $C has run out"
    endif

; ===========================================================================
    org 0D00h
    include "glyphsa.inc"
gla_get:                        ; A = glyphsa[A]
    add a,#glyphsa&0FFh
    movp a,@a
    ret

; ===========================================================================
    org 0E00h
    include "edtexta.inc"
    if $ > 0F00h
    error "the editor's library's page $E has run out"
    endif

    PAGE_TAIL
