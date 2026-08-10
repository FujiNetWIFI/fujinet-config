' fujinet.bas -- FujiNet cartridge mailbox transport.
'
' Mailbox layout is the hand-synchronized copy of
' fujinet-firmware/pico/intellivision/firmware/fuji_mailbox.h. Do not change
' these addresses without re-checking that file. Trimmed from the proven
' fujinet-5cardstud/intv/fujinet.bas: this program never uses the N: network
' device or appkeys directly (ROM transfer is handled entirely by the ESP32
' media-type layer via MOUNT_IMAGE), so those blocks are dropped here.
'
' MEMATTR intentionally stops at $97FF, short of the $9800-$9FFF mailbox
' itself: on real hardware the RP2040 maps that whole window as RAM
' unconditionally (inty_cart.c hardcodes it, independent of what this .cfg
' says), so declaring less here doesn't affect real hardware or what POKE
' can reach at runtime. But jzIntv's --fujinet peripheral emulation
' registers its own handler for $9800-$9FFF *after* the cart's generic
' MEMATTR RAM, and its layered bus dispatch lets whichever peripheral
' registered first answer a given address -- so declaring the full
' $8000-$9FFF range here would silently shadow the emulator's FujiNet
' peripheral with inert RAM, and the mailbox would never come up under
' --fujinet even though it works on real hardware.
    ASM MEMATTR $8000, $97FF, "+RWN"

    CONST FN_MAGIC0     = $9800
    CONST FN_MAGIC1     = $9801
    CONST FN_SEQ        = $9803
    CONST FN_ACKSEQ     = $9804
    CONST FN_DEVICE     = $9805
    CONST FN_CMD        = $9806
    CONST FN_NPARAM     = $9807
    CONST FN_TXLEN_LO   = $9808
    CONST FN_TXLEN_HI   = $9809
    CONST FN_ERR        = $980B
    CONST FN_RXLEN_LO   = $980C
    CONST FN_RXLEN_HI   = $980D
    CONST FN_REPLY_CMD  = $980E
    CONST FN_PARAM_SIZE = $9810
    CONST FN_PARAM_VAL  = $9820
    CONST FN_TX         = $9840
    CONST FN_RX         = $9940

    ' Boot-progress cells (RP2040-published), used by st_boot.bas while
    ' polling a MOUNT_IMAGE transaction that may run far longer than an
    ' ordinary mailbox round trip. See fuji_mailbox.h for the full layout.
    CONST FN_BOOT_STATE = $9818
    CONST FN_BOOT_PCT   = $9819
    CONST FN_BOOT_ERR   = $981A
    CONST FUJI_BOOT_FAILED = $80   ' FN_BOOT_STATE value; mirrors fuji_mailbox.h

    CONST FUJICMD_ACK = $06
    CONST FUJICMD_NAK = $15

    CONST FUJI_DEVICEID = $70

    ' fn_ok: 1 = last transaction produced ACK, 0 = timeout or NAK.
    ' mb_err: FN_ERR value on failure (0 on timeout, since the RP2040 never answered).
    DIM fn_ok, mb_err
    DIM mb_dev, mb_cmd, mb_nparam, mb_seq
    DIM #fn_txlen
    DIM #fn_t          ' generic frame-count timeout counter
    DIM #fn_src        ' VARPTR source for putstr/getstr
    DIM fn_len, fn_i   ' generic length/index for putstr/getstr

' ---------------------------------------------------------------------------
' fn_wait_mailbox: bounded wait for the RP2040 magic bytes at boot.
' Sets fn_ok = 1 if the mailbox came up within 180 frames (3s), else 0.
' ---------------------------------------------------------------------------
fn_wait_mailbox: PROCEDURE
    #fn_t = 0
    WHILE (((PEEK(FN_MAGIC0) AND 255) <> 70) OR ((PEEK(FN_MAGIC1) AND 255) <> 78)) AND (#fn_t < 180)
        #fn_t = #fn_t + 1
        WAIT
    WEND
    IF #fn_t >= 180 THEN
        fn_ok = 0
    ELSE
        fn_ok = 1
    END IF
END

' ---------------------------------------------------------------------------
' fn_transact: issue the transaction described by mb_dev/mb_cmd/mb_nparam/
' #fn_txlen (payload already staged at FN_TX) and block for the reply.
' seq is ALWAYS derived from the RP2040's own FN_ACKSEQ, never from a local
' counter -- a console reset zeroes IntyBASIC vars but not the RP2040, so a
' locally incrementing seq would recompute the same value forever and never
' trigger a second transaction after the first boot.
' Sets fn_ok = 1 on ACK, 0 on timeout/NAK (mb_err holds the reason).
' ---------------------------------------------------------------------------
fn_transact: PROCEDURE
    POKE (FN_DEVICE), mb_dev
    POKE (FN_CMD), mb_cmd
    POKE (FN_NPARAM), mb_nparam
    POKE (FN_TXLEN_LO), #fn_txlen AND 255
    POKE (FN_TXLEN_HI), #fn_txlen / 256

    mb_seq = (PEEK(FN_ACKSEQ) AND 255) + 1
    IF mb_seq = 0 THEN mb_seq = 1
    POKE (FN_SEQ), mb_seq

    #fn_t = 0
    WHILE ((PEEK(FN_ACKSEQ) AND 255) <> mb_seq) AND (#fn_t < 900)
        #fn_t = #fn_t + 1
        WAIT
    WEND

    IF #fn_t >= 900 THEN
        fn_ok = 0
        mb_err = 0
        RETURN
    END IF

    IF (PEEK(FN_REPLY_CMD) AND 255) <> FUJICMD_ACK THEN
        fn_ok = 0
        mb_err = PEEK(FN_ERR) AND 255
        RETURN
    END IF

    fn_ok = 1
END

' ---------------------------------------------------------------------------
' fn_param: stage transaction parameter #pm_i (0-based), pm_size bytes
' (1, 2 or 4), value #pm_val, little-endian, into the mailbox's param table.
' ---------------------------------------------------------------------------
DIM pm_i, pm_size
DIM #pm_val
fn_param: PROCEDURE
    POKE (FN_PARAM_SIZE + pm_i), pm_size
    POKE (FN_PARAM_VAL + pm_i * 4), #pm_val AND 255
    IF pm_size > 1 THEN POKE (FN_PARAM_VAL + pm_i * 4 + 1), (#pm_val / 256) AND 255
    IF pm_size > 2 THEN
        POKE (FN_PARAM_VAL + pm_i * 4 + 2), 0
        POKE (FN_PARAM_VAL + pm_i * 4 + 3), 0
    END IF
END

' ---------------------------------------------------------------------------
' fn_putstr: append fn_len ASCII bytes into FN_TX, starting at the current
' #fn_txlen, from the address in #fn_src -- either VARPTR of a ROM DATA
' label or a RAM address (a scratch buffer, or even FN_RX). PEEK is uniform
' across ROM/RAM on the CP-1610's unified address space, so one procedure
' covers both. Advances #fn_txlen.
' ---------------------------------------------------------------------------
fn_putstr: PROCEDURE
    FOR fn_i = 0 TO fn_len - 1
        POKE (FN_TX + #fn_txlen + fn_i), PEEK(#fn_src + fn_i) AND 255
    NEXT fn_i
    #fn_txlen = #fn_txlen + fn_len
END

' ---------------------------------------------------------------------------
' fn_strlen: scan the NUL-terminated field at #fn_src (max ls_max bytes) and
' set fn_len to the length up to (but not including) the first NUL. Use this
' before fn_putstr whenever the source is a fixed-width padded field --
' putstr has no idea where the real string ends, and blindly copying the
' full padded width embeds a literal $00 byte plus whatever garbage follows
' it.
' ---------------------------------------------------------------------------
DIM ls_max
fn_strlen: PROCEDURE
    fn_len = 0
    WHILE (fn_len < ls_max) AND ((PEEK(#fn_src + fn_len) AND 255) <> 0)
        fn_len = fn_len + 1
    WEND
END
