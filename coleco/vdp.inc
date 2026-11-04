; vdp.inc -- the TMS9918A, Graphics I, and the VRAM map.
;
; In active display the VDP needs about 29 T-states between data-port
; accesses; every loop here takes more than that per byte.

VDP_DATA        EQU     0BEH
VDP_CTRL        EQU     0BFH

NAME_TAB        EQU     1800H           ; the screen, 32 x 24
SAVE_TAB        EQU     1C00H           ; what a pop-up or the scroll covers
COLR_TAB        EQU     2000H           ; a colour per 8 patterns
PATT_TAB        EQU     0000H
SPAT_TAB        EQU     1B00H           ; sprite attributes
SPPT_TAB        EQU     3800H           ; sprite patterns

; vdp_reg: VDP register B = A. Clobbers A.
vdp_reg:
        OUT     (VDP_CTRL),A
        LD      A,B
        OR      80H
        OUT     (VDP_CTRL),A
        RET

; vdp_wr_addr: the VDP at VRAM HL, for writing. Clobbers A.
vdp_wr_addr:
        LD      A,L
        OUT     (VDP_CTRL),A
        LD      A,H
        AND     3FH
        OR      40H
        OUT     (VDP_CTRL),A
        RET

; vdp_rd_addr: the VDP at VRAM HL, for reading. Clobbers A.
vdp_rd_addr:
        LD      A,L
        OUT     (VDP_CTRL),A
        LD      A,H
        AND     3FH
        OUT     (VDP_CTRL),A
        RET

; vdp_write: BC bytes from HL to VRAM DE. Clobbers A, BC, HL.
vdp_write:
        EX      DE,HL
        CALL    vdp_wr_addr
        EX      DE,HL
vw_loop:
        LD      A,(HL)
        OUT     (VDP_DATA),A
        INC     HL
        DEC     BC
        LD      A,B
        OR      C
        JR      NZ,vw_loop
        RET

; vdp_fill: BC bytes of A from VRAM HL. Clobbers A, BC, E.
vdp_fill:
        LD      E,A
        CALL    vdp_wr_addr
vf_loop:
        LD      A,E
        OUT     (VDP_DATA),A
        DEC     BC
        LD      A,B
        OR      C
        JR      NZ,vf_loop
        RET

; vdp_read: B bytes from VRAM HL to DE (1-255). Clobbers A, B, DE.
vdp_read:
        CALL    vdp_rd_addr
        PUSH    HL                      ; the VDP's read-ahead needs a moment
        POP     HL
vr_loop:
        IN      A,(VDP_DATA)
        LD      (DE),A
        INC     DE
        DJNZ    vr_loop
        RET

; vram_copy: B bytes (1-32) from VRAM HL to VRAM DE, through rowbuf.
vram_copy:
        PUSH    DE
        PUSH    BC
        LD      DE,rowbuf
        CALL    vdp_read
        POP     BC
        POP     DE
        LD      HL,rowbuf
        LD      C,B
        LD      B,0
        JP      vdp_write
