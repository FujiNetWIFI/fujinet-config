; Read native ATASCII without conio's display cursor side effects.
.export _bbs_getkey
.include "atari.inc"
.code
_bbs_getkey:
    lda #12
    sta ICAX1Z
    jsr keyboard
    ldx #0
    rts
keyboard:
    lda KEYBDV+5
    pha
    lda KEYBDV+4
    pha
    rts
