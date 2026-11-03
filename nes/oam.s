; oam.s -- the sprite shadow, and the one way it reaches the PPU.
;
; cc65's NES runtime hides every sprite at power-on and never touches OAM
; again, and its NMI spends most of vblank flushing conio's write buffer. So
; the shadow goes over by DMA only when it changes, right after a vblank
; whose flush had nothing to do: wait for the buffer to drain, then for the
; next NMI, and the 513-cycle DMA lands well inside vblank.
;
; void oam_sync(void);

        .export         _oam, _oam_sync
        .import         ppubuf_waitempty

        .include        "nes.inc"

; DMA takes a whole page, and the linker config the library supplies can't
; align BSS to one, so the shadow is the page-aligned 256 bytes of these 511.
.bss

oam_buf:
        .res    511
_oam    = (oam_buf + 255) & $FF00

.code

.proc   _oam_sync

        jsr     ppubuf_waitempty
        lda     #0
        sta     VBLANK_FLAG
@wait:  lda     VBLANK_FLAG     ; set by the NMI, after an empty flush
        beq     @wait
        lda     #0
        sta     PPU_SPR_ADDR
        lda     #>_oam
        sta     APU_SPR_DMA
        rts

.endproc
