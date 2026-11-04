; mailbox.inc -- the Z80 half of the FujiNet cartridge mailbox.
;
; Mirrors fujinet-firmware/pico/coleco/firmware/include/fuji_mailbox.h,
; the source of truth; every equate here is a console address.
;
; The cart edge has no /RD or /WR, so both directions ride reads: a
; register write is a read of FN_REGSEL+reg then of FN_REGDAT+value, a TX
; byte is a read of FN_TXDATA+byte, and the reply is plain ROM the cart
; repaints. Never write anywhere in 8000H-FFFFH: on the console a write
; looks like a read and would fire a hotspot.

FN_REPLY        EQU     0F800H          ; the whole 1K reply
FN_ACKSEQ       EQU     0FC00H          ; echoes SEQ when the reply is ready
FN_STATUS       EQU     0FC01H
FN_ERR          EQU     0FC02H          ; the transport's verdict, 0 = ok
FN_REPLYCMD     EQU     0FC03H          ; 06H ACK / 15H NAK
FN_BOOTSTATE    EQU     0FC06H
FN_BOOTPCT      EQU     0FC07H          ; 0-100 while the image arrives
FN_BOOTERR      EQU     0FC08H
FN_MAGICF       EQU     0FC09H          ; 'F'
FN_MAGICN       EQU     0FC0AH          ; 'N'
FN_SWAP         EQU     0FDFEH          ; read once, armed: the staged image

FN_REGSELP      EQU     0FDH            ; hotspot pages (zmac has no HIGH)
FN_REGDATP      EQU     0FEH
FN_TXDATAP      EQU     0FFH

FR_DEVICE       EQU     00H             ; registers
FR_CMD          EQU     01H
FR_NPARAM       EQU     02H
FR_DATA_RST     EQU     05H
FR_SEQ          EQU     10H
FR_BOOTLOCK     EQU     11H
FN_BOOTLOCK_MAGIC EQU   0B5H

BOOT_IDLE       EQU     00H
BOOT_XFER       EQU     01H
BOOT_READY      EQU     02H
BOOT_FAILED     EQU     80H

FN_ACK          EQU     06H
FN_ETIMEOUT     EQU     02H
FN_DEV_FUJI     EQU     70H

; FNREGWR: register C = A. The two reads go back to back. Clobbers A, B,
; D, E.
FNREGWR:
        LD      B,A
        LD      D,FN_REGSELP
        LD      E,C
        LD      A,(DE)
        LD      D,FN_REGDATP
        LD      E,B
        LD      A,(DE)
        RET

; FNTXBYT: A onto the TX stream. Clobbers A, D, E.
FNTXBYT:
        LD      D,FN_TXDATAP
        LD      E,A
        LD      A,(DE)
        RET

; FNPARB: a 1-byte parameter A. Clobbers A, B, D, E.
FNPARB:
        LD      B,A
        LD      A,1
        CALL    FNTXBYT
        LD      A,B
        JP      FNTXBYT

; FNPARW: a 2-byte parameter HL. Clobbers A, D, E.
FNPARW:
        LD      A,2
        CALL    FNTXBYT
        LD      A,L
        CALL    FNTXBYT
        LD      A,H
        JP      FNTXBYT

; FNBEGIN: begin command E for device A with L parameters; the caller
; then streams the parameters and payload.
FNBEGIN:
        PUSH    HL
        PUSH    DE
        PUSH    AF
        LD      C,FR_DATA_RST
        XOR     A
        CALL    FNREGWR
        POP     AF
        LD      C,FR_DEVICE
        CALL    FNREGWR
        POP     DE
        LD      A,E
        LD      C,FR_CMD
        CALL    FNREGWR
        POP     HL
        LD      A,L
        LD      C,FR_NPARAM
        JP      FNREGWR

; FN_NEXTSEQ: A = the SEQ the next transaction must carry: the cart's
; ACKSEQ + 1, wrapping 255 to 1. Never a local counter: a console RESET
; restarts this program but not the cart.
FN_NEXTSEQ:
        LD      A,(FN_ACKSEQ)
        INC     A
        RET     NZ
        INC     A
        RET

; FNCOMMIT: send it and wait B quanta of about 0.9 s for the ACKSEQ.
; Carry set on timeout, else A = FN_ERR (Z when 0).
FNCOMMIT:
        CALL    FN_NEXTSEQ
        LD      H,A
        LD      C,FR_SEQ
        CALL    FNREGWR
fc_outer:
        LD      DE,0
fc_inner:
        LD      A,(FN_ACKSEQ)
        CP      H
        JR      Z,fc_done
        DEC     DE
        LD      A,D
        OR      E
        JR      NZ,fc_inner
        DJNZ    fc_outer
        SCF
        RET
fc_done:
        LD      A,(FN_ERR)
        OR      A
        RET

; FNCHECK: Z if a FujiNet cart is underneath. Clobbers A.
FNCHECK:
        LD      A,(FN_MAGICF)
        CP      'F'
        RET     NZ
        LD      A,(FN_MAGICN)
        CP      'N'
        RET
