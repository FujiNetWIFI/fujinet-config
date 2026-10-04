; disp_pal.s -- one palette entry through cc65's PPU write buffer, so it lands
; during vblank like the rest of conio's writes.
;
; void __fastcall__ disp_pal_set(unsigned char idx, unsigned char val);

        .export         _disp_pal_set
        .import         popa, ppubuf_put

.code

.proc   _disp_pal_set

        pha                     ; val
        jsr     popa            ; idx
        tax                     ; X = address low
        pla                     ; A = value
        ldy     #$3F            ; Y = address high
        jmp     ppubuf_put

.endproc
